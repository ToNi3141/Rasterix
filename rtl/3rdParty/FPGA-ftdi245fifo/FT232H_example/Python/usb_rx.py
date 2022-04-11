#-*- coding:utf-8 -*-
# Python3

from USB_FT232H import USB_FT232H_sync245mode
   
if __name__ == '__main__':
    usb = USB_FT232H_sync245mode(b'USB <-> Serial Converter')

    data, rxlength = usb.recv(32)
    
    print("recv %d B" % rxlength)
    print(data)

    usb.close()
    