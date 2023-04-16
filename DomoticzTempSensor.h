#ifndef DomoticzTempSensor_h
#define DomoticzTempSensor_h

#include <OneWire.h>
#include <DS18B20.h>


class DomoticzTempSensor {
  private:
    // Create instances of the OneWire and DallasTemperature libraries
    OneWire oneWireObj;
    DS18B20 sensorObj;

  public:
    // Store the index as a string
    String idx;

    // Constructor
    DomoticzTempSensor(String idx, int busPin) : oneWireObj(busPin), sensorObj(&oneWireObj) 
    {
      this->idx = idx;
    }

    // Wrapper for begin function to call in setup()
    void begin()
    {
      sensorObj.begin();
    }

    // Get the temperature from the sensor
    float getTemperature() 
    {
      float temp = 0.0;
      if (sensorObj.isConversionComplete())
      {
        temp = sensorObj.getTempC();
        sensorObj.requestTemperatures();
      }
      return temp;
    }
};

#endif // DomoticzTempSensor_h