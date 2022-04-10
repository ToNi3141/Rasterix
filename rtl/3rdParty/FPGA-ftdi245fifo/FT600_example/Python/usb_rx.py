#-*- coding:utf-8 -*-
# Python3

from USB_FT600 import USB_FT600_sync245mode
   
if __name__ == '__main__':
    usb = USB_FT600_sync245mode()

    data, rxlength = usb.recv(32)
    
    print("recv %d B" % rxlength)
    print(data)

    usb.close()
    