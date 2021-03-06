/*
 * usb320AsynPortDriver.cpp
 * Based on testAsynPortDriver.cpp
 *
 * Author: Aiyu Zhou
 *
 * Created Apr. 24, 2021
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <algorithm>

#include <epicsTypes.h>
#include <epicsTime.h>
#include <epicsThread.h>
#include <epicsString.h>
#include <epicsTimer.h>
#include <epicsMutex.h>
#include <epicsEvent.h>
#include <iocsh.h>

#include "usb320AsynPortDriver.h"
#include <epicsExport.h>

#define FREQUENCY 1000       /* Frequency in Hz */
#define AMPLITUDE 1.0        /* Plus and minus peaks of sin wave */
#define NUM_DIVISIONS 10     /* Number of scope divisions in X and Y */
#define MIN_UPDATE_TIME 0.02 /* Minimum update time, to prevent CPU saturation */

#define MAX_ENUM_STRING_SIZE 20
#define MAX_FUTEK_DEVICE 5

using namespace std;

static int allVoltsPerDivSelections[NUM_VERT_SELECTIONS]={1,2,5,10};

static const char *driverName="usb320AsynPortDriver";
void simTask(void *drvPvt);

_FUTEK_USB_DLL::FUTEK_USB_DLL futekUSBDll;
int nfutekDevice=0;
string futekDeviceSn[MAX_FUTEK_DEVICE]={"0","0","0","0","0"};
PVOID futekDeviceHandle[MAX_FUTEK_DEVICE]={NULL,NULL,NULL,NULL,NULL};
BYTE futekDeviceChanNum[MAX_FUTEK_DEVICE]={0,0,0,0,0};
usb320AsynPortDriver *futekUSBDevices[MAX_FUTEK_DEVICE]={NULL,NULL,NULL,NULL,NULL};

/** Constructor for the testAsynPortDriver class.
  * Calls constructor for the asynPortDriver base class.
  * \param[in] portName The name of the asyn port driver to be created.
  * \param[in] maxPoints The maximum  number of points in the volt and time arrays */
