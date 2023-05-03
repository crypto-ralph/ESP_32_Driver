#ifndef DomoticzOutput_h
#define DomoticzOutput_h

#include <Arduino.h>

class DomoticzOutput {
  private:
    int _pin;
    bool _state;
    int relayNumber;

  public:
    // Store the index as a string
    String enpointOn;
    String enpointOff;
    String relayName;

    DomoticzOutput(int pin, int _relayNumber, String _relayName) : relayNumber(_relayNumber), relayName(_relayName)
    {
      _pin = pin;
      _state = LOW;
      pinMode(_pin, OUTPUT);
    }
    
    void setHigh() 
    {
      digitalWrite(_pin, HIGH);
      _state = HIGH;
    }
    
    void setLow() 
    {
      digitalWrite(_pin, LOW);
      _state = LOW;
    }
    
    bool getState() 
    {
      return _state;
    }

    int getRelayNumber()
    {
      return relayNumber;
    }

    bool processAction(const String &action)
    {
      if (action == "on")
      {
        this->setHigh();
        return true;
      }
      else if (action == "off")
      {
        this->setLow();
        return true;
      }
      return false;
    }
};

#endif // DomoticzOutput_h