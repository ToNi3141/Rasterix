/**
 * Copyright (C) 2021 Xilinx, Inc
 *
 * Licensed under the Apache License, Version 2.0 (the "License"). You may
 * not use this file except in compliance with the License. A copy of the
 * License is located at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

/* DMA Proxy
 *
 * This module is designed to be a small example of a DMA device driver that is
 * a client to the DMA Engine using the AXI DMA / MCDMA driver. It serves as a proxy
 * for kernel space DMA control to a user space application.
 *
 * A zero copy scheme is provided by allowing user space to mmap a kernel allocated
 * memory region into user space, referred to as a set of channel buffers. Ioctl functions 
 * are provided to start a DMA transfer (non-blocking), finish a DMA transfer (blocking) 
 * previously started, or start and finish a DMA transfer blocking until it is complete.
 * An input argument which specifies a channel buffer number (0 - N) to be used for the
 * transfer is required.
 *
 * By default the kernel memory allocated for user space mapping is going to be 
 * non-cached at this time. Non-cached memory is pretty slow for the application.
 * A h/w coherent system for MPSOC has been tested and is recommended for higher
 * performance applications. 
 *
 * Hardware coherency requires the following items in the system as documented on the 
 * Xilinx wiki and summarized below::
 *   The AXI DMA read and write channels AXI signals must be tied to the correct state to
 *    generate coherent transactions.
 *   An HPC slave port on MPSOC is required
 *   The CCI of MPSOC must be initialized prior to the APU booting Linux
 *   A dma-coherent property is added in the device tree for the proxy driver.
 *
 * There is an associated user space application, dma_proxy_test.c, and dma_proxy.h
 * that works with this device driver.
 *
 * The hardware design was tested with an AXI DMA / MCDMA  with scatter gather and
 * with the transmit channel looped back to the receive channel. It should
 * work with or without scatter gather as the scatter gather mentioned in the 
 * driver is only at the s/w framework level rather than in the hw.
 *
 * This driver is character driver which creates devices that user space can
 * access for each DMA channel, such as /dev/dma_proxy_rx and /dev/dma_proxy_tx.
 * The number and names of channels are taken from the device tree.
 * Multiple instances of the driver (with multiple IPs) are also supported.

 * An internal test mode is provided to allow it to be self testing without the 
 * need for a user space application and this mode is good for making bigger
 * changes to this driver.
 *
 * This driver is designed to be simple to help users get familiar with how to 
 * use the DMA driver provided by Xilinx which uses the Linux DMA Engine. 
 *
 * To use this driver a node must be added into the device tree.  Add a 
 * node similar to the examples below adjusting the dmas property to match the
 * name of the AXI DMA / MCDMA node.
 * 
 * The dmas property contains pairs with the first of each pair being a reference
 * to the DMA IP in the device tree and the second of each pair being the
 * channel of the DMA IP. For the AXI DMA IP the transmit channel is always 0 and
 * the receive is always 1. For the AXI MCDMA IP the 1st transmit channel is
 * always 0 and receive channels start at 16 since there can be a maximum of 16
 * transmit channels. Each name in the dma-names corresponds to a pair in the dmas
 * property and is only a logical name that allows user space access to the channel
 * such that the name can be any name as long as it is unique.
 *
 *	For h/w coherent systems with MPSoC, the property dma-coherent can be added
 * to the node in the device tree. 
 * 
 * Example device tree nodes: 
 *
 * For AXI DMA with transmit and receive channels with a loopback in hardware
 * 
 * dma_proxy {
 *   compatible ="xlnx,dma_proxy";
 *   dmas = <&axi_dma_1_loopback 0  &axi_dma_1_loopback 1>;
 *   dma-names = "dma_proxy_tx", "dma_proxy_rx";
 * };
 *
 * For AXI DMA with only the receive channel
 * 
 * dma_proxy2 {
 *   compatible ="xlnx,dma_proxy";
 *   dmas = <&axi_dma_0_noloopback 1>;
 *   dma-names = "dma_proxy_rx_only";
 * };
 *
 * For AXI MCDMA with two channels 
 *
 * dma_proxy3 {
 *   compatible ="xlnx,dma_proxy";
 *   dmas = <&axi_mcdma_0 0  &axi_mcdma_0 16 &axi_mcdma_0 1 &axi_mcdma_0 17> ;
 *   dma-names = "dma_proxy_tx_0", "dma_proxy_rx_0", "dma_proxy_tx_1", "dma_proxy_rx_1";
 * };
 */

