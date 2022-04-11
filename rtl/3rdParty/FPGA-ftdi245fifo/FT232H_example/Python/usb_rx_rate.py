#-*- coding:utf-8 -*-
# Python3

import time

from USB_FT232H import USB_FT232H_sync245mode
   
if __name__ == '__main__':
    usb = USB_FT232H_sync245mode(b'USB <-> Serial Converter')

    print("\n  Reading...")
    
    rxlength_total = 0
    time_start = time.time()
    for ii in range(32):
        for jj in range(16):
            recv_data, rxlength = usb.recv(4096)
            rxlength_total += rxlength
        print("    recv %dB" % rxlength_total)
    
    time_cost = time.time() - time_start
    print("\n  time:%.2fs   rate:%.2fMBps" % (time_cost,rxlength_total/(1+time_cost*1000000.0) ) )

    usb.close()
    