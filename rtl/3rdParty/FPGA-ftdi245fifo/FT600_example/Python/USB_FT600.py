#-*- coding:utf-8 -*-

import sys
import ftd3xx

class USB_FT600_sync245mode():
    
    def __init__(self):
        usb = None
        if sys.platform == 'win32':
            usb = ftd3xx.create(0, ftd3xx._ftd3xx_win32.FT_OPEN_BY_INDEX)
        elif sys.platform == 'linux2':
            usb = ftd3xx.create(0, ftd3xx._ftd3xx_linux.FT_OPEN_BY_INDEX)
        if usb is None:
            raise Exception("Can't find or open Device!")
        if sys.platform == 'win32' and usb.getDriverVersion() < 0x01020006:
            usb.close()
            raise Exception("Old kernel driver version. Please update driver!")
        if usb.getDeviceDescriptor().bcdUSB < 0x300:
            print("*** Warning: Device is NOT connected using USB3.0 cable or port!")
        cfg = usb.getChipConfiguration()
        numChannel = [4, 2, 1, 0, 0][cfg.ChannelConfig]
        if numChannel != 1:
            usb.close()
            raise Exception("Number of Channels invalid! (numChannel=%d)" % (numChannel,) )
        self._usb = usb
    
    def send(self, data):
        txlength = self._usb.writePipe(0x02, data, len(data))
        return txlength
    
    def recv(self, length):
        data = bytes(length)
        rxlength = self._usb.readPipe(0x82, data, length)
        return data[0:rxlength], rxlength
    
    def close(self):
        self._usb.close()


if __name__ == '__main__':

    usb = USB_FT600_sync245mode()

    usb.close()












