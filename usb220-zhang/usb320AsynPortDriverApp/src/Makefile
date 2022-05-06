TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

DBD += usb320AsynPortDriver.dbd
LIBRARY_IOC += usb320AsynPortDriverSupport
usb320AsynPortDriverSupport_SRCS += usb320AsynPortDriver.cpp
usb320AsynPortDriverSupport_LIBS += asyn
usb320AsynPortDriverSupport_LIBS += $(EPICS_BASE_IOC_LIBS)

usb320AsynPortDriverSupport_LIBS += ftd2xx
usb320AsynPortDriverSupport_LIBS += FUTEK_USB-x86_64

#=============================

PROD_IOC += usb320AsynPortDriver

# <name>_registerRecordDeviceDriver.cpp will be created from <name>.dbd
usb320AsynPortDriver_SRCS_DEFAULT += usb320AsynPortDriver_registerRecordDeviceDriver.cpp usb320AsynPortDriverMain.cpp
usb320AsynPortDriverVx_SRCS_vxWorks += usb320AsynPortDriver_registerRecordDeviceDriver.cpp
usb320AsynPortDriver_LIBS += usb320AsynPortDriverSupport asyn
usb320AsynPortDriver_LIBS += $(EPICS_BASE_IOC_LIBS)

usb320AsynPortDriver_LIBS += ftd2xx
usb320AsynPortDriver_LIBS += FUTEK_USB-x86_64

usb320AsynPortDriver_OBJS_vxWorks += $(EPICS_BASE_BIN)/vxComLibrary

#===========================

include $(TOP)/configure/RULES

#----------------------------------------
# ADD RULES AFTER THIS LINE
