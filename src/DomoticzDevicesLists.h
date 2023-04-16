#ifndef DomoticzDevicesLists_h
#define DomoticzDevicesLists_h

#include "DomoticzTempSensor.h"
#include "DomoticzOutput.h"

// Temperature sensors config
DomoticzTempSensor temperatureSensors[] = {
    DomoticzTempSensor("1", 25),
    DomoticzTempSensor("2", 26),
    // Add more sensors here
};

// Outputs config
DomoticzOutput outputRelays[] = {
    DomoticzOutput(22, "relay1on", "relay1off"),
};

#endif //DomoticzDevicesLists_h