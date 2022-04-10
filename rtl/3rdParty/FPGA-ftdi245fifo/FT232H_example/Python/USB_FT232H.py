#-*- coding:utf-8 -*-

import ftd2xx

BUFFER_SIZE = 65536

class USB_FT232H_sync245mode():
    
    def __init__(self, name_byte_array):
        self._usb = None
        for i in range(16):
            try:
                usb = ftd2xx.open(i)
            except:
                continue
            if usb.description == name_byte_array:
                usb.setTimeouts(1000,1000)
                usb.setUSBParameters(BUFFER_SIZE, BUFFER_SIZE)
                usb.setBitMode(0xff, 0x40)
                print('\n  opened usb[%d]: %s\n' % (i, str(usb.description, encoding = "utf8"),) )
                self._usb = usb
                return
            else:
                usb.close()
        raise Exception('could not open USB device: %s' % str(name_byte_array, encoding = "utf8"))
    
    def send(self, data):
        return self._usb.write(data)
    
    def recv(self, length):
        data = self._usb.read(length)
        return data, len(data)
    
    def close(self):
        self._usb.close()


if __name__ == '__main__':

    usb = USB_FT232H_sync245mode(b'USB <-> Serial Converter')

    usb.close()