#include <linux/dmaengine.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/dma-mapping.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/workqueue.h>
#include <linux/platform_device.h>
#include <linux/of_dma.h>
#include <linux/ioctl.h>
#include <linux/uaccess.h>
#include "dma-proxy.h"

MODULE_LICENSE("GPL");

#define DRIVER_NAME 			"dma_proxy"
#define TX_CHANNEL			0
#define RX_CHANNEL			1
#define ERROR 					-1
#define TEST_SIZE 			1024

/* The following module parameter controls if the internal test runs when the module is inserted.
 * Note that this test requires a transmit and receive channel to function and uses the first
 * transmit and receive channnels when multiple channels exist.
 */
static unsigned internal_test = 0;
module_param(internal_test, int, S_IRUGO);

/* The following data structures represent a single channel of DMA, transmit or receive in the case
 * when using AXI DMA.  It contains all the data to be maintained for the channel.
 */
struct proxy_bd {
	struct completion cmp;
	dma_cookie_t cookie;
	dma_addr_t dma_handle;
	struct scatterlist sglist;
};
struct dma_proxy_channel {
	struct channel_buffer *buffer_table_p;	/* user to kernel space interface */
	dma_addr_t buffer_phys_addr;

	struct device *proxy_device_p;				/* character device support */
	struct device *dma_device_p;
	dev_t dev_node;
	struct cdev cdev;
	struct class *class_p;

	struct proxy_bd bdtable[BUFFER_COUNT];

	struct dma_chan *channel_p;				/* dma support */
	u32 direction;						/* DMA_MEM_TO_DEV or DMA_DEV_TO_MEM */
	int bdindex;
};

struct dma_proxy {
	int channel_count;
	struct dma_proxy_channel *channels;
	char **names;
	struct work_struct work;
};

static int total_count;

/* Handle a callback and indicate the DMA transfer is complete to another
 * thread of control
 */
static void sync_callback(void *completion)
{
	/* Indicate the DMA transaction completed to allow the other
	 * thread of control to finish processing
	 */
	complete(completion);
}

/* Prepare a DMA buffer to be used in a DMA transaction, submit it to the DMA engine
 * to ibe queued and return a cookie that can be used to track that status of the
 * transaction
 */
static void start_transfer(struct dma_proxy_channel *pchannel_p)
{
	enum dma_ctrl_flags flags = DMA_CTRL_ACK | DMA_PREP_INTERRUPT;
	struct dma_async_tx_descriptor *chan_desc;
	struct dma_device *dma_device = pchannel_p->channel_p->device;
	int bdindex = pchannel_p->bdindex;

	/* A single entry scatter gather list is used as it's not clear how to do it with a simpler method.
	 * Get a descriptor for the transfer ready to submit
	 */
	sg_init_table(&pchannel_p->bdtable[bdindex].sglist, 1);
	sg_dma_address(&pchannel_p->bdtable[bdindex].sglist) = pchannel_p->bdtable[bdindex].dma_handle;
	sg_dma_len(&pchannel_p->bdtable[bdindex].sglist) = pchannel_p->buffer_table_p[bdindex].length;

	chan_desc = dma_device->device_prep_slave_sg(pchannel_p->channel_p, &pchannel_p->bdtable[bdindex].sglist, 1, 
						pchannel_p->direction, flags, NULL);

	if (!chan_desc) {
		printk(KERN_ERR "dmaengine_prep*() error\n");
	} else {
		chan_desc->callback = sync_callback;
		chan_desc->callback_param = &pchannel_p->bdtable[bdindex].cmp;

		/* Initialize the completion for the transfer and before using it
		 * then submit the transaction to the DMA engine so that it's queued
		 * up to be processed later and get a cookie to track it's status
		 */
		init_completion(&pchannel_p->bdtable[bdindex].cmp);

		pchannel_p->bdtable[bdindex].cookie = dmaengine_submit(chan_desc);
		if (dma_submit_error(pchannel_p->bdtable[bdindex].cookie)) {
			printk("Submit error\n");
	 		return;
		}

		/* Start the DMA transaction which was previously queued up in the DMA engine
		 */
		dma_async_issue_pending(pchannel_p->channel_p);
	}
}