usb320AsynPortDriver::usb320AsynPortDriver(const char *portName, int maxPoints)
   : asynPortDriver(portName,
                    1, /* maxAddr */
                    asynInt32Mask | asynFloat64Mask | asynFloat64ArrayMask | asynEnumMask | asynDrvUserMask, /* Interface mask */
                    asynInt32Mask | asynFloat64Mask | asynFloat64ArrayMask | asynEnumMask,  /* Interrupt mask */
                    0, /* asynFlags.  This driver does not block and it is not multi-device, so flag is 0 */
                    1, /* Autoconnect */
                    0, /* Default priority */
                    0) /* Default stack size*/
{
    asynStatus status;
    int i;
    const char *functionName = "usb320AsynPortDriver";

    eventId_ = epicsEventCreate(epicsEventEmpty);
    createParam(P_RunString,                asynParamInt32,           &P_Run);
    createParam(P_NormalData0String,         asynParamFloat64,         &P_NormalData0);
    createParam(P_OffsetVal0String,          asynParamFloat64,         &P_OffsetVal0);
    createParam(P_FullscaleVal0String,       asynParamFloat64,         &P_FullscaleVal0);
    createParam(P_FullscaleLoad0String,      asynParamFloat64,         &P_FullscaleLoad0);
    createParam(P_DecimalPoint0String,       asynParamFloat64,         &P_DecimalPoint0);
    createParam(P_UpdateTimeString,         asynParamFloat64,         &P_UpdateTime);
    createParam(P_PressureValue0String,       asynParamFloat64,         &P_PressureValue0);
    createParam(P_SerialNumber0String,       asynParamFloat64,         &P_SerialNumber0);

    createParam(P_NormalData1String,         asynParamFloat64,         &P_NormalData1);
    createParam(P_OffsetVal1String,          asynParamFloat64,         &P_OffsetVal1);
    createParam(P_FullscaleVal1String,       asynParamFloat64,         &P_FullscaleVal1);
    createParam(P_FullscaleLoad1String,      asynParamFloat64,         &P_FullscaleLoad1);
    createParam(P_DecimalPoint1String,       asynParamFloat64,         &P_DecimalPoint1);
    createParam(P_PressureValue1String,       asynParamFloat64,         &P_PressureValue1);
    createParam(P_SerialNumber1String,       asynParamFloat64,         &P_SerialNumber1);

    createParam(P_NormalData2String,         asynParamFloat64,         &P_NormalData2);
    createParam(P_OffsetVal2String,          asynParamFloat64,         &P_OffsetVal2);
    createParam(P_FullscaleVal2String,       asynParamFloat64,         &P_FullscaleVal2);
    createParam(P_FullscaleLoad2String,      asynParamFloat64,         &P_FullscaleLoad2);
    createParam(P_DecimalPoint2String,       asynParamFloat64,         &P_DecimalPoint2);
    createParam(P_PressureValue2String,       asynParamFloat64,         &P_PressureValue2);
    createParam(P_SerialNumber2String,       asynParamFloat64,         &P_SerialNumber2);

    createParam(P_NormalData3String,         asynParamFloat64,         &P_NormalData3);
    createParam(P_OffsetVal3String,          asynParamFloat64,         &P_OffsetVal3);
    createParam(P_FullscaleVal3String,       asynParamFloat64,         &P_FullscaleVal3);
    createParam(P_FullscaleLoad3String,      asynParamFloat64,         &P_FullscaleLoad3);
    createParam(P_DecimalPoint3String,       asynParamFloat64,         &P_DecimalPoint3);
    createParam(P_PressureValue3String,       asynParamFloat64,        &P_PressureValue3);
    createParam(P_SerialNumber3String,       asynParamFloat64,         &P_SerialNumber3);


    createParam(P_NormalData4String,         asynParamFloat64,         &P_NormalData4);
    createParam(P_OffsetVal4String,          asynParamFloat64,         &P_OffsetVal4);
    createParam(P_FullscaleVal4String,       asynParamFloat64,         &P_FullscaleVal4);
    createParam(P_FullscaleLoad4String,      asynParamFloat64,         &P_FullscaleLoad4);
    createParam(P_DecimalPoint4String,       asynParamFloat64,         &P_DecimalPoint4);
    createParam(P_PressureValue4String,      asynParamFloat64,         &P_PressureValue4);
    createParam(P_SerialNumber4String,       asynParamFloat64,         &P_SerialNumber4);

    /* Set the initial values of some parameters */
    setIntegerParam(P_Run,               0);
    setDoubleParam(P_NormalData0,        12.0);
    setDoubleParam(P_UpdateTime,        1.0);

//    printf("P_NoiseAmplitude is:%f\r\n",P_NoiseAmplitude);
    printf("P_NormalData is:%d\r\n",P_NormalData0);

    string dataString = futekUSBDll.Normal_Data_Request(futekDeviceHandle[0],futekDeviceChanNum[0]);
    printf("Device normal data is:%s\r\n", dataString.c_str());
    char *dataChar = new char[dataString.length() + 1];
    strcpy(dataChar, dataString.c_str());
    char *end;
    double douValue=strtof(dataChar,&end);
    setDoubleParam(P_NormalData0, douValue);

    printf("P_NormalData0 is:%d\r\n",P_NormalData0);
    double data;
    getDoubleParam(P_NormalData0,       &data);
    printf("normalData is:%f\r\n",data);



    /* Create the thread that computes the waveforms in the background */
    status = (asynStatus)(epicsThreadCreate("testAsynPortDriverTask",
                          epicsThreadPriorityMedium,
                          epicsThreadGetStackSize(epicsThreadStackMedium),
                          (EPICSTHREADFUNC)::simTask,
                          this) == NULL);
    if (status) {
        printf("%s:%s: epicsThreadCreate failure\n", driverName, functionName);
        return;
    }
}



void simTask(void *drvPvt)
{
    usb320AsynPortDriver *pPvt = (usb320AsynPortDriver *)drvPvt;

    pPvt->simTask();
}

