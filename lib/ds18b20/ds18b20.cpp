#include "ds18b20.hpp"
#include <OneWire.h>

Ds18b20::Ds18b20(uint8_t pin)
{
  _pin = pin;
  OneWire ds(pin);

  //pinMode(pin, OUTPUT);
}