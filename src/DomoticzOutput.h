#ifndef DomoticzOutput_h
#define DomoticzOutput_h

#include <Arduino.h>

class DomoticzOutput {
  private:
    int _pin;
    bool _state;

  public:
    // Store the index as a string
    String enpointOn;
    String enpointOff;

    DomoticzOutput(int pin, String _enpointOn, String _enpointOff) : enpointOn(_enpointOn), enpointOff(_enpointOff) 
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

    bool processAction(String action)
    {
        if (action == this->enpointOn)
        {
            this->setHigh();
            return true;
        } 
        else if (action == this->enpointOff)
        {
            this->setLow();
            return true;
        }
        return false;
    }
};

#endif // DomoticzOutput_h