/** Simulation task that runs as a separate thread.  When the P_Run parameter is set to 1
  * to rub the simulation it computes a 1 kHz sine wave with 1V amplitude and user-controllable
  * noise, and displays it on
  * a simulated scope.  It computes waveforms for the X (time) and Y (volt) axes, and computes
  * statistics about the waveform. */
void usb320AsynPortDriver::simTask(void)
{
    /* This thread computes the waveform and does callbacks with it */

    double updateTime;
    epicsInt32 run;
   
    double normalData0;

    lock();
    /* Loop forever */
    while (1) {
//        getIntegerParam(P_Run, &run);
        // Release the lock while we wait for a command to start or wait for updateTime
        unlock();
        run=1;
//        updateTime=0.5;
        getDoubleParam(P_UpdateTime,       &updateTime);
        if (run) epicsEventWaitWithTimeout(eventId_, updateTime);
        else     (void) epicsEventWait(eventId_);
        // Take the lock again
        lock();
        /* run could have changed while we were waiting */
//        getIntegerParam(P_Run, &run);
        if (!run) continue;
        getDoubleParam(P_NormalData0,       &normalData0);
        printf("P_NormalData0 is:%d\r\n",P_NormalData0);
        printf("normalData0 is:%d\r\n",normalData0);

        string dataString,offsetValString,fullScaleValString,fullScaleLoadString,decPointString;
        char *dataChar,*offsetChar,*fullScaleValChar,*fullScaleLoadChar,*decPointChar,*end,*serialNumberChar;
        double dataDou,offsetValDou,fullScaleValDou,fullScaleLoadDou,decPointDou,pressureDou,serialNumberDou;
        double dataRead,offsetValRead,fullScaleValRead,fullScaleLoadRead,decPointRead,pressureRead,serialNumberRead;
        double tempF,tempX,tempY;
        for(int i=0;i<nfutekDevice;i++){
 /*         serialNumberString = futekUSBDll.Get_Device_Serial_Number(PVOID (i));
          printf("Device Serial Number string is:%s\r\n", serialNumberString.c_str());
          serialNumberChar = new char[serialNumberString.length() + 1];          
          strcpy(serialNumberChar, serialNumberString.c_str());
          serialNumberDou=strtof(serialNumberChar,&end);
*/
        //  serialNumberString = futekUSBDll.Get_Device_Serial_Number((PVOID) futekDeviceSn[nfutekDevice]);
          printf("Device Serial Number string is:%s\r\n", futekDeviceSn[i].c_str());
          serialNumberChar = new char[futekDeviceSn[i].length() + 1];          
          strcpy(serialNumberChar, futekDeviceSn[i].c_str());
          serialNumberDou=strtof(serialNumberChar,&end);

          dataString = futekUSBDll.Normal_Data_Request(futekDeviceHandle[i],futekDeviceChanNum[i]);
          printf("Device normal data string is:%s\r\n", dataString.c_str());
          dataChar = new char[dataString.length() + 1];
          strcpy(dataChar, dataString.c_str());
          dataDou=strtof(dataChar,&end);

          offsetValString = futekUSBDll.Get_Offset_Value(futekDeviceHandle[i],futekDeviceChanNum[i]);
          printf("Offset value string is:%s\r\n", offsetValString.c_str());
          offsetChar = new char[offsetValString.length() + 1];
          strcpy(offsetChar, offsetValString.c_str());
          offsetValDou=strtof(offsetChar,&end);

          fullScaleValString = futekUSBDll.Get_Fullscale_Value(futekDeviceHandle[i],futekDeviceChanNum[i]);
          printf("FullScale value string is:%s\r\n", fullScaleValString.c_str());
          fullScaleValChar = new char[fullScaleValString.length() + 1];
          strcpy(fullScaleValChar, fullScaleValString.c_str());
          fullScaleValDou=strtof(fullScaleValChar,&end);

          fullScaleLoadString = futekUSBDll.Get_Fullscale_Load(futekDeviceHandle[i],futekDeviceChanNum[i]);
          printf("FullScale load string is:%s\r\n", fullScaleLoadString.c_str());
          fullScaleLoadChar = new char[fullScaleLoadString.length() + 1];
          strcpy(fullScaleLoadChar, fullScaleLoadString.c_str());
          fullScaleLoadDou=strtof(fullScaleLoadChar,&end);

          decPointString = futekUSBDll.Get_Decimal_Point(futekDeviceHandle[i],futekDeviceChanNum[i]);
          printf("Decimal point string is:%s\r\n", decPointString.c_str());
          decPointChar = new char[decPointString.length() + 1];
          strcpy(decPointChar, decPointString.c_str());
          decPointDou=strtof(decPointChar,&end);
          
          tempF=pow(10,decPointDou);
          tempX=dataDou-offsetValDou;
          tempY=fullScaleValDou-offsetValDou;
          pressureDou=tempX/tempY*fullScaleLoadDou/tempF;

          switch(i)
          {
            case 0:
           
              setDoubleParam(P_SerialNumber0, serialNumberDou);
              printf("P_SerialNumber0 is:%d\r\n",P_SerialNumber0);
              getDoubleParam(P_SerialNumber0,       &dataRead);
              printf("SerialNumber0 is:%f\r\n",serialNumberRead);
   
              setDoubleParam(P_NormalData0, dataDou);
              printf("P_NormalData0 is:%d\r\n",P_NormalData0);
              getDoubleParam(P_NormalData0,       &dataRead);
              printf("normalData0 is:%f\r\n",dataRead);

              setDoubleParam(P_OffsetVal0, offsetValDou);
              printf("P_OffsetVal0 is:%d\r\n",P_OffsetVal0);
              getDoubleParam(P_OffsetVal0,       &offsetValRead);
              printf("offsetVal0 is:%f\r\n",offsetValRead);

              setDoubleParam(P_FullscaleVal0, fullScaleValDou);
              printf("P_FullscaleVal0 is:%d\r\n",P_FullscaleVal0);
              getDoubleParam(P_FullscaleVal0,       &fullScaleValRead);
              printf("fullScaleVal0 is:%f\r\n",fullScaleValRead);

              setDoubleParam(P_FullscaleLoad0, fullScaleLoadDou);
              printf("P_FullscaleLoad0 is:%d\r\n",P_FullscaleLoad0);
              getDoubleParam(P_FullscaleLoad0,       &fullScaleLoadRead);
              printf("fullScaleLoad0 is:%f\r\n",fullScaleLoadRead);

              setDoubleParam(P_DecimalPoint0, decPointDou);
              printf("P_DecimalPoint0 is:%d\r\n",P_DecimalPoint0);
              getDoubleParam(P_DecimalPoint0,       &decPointRead);
              printf("decPoint0 is:%f\r\n",decPointRead);

              setDoubleParam(P_PressureValue0, pressureDou);
              printf("P_PressureValue0 is:%d\r\n",P_PressureValue0);
              getDoubleParam(P_PressureValue0,       &pressureRead);
              printf("pressure0 is:%f\r\n",pressureRead);
              break;
            case 1:
              setDoubleParam(P_SerialNumber1, serialNumberDou);
              printf("P_SerialNumber1 is:%d\r\n",P_SerialNumber1);
              getDoubleParam(P_SerialNumber1,       &dataRead);
              printf("SerialNumber1 is:%f\r\n",serialNumberRead);

              setDoubleParam(P_NormalData1, dataDou);
              printf("P_NormalData1 is:%d\r\n",P_NormalData1);
              getDoubleParam(P_NormalData1,       &dataRead);
              printf("normalData1 is:%f\r\n",dataRead);

              setDoubleParam(P_OffsetVal1, offsetValDou);
              printf("P_OffsetVal1 is:%d\r\n",P_OffsetVal1);
              getDoubleParam(P_OffsetVal1,       &offsetValRead);
              printf("offsetVal1 is:%f\r\n",offsetValRead);

              setDoubleParam(P_FullscaleVal1, fullScaleValDou);
              printf("P_FullscaleVal1 is:%d\r\n",P_FullscaleVal1);
              getDoubleParam(P_FullscaleVal1,       &fullScaleValRead);
              printf("fullScaleVal1 is:%f\r\n",fullScaleValRead);

              setDoubleParam(P_FullscaleLoad1, fullScaleLoadDou);
              printf("P_FullscaleLoad1 is:%d\r\n",P_FullscaleLoad1);
              getDoubleParam(P_FullscaleLoad1,       &fullScaleLoadRead);
              printf("fullScaleLoad1 is:%f\r\n",fullScaleLoadRead);

              setDoubleParam(P_DecimalPoint1, decPointDou);
              printf("P_DecimalPoint1 is:%d\r\n",P_DecimalPoint1);
              getDoubleParam(P_DecimalPoint1,       &decPointRead);
              printf("decPoint1 is:%f\r\n",decPointRead);

              setDoubleParam(P_PressureValue1, pressureDou);
              printf("P_PressureValue1 is:%d\r\n",P_PressureValue1);
              getDoubleParam(P_PressureValue1,       &pressureRead);
              printf("pressure1 is:%f\r\n",pressureRead);
              break;
            case 2:
              setDoubleParam(P_SerialNumber2, serialNumberDou);
              printf("P_SerialNumber2 is:%d\r\n",P_SerialNumber2);
              getDoubleParam(P_SerialNumber2,       &dataRead);
              printf("SerialNumber2 is:%f\r\n",serialNumberRead);

              setDoubleParam(P_NormalData2, dataDou);
              printf("P_NormalData2 is:%d\r\n",P_NormalData2);
              getDoubleParam(P_NormalData2,       &dataRead);
              printf("normalData2 is:%f\r\n",dataRead);

              setDoubleParam(P_OffsetVal2, offsetValDou);
              printf("P_OffsetVal2 is:%d\r\n",P_OffsetVal2);
              getDoubleParam(P_OffsetVal2,       &offsetValRead);
              printf("offsetVal2 is:%f\r\n",offsetValRead);

              setDoubleParam(P_FullscaleVal2, fullScaleValDou);
              printf("P_FullscaleVal2 is:%d\r\n",P_FullscaleVal2);
              getDoubleParam(P_FullscaleVal2,       &fullScaleValRead);
              printf("fullScaleVal2 is:%f\r\n",fullScaleValRead);

              setDoubleParam(P_FullscaleLoad2, fullScaleLoadDou);
              printf("P_FullscaleLoad2 is:%d\r\n",P_FullscaleLoad2);
              getDoubleParam(P_FullscaleLoad2,       &fullScaleLoadRead);
              printf("fullScaleLoad2 is:%f\r\n",fullScaleLoadRead);

              setDoubleParam(P_DecimalPoint2, decPointDou);
              printf("P_DecimalPoint2 is:%d\r\n",P_DecimalPoint2);
              getDoubleParam(P_DecimalPoint2,       &decPointRead);
              printf("decPoint2 is:%f\r\n",decPointRead);

              setDoubleParam(P_PressureValue2, pressureDou);
              printf("P_PressureValue2 is:%d\r\n",P_PressureValue2);
              getDoubleParam(P_PressureValue2,       &pressureRead);
              printf("pressure2 is:%f\r\n",pressureRead);
              break;
            case 3:
              setDoubleParam(P_SerialNumber3, serialNumberDou);
              printf("P_SerialNumber3 is:%d\r\n",P_SerialNumber3);
              getDoubleParam(P_SerialNumber3,       &dataRead);
              printf("SerialNumber3 is:%f\r\n",serialNumberRead);

              setDoubleParam(P_NormalData3, dataDou);
              printf("P_NormalData3 is:%d\r\n",P_NormalData3);
              getDoubleParam(P_NormalData3,       &dataRead);
              printf("normalData3 is:%f\r\n",dataRead);

              setDoubleParam(P_OffsetVal3, offsetValDou);
              printf("P_OffsetVal3 is:%d\r\n",P_OffsetVal3);
              getDoubleParam(P_OffsetVal3,       &offsetValRead);
              printf("offsetVal3 is:%f\r\n",offsetValRead);

              setDoubleParam(P_FullscaleVal3, fullScaleValDou);
              printf("P_FullscaleVal3 is:%d\r\n",P_FullscaleVal3);
              getDoubleParam(P_FullscaleVal3,       &fullScaleValRead);
              printf("fullScaleVal3 is:%f\r\n",fullScaleValRead);

              setDoubleParam(P_FullscaleLoad3, fullScaleLoadDou);
              printf("P_FullscaleLoad3 is:%d\r\n",P_FullscaleLoad3);
              getDoubleParam(P_FullscaleLoad3,       &fullScaleLoadRead);
              printf("fullScaleLoad3 is:%f\r\n",fullScaleLoadRead);

              setDoubleParam(P_DecimalPoint3, decPointDou);
              printf("P_DecimalPoint3 is:%d\r\n",P_DecimalPoint3);
              getDoubleParam(P_DecimalPoint3,       &decPointRead);
              printf("decPoint3 is:%f\r\n",decPointRead);

              setDoubleParam(P_PressureValue3, pressureDou);
              printf("P_PressureValue3 is:%d\r\n",P_PressureValue3);
              getDoubleParam(P_PressureValue3,       &pressureRead);
              printf("pressure3 is:%f\r\n",pressureRead);
              break;
           case 4:
              setDoubleParam(P_SerialNumber4, serialNumberDou);
              printf("P_SerialNumber4 is:%d\r\n",P_SerialNumber4);
              getDoubleParam(P_SerialNumber4,       &dataRead);
              printf("SerialNumber4 is:%f\r\n",serialNumberRead);

              setDoubleParam(P_NormalData4, dataDou);
              printf("P_NormalData4 is:%d\r\n",P_NormalData4);
              getDoubleParam(P_NormalData4,       &dataRead);
              printf("normalData4 is:%f\r\n",dataRead);

              setDoubleParam(P_OffsetVal4, offsetValDou);
              printf("P_OffsetVal4 is:%d\r\n",P_OffsetVal4);
              getDoubleParam(P_OffsetVal4,       &offsetValRead);
              printf("offsetVal4 is:%f\r\n",offsetValRead);

              setDoubleParam(P_FullscaleVal4, fullScaleValDou);
              printf("P_FullscaleVal4 is:%d\r\n",P_FullscaleVal4);
              getDoubleParam(P_FullscaleVal4,       &fullScaleValRead);
              printf("fullScaleVal4 is:%f\r\n",fullScaleValRead);

              setDoubleParam(P_FullscaleLoad4, fullScaleLoadDou);
              printf("P_FullscaleLoad4 is:%d\r\n",P_FullscaleLoad4);
              getDoubleParam(P_FullscaleLoad4,       &fullScaleLoadRead);
              printf("fullScaleLoad4 is:%f\r\n",fullScaleLoadRead);

              setDoubleParam(P_DecimalPoint4, decPointDou);
              printf("P_DecimalPoint4 is:%d\r\n",P_DecimalPoint4);
              getDoubleParam(P_DecimalPoint4,       &decPointRead);
              printf("decPoint4 is:%f\r\n",decPointRead);

              setDoubleParam(P_PressureValue4, pressureDou);
              printf("P_PressureValue1 is:%d\r\n",P_PressureValue4);
              getDoubleParam(P_PressureValue4,       &pressureRead);
              printf("pressure4 is:%f\r\n",pressureRead);
              break;
            default:
              break;
          }
        }
        callParamCallbacks();
//        doCallbacksFloat64Array(pData_, maxPoints, P_Waveform, 0);
    }
}