/* Wait for a DMA transfer that was previously submitted to the DMA engine
 */
static void wait_for_transfer(struct dma_proxy_channel *pchannel_p)
{
	unsigned long timeout = msecs_to_jiffies(3000);
	enum dma_status status;
	int bdindex = pchannel_p->bdindex;

	pchannel_p->buffer_table_p[bdindex].status = PROXY_BUSY;

	/* Wait for the transaction to complete, or timeout, or get an error
	 */
	timeout = wait_for_completion_timeout(&pchannel_p->bdtable[bdindex].cmp, timeout);
	status = dma_async_is_tx_complete(pchannel_p->channel_p, pchannel_p->bdtable[bdindex].cookie, NULL, NULL);

	if (timeout == 0)  {
		pchannel_p->buffer_table_p[bdindex].status  = PROXY_TIMEOUT;
		printk(KERN_ERR "DMA timed out\n");
	} else if (status != DMA_COMPLETE) {
		pchannel_p->buffer_table_p[bdindex].status = PROXY_ERROR;
		printk(KERN_ERR "DMA returned completion callback status of: %s\n",
			   status == DMA_ERROR ? "error" : "in progress");
	} else
		pchannel_p->buffer_table_p[bdindex].status = PROXY_NO_ERROR;
}

/* The following functions are designed to test the driver from within the device
 * driver without any user space. It uses the first channel buffer for the transmit and receive.
 * If this works but the user application does not then the user application is at fault.
 */
static void tx_test(struct work_struct *local_work)
{
	struct dma_proxy *lp;
	lp = container_of(local_work, struct dma_proxy, work);

	/* Use the 1st buffer for the test
	 */
	lp->channels[TX_CHANNEL].buffer_table_p[0].length = TEST_SIZE;
	lp->channels[TX_CHANNEL].bdindex = 0;

	start_transfer(&lp->channels[TX_CHANNEL]);
	wait_for_transfer(&lp->channels[TX_CHANNEL]);
}

static void test(struct dma_proxy *lp)
{
	int i;

	printk("Starting internal test\n");

	/* Initialize the buffers for the test
	 */
	for (i = 0; i < TEST_SIZE / sizeof(unsigned int); i++) {
		lp->channels[TX_CHANNEL].buffer_table_p[0].buffer[i] = i;
		lp->channels[RX_CHANNEL].buffer_table_p[0].buffer[i] = 0;
	}

	/* Since the transfer function is blocking the transmit channel is started from a worker
	 * thread
	 */
	INIT_WORK(&lp->work, tx_test);
	schedule_work(&lp->work);

	/* Receive the data that was just sent and looped back
	 */
	lp->channels[RX_CHANNEL].buffer_table_p->length = TEST_SIZE;
	lp->channels[TX_CHANNEL].bdindex = 0;

	start_transfer(&lp->channels[RX_CHANNEL]);
	wait_for_transfer(&lp->channels[RX_CHANNEL]);

	/* Verify the receiver buffer matches the transmit buffer to
	 * verify the transfer was good
	 */
	for (i = 0; i < TEST_SIZE / sizeof(unsigned int); i++)
		if (lp->channels[TX_CHANNEL].buffer_table_p[0].buffer[i] !=
			lp->channels[RX_CHANNEL].buffer_table_p[0].buffer[i]) {
			printk("buffers not equal, first index = %d\n", i);
			break;
		}

	printk("Internal test complete\n");
}

