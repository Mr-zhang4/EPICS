dbLoadDatabase("../../dbd/usb320AsynPortDriver.dbd")
usb320AsynPortDriver_registerRecordDeviceDriver(pdbbase)

# Turn on asynTraceFlow and asynTraceError for global trace, i.e. no connected asynUser.

usb320AsynPortDriverConfigure("futekUSB220", 1000)

dbLoadRecords("../../db/usb320AsynPortDriver.db","P=testAPD:,R=scope1:,PORT=futekUSB220,ADDR=0,TIMEOUT=1,NPOINTS=1000")
dbLoadRecords("../../db/asynRecord.db","P=testAPD:,R=asyn1,PORT=futekUSB220,ADDR=0,OMAX=80,IMAX=80")

#asynSetTraceMask("testAPD",0,0xff)
#asynSetTraceMask("testAPD",0,0xff)
asynSetTraceIOMask("futekUSB220",0,0x2)

iocInit()