/** Called when asyn clients call pasynInt32->write().
  * This function sends a signal to the simTask thread if the value of P_Run has changed.
  * For all parameters it sets the value in the parameter library and calls any registered callbacks..
  * \param[in] pasynUser pasynUser structure that encodes the reason and address.
  * \param[in] value Value to write. */
asynStatus usb320AsynPortDriver::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    int function = pasynUser->reason;
    asynStatus status = asynSuccess;
    const char *paramName;
    const char* functionName = "writeInt32";

    /* Set the parameter in the parameter library. */
    status = (asynStatus) setIntegerParam(function, value);

    /* Fetch the parameter string name for possible use in debugging */
    getParamName(function, &paramName);

    if (function == P_Run) {
        /* If run was set then wake up the simulation task */
        if (value) epicsEventSignal(eventId_);
    }
/*    else if (function == P_VertGainSelect) {
        setVertGain();
    }
    else if (function == P_VoltsPerDivSelect) {
        setVoltsPerDiv();
    }
    else if (function == P_TimePerDivSelect) {
        setTimePerDiv();
    }*/
    else {
        /* All other parameters just get set in parameter list, no need to
         * act on them here */
    }

    /* Do callbacks so higher layers see any changes */
    status = (asynStatus) callParamCallbacks();

    if (status)
        epicsSnprintf(pasynUser->errorMessage, pasynUser->errorMessageSize,
                  "%s:%s: status=%d, function=%d, name=%s, value=%d",
                  driverName, functionName, status, function, paramName, value);
    else
        asynPrint(pasynUser, ASYN_TRACEIO_DRIVER,
              "%s:%s: function=%d, name=%s, value=%d\n",
              driverName, functionName, function, paramName, value);
    return status;
}

