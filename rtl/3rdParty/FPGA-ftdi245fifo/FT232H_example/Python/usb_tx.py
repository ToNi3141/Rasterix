#-*- coding:utf-8 -*-
# Python3

from USB_FT232H import USB_FT232H_sync245mode
   
if __name__ == '__main__':
    usb = USB_FT232H_sync245mode(b'USB <-> Serial Converter')
    
    txlength = usb.send(b'0123456789abcdef')
    
    print("%d B sent" % txlength)

    usb.close()
    