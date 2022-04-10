#-*- coding:utf-8 -*-
# Python3

from USB_FT600 import USB_FT600_sync245mode
   
if __name__ == '__main__':
    usb = USB_FT600_sync245mode()
    
    txlength = usb.send(b'0123456789abcdef')
    
    print("%d B sent" % txlength)

    usb.close()
    