/** Called when asyn clients call pasynFloat64->write().
  * This function sends a signal to the simTask thread if the value of P_UpdateTime has changed.
  * For all  parameters it  sets the value in the parameter library and calls any registered callbacks.
  * \param[in] pasynUser pasynUser structure that encodes the reason and address.
  * \param[in] value Value to write. */
asynStatus usb320AsynPortDriver::writeFloat64(asynUser *pasynUser, epicsFloat64 value)
{
    int function = pasynUser->reason;
    asynStatus status = asynSuccess;
    epicsInt32 run;
    const char *paramName;
    const char* functionName = "writeFloat64";

    /* Set the parameter in the parameter library. */
    status = (asynStatus) setDoubleParam(function, value);

    /* Fetch the parameter string name for possible use in debugging */
    getParamName(function, &paramName);

//    if (function == P_UpdateTime) {
        /* Make sure the update time is valid. If not change it and put back in parameter library */
//        if (value < MIN_UPDATE_TIME) {
//            asynPrint(pasynUser, ASYN_TRACE_WARNING,
//                "%s:%s: warning, update time too small, changed from %f to %f\n",
//                driverName, functionName, value, MIN_UPDATE_TIME);
//            value = MIN_UPDATE_TIME;
//            setDoubleParam(P_UpdateTime, value);
//        }
        /* If the update time has changed and we are running then wake up the simulation task */
//        getIntegerParam(P_Run, &run);
//        if (run) epicsEventSignal(eventId_);
//    } else {
        /* All other parameters just get set in parameter list, no need to
         * act on them here */
//    }

    /* Do callbacks so higher layers see any changes */
    status = (asynStatus) callParamCallbacks();

    if (status)
        epicsSnprintf(pasynUser->errorMessage, pasynUser->errorMessageSize,
                  "%s:%s: status=%d, function=%d, name=%s, value=%f",
                  driverName, functionName, status, function, paramName, value);
    else
        asynPrint(pasynUser, ASYN_TRACEIO_DRIVER,
              "%s:%s: function=%d, name=%s, value=%f\n",
              driverName, functionName, function, paramName, value);
    return status;
}


