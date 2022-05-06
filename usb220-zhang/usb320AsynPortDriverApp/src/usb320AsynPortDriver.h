/*
 * usb320AsynPortDriver.h
 *
 * Based on tempAsynPortDriver.h
 *
 * Author: Aiyu Zhou
 *
 * Created Apr. 24, 2021
 */

#include "asynPortDriver.h"
#include "FUTEK_USB_DLL.h"
#include "string.h"

using namespace std;
#define NUM_VERT_SELECTIONS 4


/* These are the drvInfo strings that are used to identify the parameters.
 * They are used by asyn clients, including standard asyn device support */
#define P_RunString                "RUN"                  /* asynInt32,    r/w */
#define P_NormalData0String         "NORMAL_DATA_0"                /* asynFloat64,    r/w */
#define P_OffsetVal0String          "OFFSET_VALUE_0"               /* asynFloat64,    r/w */
#define P_FullscaleVal0String       "FULLSCALE_VALUE_0"            /* asynFloat64,    r/w */
#define P_FullscaleLoad0String      "FULLSCALE_LOAD_0"             /* asynFloat64,    r/w */
#define P_DecimalPoint0String       "DECIMAL_POINT_0"              /* asynFloat64,    r/w */
#define P_UpdateTimeString         "UPDATE_TIME"                /* asynFloat64,    r/w */
#define P_PressureValue0String       "PRESSURE_0"              /* asynFloat64,    r/w */
#define P_SerialNumber0String       "SERIAL_NUMBER_0"              /* asynFloat64,    r/w */

#define P_NormalData1String         "NORMAL_DATA_1"                /* asynFloat64,    r/w */
#define P_OffsetVal1String          "OFFSET_VALUE_1"               /* asynFloat64,    r/w */
#define P_FullscaleVal1String       "FULLSCALE_VALUE_1"            /* asynFloat64,    r/w */
#define P_FullscaleLoad1String      "FULLSCALE_LOAD_1"             /* asynFloat64,    r/w */
#define P_DecimalPoint1String       "DECIMAL_POINT_1"              /* asynFloat64,    r/w */
#define P_PressureValue1String       "PRESSURE_1"              /* asynFloat64,    r/w */
#define P_SerialNumber1String       "SERIAL_NUMBER_1"              /* asynFloat64,    r/w */

#define P_NormalData2String         "NORMAL_DATA_2"                /* asynFloat64,    r/w */
#define P_OffsetVal2String          "OFFSET_VALUE_2"               /* asynFloat64,    r/w */
#define P_FullscaleVal2String       "FULLSCALE_VALUE_2"            /* asynFloat64,    r/w */
#define P_FullscaleLoad2String      "FULLSCALE_LOAD_2"             /* asynFloat64,    r/w */
#define P_DecimalPoint2String       "DECIMAL_POINT_2"              /* asynFloat64,    r/w */
#define P_PressureValue2String       "PRESSURE_2"              /* asynFloat64,    r/w */
#define P_SerialNumber2String       "SERIAL_NUMBER_2"              /* asynFloat64,    r/w */

#define P_NormalData3String         "NORMAL_DATA_3"                /* asynFloat64,    r/w */
#define P_OffsetVal3String          "OFFSET_VALUE_3"               /* asynFloat64,    r/w */
#define P_FullscaleVal3String       "FULLSCALE_VALUE_3"            /* asynFloat64,    r/w */
#define P_FullscaleLoad3String      "FULLSCALE_LOAD_3"             /* asynFloat64,    r/w */
#define P_DecimalPoint3String       "DECIMAL_POINT_3"              /* asynFloat64,    r/w */
#define P_PressureValue3String       "PRESSURE_3"              /* asynFloat64,    r/w */
#define P_SerialNumber3String       "SERIAL_NUMBER_3"              /* asynFloat64,    r/w */

#define P_NormalData4String         "NORMAL_DATA_4"                /* asynFloat64,    r/w */
#define P_OffsetVal4String          "OFFSET_VALUE_4"               /* asynFloat64,    r/w */
#define P_FullscaleVal4String       "FULLSCALE_VALUE_4"            /* asynFloat64,    r/w */
#define P_FullscaleLoad4String      "FULLSCALE_LOAD_4"             /* asynFloat64,    r/w */
#define P_DecimalPoint4String       "DECIMAL_POINT_4"              /* asynFloat64,    r/w */
#define P_PressureValue4String       "PRESSURE_4"              /* asynFloat64,    r/w */
#define P_SerialNumber4String       "SERIAL_NUMBER_4"              /* asynFloat64,    r/w */
/** Class that demonstrates the use of the asynPortDriver base class to greatly simplify the task
  * of writing an asyn port driver.
  * This class does a simple simulation of a digital oscilloscope.  It computes a waveform, computes
  * statistics on the waveform, and does callbacks with the statistics and the waveform data itself.
  * I have made the methods of this class public in order to generate doxygen documentation for them,
  * but they should really all be private. */
class usb320AsynPortDriver : public asynPortDriver {
public:
    usb320AsynPortDriver(const char *portName, int maxArraySize);

    /* These are the methods that we override from asynPortDriver */
    virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);
    virtual asynStatus writeFloat64(asynUser *pasynUser, epicsFloat64 value);
    virtual asynStatus readFloat64Array(asynUser *pasynUser, epicsFloat64 *value,
                                        size_t nElements, size_t *nIn);
    virtual asynStatus readEnum(asynUser *pasynUser, char *strings[], int values[], int severities[],
                                size_t nElements, size_t *nIn);

    /* These are the methods that are new to this class */
    void simTask(void);

protected:
    /** Values used for pasynUser->reason, and indexes into the parameter library. */
    int P_Run;
    int P_NormalData0;
    int P_OffsetVal0;
    int P_FullscaleVal0;
    int P_FullscaleLoad0;
    int P_DecimalPoint0; 
    int P_PressureValue0; 
    int P_UpdateTime;
    int P_SerialNumber0;

    int P_NormalData1;
    int P_OffsetVal1;
    int P_FullscaleVal1;
    int P_FullscaleLoad1;
    int P_PressureValue1; 
    int P_DecimalPoint1;
    int P_SerialNumber1;

    int P_NormalData2;
    int P_OffsetVal2;
    int P_FullscaleVal2;
    int P_FullscaleLoad2;
    int P_PressureValue2; 
    int P_DecimalPoint2;
    int P_SerialNumber2;

    int P_NormalData3;
    int P_OffsetVal3;
    int P_FullscaleVal3;
    int P_FullscaleLoad3;
    int P_DecimalPoint3;
    int P_PressureValue3; 
    int P_SerialNumber3;

    int P_NormalData4;
    int P_OffsetVal4;
    int P_FullscaleVal4;
    int P_FullscaleLoad4;
    int P_DecimalPoint4;
    int P_PressureValue4;
    int P_SerialNumber4; 
private:
    /* Our data */
    epicsEventId eventId_;
    epicsFloat64 *pData_;
    epicsFloat64 *pTimeBase_;
};