/* Map the memory for the channel interface into user space such that user space can
 * access it using coherent memory which will be non-cached for s/w coherent systems
 * such as Zynq 7K or the current default for Zynq MPSOC. MPSOC can be h/w coherent
 * when set up and then the memory will be cached.
 */
static int mmap(struct file *file_p, struct vm_area_struct *vma)
{
	struct dma_proxy_channel *pchannel_p = (struct dma_proxy_channel *)file_p->private_data;

	return dma_mmap_coherent(pchannel_p->dma_device_p, vma,
					   pchannel_p->buffer_table_p, pchannel_p->buffer_phys_addr,
					   vma->vm_end - vma->vm_start);
}

/* Open the device file and set up the data pointer to the proxy channel data for the
 * proxy channel such that the ioctl function can access the data structure later.
 */
static int local_open(struct inode *ino, struct file *file)
{
	file->private_data = container_of(ino->i_cdev, struct dma_proxy_channel, cdev);

	return 0;
}

/* Close the file and there's nothing to do for it
 */
static int release(struct inode *ino, struct file *file)
{
	struct dma_proxy_channel *pchannel_p = (struct dma_proxy_channel *)file->private_data;
	struct dma_device *dma_device = pchannel_p->channel_p->device;

	/* Stop all the activity when the channel is closed assuming this
	 * may help if the application is aborted without normal closure
	 * This is not working and causes an issue that may need investigation in the 
	 * DMA driver at the lower level.
	 */
#if 0
	dma_device->device_terminate_all(pchannel_p->channel_p);
#endif
	return 0;
}

/* Perform I/O control to perform a DMA transfer using the input as an index
 * into the buffer descriptor table such that the application is in control of
 * which buffer to use for the transfer.The BD in this case is only a s/w
 * structure for the proxy driver, not related to the hw BD of the DMA.
 */
static long ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct dma_proxy_channel *pchannel_p = (struct dma_proxy_channel *)file->private_data;
	dma_addr_t test;

	/* Get the bd index from the input argument as all commands require it
	 */
	copy_from_user(&pchannel_p->bdindex, (int *)arg, sizeof(pchannel_p->bdindex)); 

	/* Perform the DMA transfer on the specified channel blocking til it completes
	 */
	switch(cmd) {
		case START_XFER:
			start_transfer(pchannel_p);
			break;
		case FINISH_XFER:
			wait_for_transfer(pchannel_p);
			break;
		case XFER:
			start_transfer(pchannel_p);
			wait_for_transfer(pchannel_p);
			break;
	}

	return 0;
}

static struct file_operations dm_fops = {
	.owner    = THIS_MODULE,
	.open     = local_open,
	.release  = release,
	.unlocked_ioctl = ioctl,
	.mmap	= mmap
};

static char *mem_devnode(struct device *dev, umode_t *mode)
{
	if (mode)
		*mode = 0666;
	return NULL;
}

/* Initialize the driver to be a character device such that is responds to
 * file operations.
 */
static int cdevice_init(struct dma_proxy_channel *pchannel_p, char *name)
{
	int rc;
	char device_name[32] = "dma_proxy";
	static struct class *local_class_p = NULL;

	/* Allocate a character device from the kernel for this driver.
	 */
	rc = alloc_chrdev_region(&pchannel_p->dev_node, 0, 1, "dma_proxy");

	if (rc) {
		dev_err(pchannel_p->dma_device_p, "unable to get a char device number\n");
		return rc;
	}

	/* Initialize the device data structure before registering the character 
	 * device with the kernel.
	 */
	cdev_init(&pchannel_p->cdev, &dm_fops);
	pchannel_p->cdev.owner = THIS_MODULE;
	rc = cdev_add(&pchannel_p->cdev, pchannel_p->dev_node, 1);

	if (rc) {
		dev_err(pchannel_p->dma_device_p, "unable to add char device\n");
		goto init_error1;
	}

	/* Only one class in sysfs is to be created for multiple channels,
	 * create the device in sysfs which will allow the device node
	 * in /dev to be created
	 */
	if (!local_class_p) {
		local_class_p = class_create(THIS_MODULE, DRIVER_NAME);
		local_class_p->devnode = mem_devnode;
		if (IS_ERR(pchannel_p->dma_device_p->class)) {
			dev_err(pchannel_p->dma_device_p, "unable to create class\n");
			rc = ERROR;
			goto init_error2;
		}
	}
	pchannel_p->class_p = local_class_p;

	/* Create the device node in /dev so the device is accessible
	 * as a character device
	 */
	strcat(device_name, name);
	pchannel_p->proxy_device_p = device_create(pchannel_p->class_p, NULL,
					  	 pchannel_p->dev_node, NULL, name);

	if (IS_ERR(pchannel_p->proxy_device_p)) {
		dev_err(pchannel_p->dma_device_p, "unable to create the device\n");
		goto init_error3;
	}

	return 0;

init_error3:
	class_destroy(pchannel_p->class_p);

init_error2:
	cdev_del(&pchannel_p->cdev);

init_error1:
	unregister_chrdev_region(pchannel_p->dev_node, 1);
	return rc;
}

