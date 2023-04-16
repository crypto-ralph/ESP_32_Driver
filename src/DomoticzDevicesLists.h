#ifndef DomoticzDevicesLists_h
#define DomoticzDevicesLists_h

#include "DomoticzTempSensor.h"
#include "DomoticzOutput.h"

// GPIO25 DAC 0-10V output AHU0
// GPIO26 PWM

// Temperature sensors config
DomoticzTempSensor temperatureSensors[] = {
    DomoticzTempSensor("1", 25, "Tout"),
    DomoticzTempSensor("2", 26, ""),
    DomoticzTempSensor("3", 27, ""),
    // Add more sensors here
};

// Outputs config
DomoticzOutput outputRelays[] = {
    DomoticzOutput(22, "relay1on", "relay1off"),
};

String getSensorListHtml()
{
    String html = "<ul>";
    for (auto &sensor : temperatureSensors)
    {
        html += "<li>Temperature Sensor ";
        html += sensor.idx;
        html += " (GPIO: ";
        html += sensor.busPin;
        html += ")</li>";
    }
    html += "</ul>";
    return html;
}

#endif // DomoticzDevicesLists_h