#ifndef DomoticzDevicesLists_h
#define DomoticzDevicesLists_h

#include "DomoticzTempSensor.h"

// Temperature sensors config
DomoticzTempSensor temperatureSensors[] = {
    DomoticzTempSensor("1", 25),
    DomoticzTempSensor("2", 26),
    // Add more sensors here
};

#endif