/* Exit the character device by freeing up the resources that it created and
 * disconnecting itself from the kernel.
 */
static void cdevice_exit(struct dma_proxy_channel *pchannel_p)
{
	/* Take everything down in the reverse order
	 * from how it was created for the char device
	 */
	if (pchannel_p->proxy_device_p) {
		device_destroy(pchannel_p->class_p, pchannel_p->dev_node);

		/* If this is the last channel then get rid of the /sys/class/dma_proxy
		 */
		if (total_count == 1)
			class_destroy(pchannel_p->class_p);

		cdev_del(&pchannel_p->cdev);
		unregister_chrdev_region(pchannel_p->dev_node, 1);
	}
}

/* Create a DMA channel by getting a DMA channel from the DMA Engine and then setting
 * up the channel as a character device to allow user space control.
 */
static int create_channel(struct platform_device *pdev, struct dma_proxy_channel *pchannel_p, char *name, u32 direction)
{
	int rc, bd;

	/* Request the DMA channel from the DMA engine and then use the device from
	 * the channel for the proxy channel also.
	 */
	pchannel_p->channel_p = dma_request_slave_channel(&pdev->dev, name);
	printk("request device %s", name);
	if (!pchannel_p->channel_p) {
		dev_err(pchannel_p->dma_device_p, "DMA channel request error\n");
		return ERROR;
	}
	pchannel_p->dma_device_p = &pdev->dev; 

	/* Initialize the character device for the dma proxy channel
	 */
	rc = cdevice_init(pchannel_p, name);
	if (rc) 
		return rc;

	pchannel_p->direction = direction;

	/* Allocate DMA memory that will be shared/mapped by user space, allocating
	 * a set of buffers for the channel with user space specifying which buffer
	 * to use for a transfer..
	 */
	pchannel_p->buffer_table_p = (struct channel_buffer *)
		dmam_alloc_coherent(pchannel_p->dma_device_p,
					sizeof(struct channel_buffer) * BUFFER_COUNT,
					&pchannel_p->buffer_phys_addr, GFP_KERNEL);
	printk(KERN_INFO "Allocating memory, virtual address: %016X physical address: %016X\n", 
			pchannel_p->buffer_table_p, (void *)pchannel_p->buffer_phys_addr);

	/* Initialize each entry in the buffer descriptor table such that the physical address	
	 * address of each buffer is ready to use later.
	 */
	for (bd = 0; bd < BUFFER_COUNT; bd++) 
		pchannel_p->bdtable[bd].dma_handle = (dma_addr_t)(pchannel_p->buffer_phys_addr + 
						(sizeof(struct channel_buffer) * bd) + offsetof(struct channel_buffer, buffer));

	/* The buffer descriptor index into the channel buffers should be specified in each 
	 * ioctl but we will initialize it to be safe.
	 */
	pchannel_p->bdindex = 0;
	if (!pchannel_p->buffer_table_p) {
		dev_err(pchannel_p->dma_device_p, "DMA allocation error\n");
		return ERROR;
	}
	return 0;
}
/* Initialize the dma proxy device driver module.
 */
