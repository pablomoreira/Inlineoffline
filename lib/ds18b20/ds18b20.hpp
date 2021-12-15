#ifndef ds18b20_h
#define ds18b20_h

#include "Arduino.h"

class Ds18b20
{
    public:
        Ds18b20(uint8_t pin);
    
    private:
        int _pin;
        boolean _state;
        uint8 _num;
        uint8 _index;
};

#endif