#ifndef signal_h
#define signal_h

#include "Arduino.h"

class Signal
{
  public:
    Signal(uint8 pin,bool state);
    void hight();
    void low();
    void change();
    void setblink(uint8 num);
    void blink();
  private:
    int _pin;
    boolean _state;
    uint8 _num;
    uint8 _index;
};

#endif