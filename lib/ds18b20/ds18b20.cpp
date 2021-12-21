#include "ds18b20.hpp"
#include <Arduino.h>

Ds18b20::Ds18b20(uint8_t pin)
{
  _pin = pin;
  //OneWire _ds(_pin);
  _ds.begin(pin);
  _num = 0;
  _index = 0;
  //pinMode(pin, OUTPUT);
}
void Ds18b20::search(){
  uint8 i = 0, j = 0;
    if(_ds.search(_addr)){
      _num++;
      this->__addr2str();
    }
    else{
        _ds.reset_search();
        _num = 0;
        _index = 0;
        while (i < 8){
          j += sprintf(this->_addr2str + j,"%02hx",0);
          i++;
        }
    }
}

uint8_t Ds18b20::getNum(){
  return _num;
}

char* Ds18b20::addr2str(){
  return this->_addr2str;
}

void Ds18b20::__addr2str(){
  uint8 i = 0, j = 0;

  while (i < 8){
    j += sprintf(this->_addr2str + j,"%02hx",this->_addr[i]);
    i++;
  }
}
