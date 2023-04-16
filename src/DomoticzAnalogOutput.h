#ifndef DomoticzAnalogOutput_h
#define DomoticzAnalogOutput_h

#include <driver/dac.h>
#include <Arduino.h>

const dac_channel_t DAC_CHANNEL = DAC_CHANNEL_1; // DAC_CHANNEL_1 for GPIO25, DAC_CHANNEL_2 for GPIO26
const int DAC_OUTPUT_PIN = 25; // Change this to 26 if you're using DAC_CHANNEL_2

void setDACOutputVoltage(float voltage)
{
    // Make sure the voltage is within the 0-10V range
    voltage = constrain(voltage, 0.0, 10.0);

    // Convert the voltage to a 12-bit value (0 - 4095)
    // The conversion assumes a 3.3V reference voltage for the DAC
    uint32_t dac_value = (voltage / 10.0) * (4095 / 3.3);

    // Set the DAC output value
    dac_output_voltage(DAC_CHANNEL, dac_value);
}

#endif //DomoticzAnalogOutput_h