static int dma_proxy_probe(struct platform_device *pdev)
{
	int rc, i;
	struct dma_proxy *lp;
	struct device *dev = &pdev->dev;

	printk(KERN_INFO "dma_proxy module initialized\n");
	
	lp = (struct dma_proxy *) devm_kmalloc(&pdev->dev, sizeof(struct dma_proxy), GFP_KERNEL);
	if (!lp) {
		dev_err(dev, "Could not allocate proxy device\n");
		return -ENOMEM;
	}
	dev_set_drvdata(dev, lp);

	/* Figure out how many channels there are from the device tree based
	 * on the number of strings in the dma-names property
	 */
	lp->channel_count = device_property_read_string_array(&pdev->dev,
						 "dma-names", NULL, 0);
	if (lp->channel_count <= 0)
		return 0;

	printk("Device Tree Channel Count: %d\r\n", lp->channel_count);

	/* Allocate the memory for channel names and then get the names
    * from the device tree
	 */
	lp->names = devm_kmalloc_array(&pdev->dev, lp->channel_count, 
			sizeof(char *), GFP_KERNEL);
	if (!lp->names)
		return -ENOMEM;

	rc = device_property_read_string_array(&pdev->dev, "dma-names", 
					(const char **)lp->names, lp->channel_count);
	if (rc < 0)
		return rc;
	
	/* Allocate the memory for the channels since the number is known.
	 */
	lp->channels = devm_kmalloc(&pdev->dev,
			sizeof(struct dma_proxy_channel) * lp->channel_count, GFP_KERNEL);
	if (!lp->channels)
		return -ENOMEM;

	/* Create the channels in the proxy. The direction does not matter
	 * as the DMA channel has it inside it and uses it, other than this will not work 
	 * for cyclic mode.
	 */
	for (i = 0; i < lp->channel_count; i++) {
		printk("Creating channel %s\r\n", lp->names[i]);
		rc = create_channel(pdev, &lp->channels[i], lp->names[i], DMA_MEM_TO_DEV);

		if (rc) 
			return rc;
		total_count++;
	}

	if (internal_test)
		test(lp);
	return 0;
}
 
/* Exit the dma proxy device driver module.
 */
static int dma_proxy_remove(struct platform_device *pdev)
{
	int i;
	struct device *dev = &pdev->dev;
	struct dma_proxy *lp = dev_get_drvdata(dev);

	printk(KERN_INFO "dma_proxy module exited\n");

	/* Take care of the char device infrastructure for each
	 * channel except for the last channel. Handle the last
	 * channel seperately.
	 */
	for (i = 0; i < lp->channel_count; i++) { 
		if (lp->channels[i].proxy_device_p)
			cdevice_exit(&lp->channels[i]);
		total_count--;
	}
	/* Take care of the DMA channels and any buffers allocated
	 * for the DMA transfers. The DMA buffers are using managed
	 * memory such that it's automatically done.
	 */
	for (i = 0; i < lp->channel_count; i++)
		if (lp->channels[i].channel_p) {
			lp->channels[i].channel_p->device->device_terminate_all(lp->channels[i].channel_p);
			dma_release_channel(lp->channels[i].channel_p);
		}
	return 0;
}

static const struct of_device_id dma_proxy_of_ids[] = {
	{ .compatible = "xlnx,dma_proxy",},
	{}
};

static struct platform_driver dma_proxy_driver = {
	.driver = {
		.name = "dma_proxy_driver",
		.owner = THIS_MODULE,
		.of_match_table = dma_proxy_of_ids,
	},
	.probe = dma_proxy_probe,
	.remove = dma_proxy_remove,
};

static int __init dma_proxy_init(void)
{
	return platform_driver_register(&dma_proxy_driver);

}

static void __exit dma_proxy_exit(void)
{
	platform_driver_unregister(&dma_proxy_driver);
}

module_init(dma_proxy_init)
module_exit(dma_proxy_exit)

MODULE_AUTHOR("Xilinx, Inc.");
MODULE_DESCRIPTION("DMA Proxy Prototype");
MODULE_LICENSE("GPL v2");