/** Called when asyn clients call pasynFloat64Array->read().
  * Returns the value of the P_Waveform or P_TimeBase arrays.
  * \param[in] pasynUser pasynUser structure that encodes the reason and address.
  * \param[in] value Pointer to the array to read.
  * \param[in] nElements Number of elements to read.
  * \param[out] nIn Number of elements actually read. */
asynStatus usb320AsynPortDriver::readFloat64Array(asynUser *pasynUser, epicsFloat64 *value,
                                         size_t nElements, size_t *nIn)
{
    int function = pasynUser->reason;
    size_t ncopy;
    epicsInt32 itemp;
    asynStatus status = asynSuccess;
    epicsTimeStamp timeStamp;
    const char *functionName = "readFloat64Array";

    if (status)
        epicsSnprintf(pasynUser->errorMessage, pasynUser->errorMessageSize,
                  "%s:%s: status=%d, function=%d",
                  driverName, functionName, status, function);
    else
        asynPrint(pasynUser, ASYN_TRACEIO_DRIVER,
              "%s:%s: function=%d\n",
              driverName, functionName, function);
    return status;
}

asynStatus usb320AsynPortDriver::readEnum(asynUser *pasynUser, char *strings[], int values[], int severities[], size_t nElements, size_t *nIn)
{
    return asynSuccess;
}

