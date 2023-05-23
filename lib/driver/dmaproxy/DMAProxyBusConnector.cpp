#include "DMAProxyBusConnector.hpp"
#include "kernel/dma-proxy.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <stdint.h>
#include <signal.h>
#include <sched.h>
#include <time.h>
#include <errno.h>
#include <sys/param.h>

#include <cstring>


/* The user must tune the application number of channels to match the proxy driver device tree
 * and the names of each channel must match the dma-names in the device tree for the proxy
 * driver node. The number of channels can be less than the number of names as the other
 * channels will just not be used in testing.
 */
#define TX_CHANNEL_COUNT 1
#define RX_CHANNEL_COUNT 1

const char *tx_channel_names[] = { "dma_proxy_tx", /* add unique channel names here */ };
const char *rx_channel_names[] = { "dma_proxy_rx", /* add unique channel names here */ };

/* Internal data which should work without tuning */

struct channel {
    struct channel_buffer *buf_ptr;
    int fd;
};

static int verify;
static int test_size;
static volatile int stop = 0;
int num_transfers;

struct channel tx_channels[TX_CHANNEL_COUNT], rx_channels[RX_CHANNEL_COUNT];

namespace rr
{

DMAProxyBusConnector::DMAProxyBusConnector()
{
    char channel_name[64] = "/dev/";
    strcat(channel_name, tx_channel_names[0]);
    tx_channels[0].fd = open(channel_name, O_RDWR);
    if (tx_channels[0].fd < 1) {
        printf("Unable to open DMA proxy device file: %s\r", channel_name);
        exit(EXIT_FAILURE);
    }
    tx_channels[0].buf_ptr = (struct channel_buffer *)mmap(NULL, sizeof(struct channel_buffer) * TX_BUFFER_COUNT,
                                    PROT_READ | PROT_WRITE, MAP_SHARED, tx_channels[0].fd, 0);
    if (tx_channels[0].buf_ptr == MAP_FAILED) {
        printf("Failed to mmap tx channel\n");
        exit(EXIT_FAILURE);
    }
}

void DMAProxyBusConnector::writeData(const std::span<const uint8_t>& data)
{
    int i, counter = 0, buffer_id = 0, in_progress_count = 0;
    int stop_in_progress = 0;

    tx_channels[0].buf_ptr[buffer_id].length = data.size();
    memcpy(tx_channels[0].buf_ptr[buffer_id].buffer, data.data(), data.size());
    ioctl(tx_channels[0].fd, START_XFER, &buffer_id);

    /* Perform the DMA transfer and check the status after it completes
    * as the call blocks til the transfer is done.
    */
    ioctl(tx_channels[0].fd, FINISH_XFER, &buffer_id);
    if (tx_channels[0].buf_ptr[buffer_id].status != channel_buffer::proxy_status::PROXY_NO_ERROR)
        printf("Proxy tx transfer error\n");
}

bool DMAProxyBusConnector::clearToSend()
{
    return true;
}

} // namespace rr