/* Configuration routine.  Called directly, or from the iocsh function below */

extern "C" {

/** EPICS iocsh callable function to call constructor for the testAsynPortDriver class.
  * \param[in] portName The name of the asyn port driver to be created.
  * \param[in] maxPoints The maximum  number of points in the volt and time arrays */

int usb320AsynPortDriverConfigure(const char *portName, int maxPoints)
{
    string countStr=futekUSBDll.Get_Device_Count();
    int count=atoi(countStr.c_str());
    double data;
    PVOID tempDeviceHandle;
    printf("Count of futek USB devices is: %d\r\n", count);
    if(count>0){
      PVOID tempChar;
      for(int i=0;i<count;i++){
        tempChar=(PVOID)i;
        string sn=futekUSBDll.Get_Device_Serial_Number(tempChar);
        futekDeviceSn[nfutekDevice]=sn;
        if(i==0)
          futekUSBDevices[nfutekDevice]=new usb320AsynPortDriver(portName, maxPoints);
        nfutekDevice+=1;
        printf("%d futek devices are found!\r\n", nfutekDevice);
        }
      sort(futekDeviceSn,futekDeviceSn+nfutekDevice);
      for(int i=0;i<count;i++)
     {
        printf("Device serial number is:%s\r\n",futekDeviceSn[i].c_str());
        char *serialNumber = new char[futekDeviceSn[i].length() + 1];
        strcpy(serialNumber, futekDeviceSn[i].c_str());
        futekUSBDll.Open_Device_Connection(serialNumber);
        tempDeviceHandle = futekUSBDll.DeviceHandle;
        printf("device handle is:%d\r\n",*(int*)tempDeviceHandle);
        printf("device handle is:%d\r\n",*(int*)futekUSBDll.DeviceHandle);
        futekDeviceHandle[i]=tempDeviceHandle;
        string boardType = futekUSBDll.Get_Type_of_Board(tempDeviceHandle);
        printf("Device board type is:%s\r\n", boardType.c_str());
        string boardType1 = futekUSBDll.Get_Type_of_Board(futekDeviceHandle[i]);
        printf("Device board type 1 is:%s\r\n", boardType1.c_str());
        string value = futekUSBDll.Get_ADC_Sampling_Rate(futekDeviceHandle[i],futekDeviceChanNum[i]);
        printf("Device ADC sampling rate is:%s\r\n", value.c_str());      
    }
}    
    else{
      printf("Error. No futek device is found.\r\n");
      return(asynError);
    }
    return(asynSuccess);
}


/* EPICS iocsh shell commands */

static const iocshArg initArg0 = { "portName",iocshArgString};
static const iocshArg initArg1 = { "max points",iocshArgInt};
static const iocshArg * const initArgs[] = {&initArg0,
                                            &initArg1};
static const iocshFuncDef initFuncDef = {"usb320AsynPortDriverConfigure",2,initArgs};
static void initCallFunc(const iocshArgBuf *args)
{
    usb320AsynPortDriverConfigure(args[0].sval, args[1].ival);
}

void usb320AsynPortDriverRegister(void)
{
    iocshRegister(&initFuncDef,initCallFunc);
}

epicsExportRegistrar(usb320AsynPortDriverRegister);

}

