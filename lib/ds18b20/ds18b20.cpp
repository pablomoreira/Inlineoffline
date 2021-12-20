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
    if(_ds.search(_addr)){
      _num++;
    }
    else{
        _ds.reset_search();
        _num = 0;
        _index = 0;
    }
}

uint8_t Ds18b20::getNum(){
  return _num;
}

char* Ds18b20::addr2str(){

  int j = 0;
  int i = 0;

  while (i < 8){
    j += sprintf(this->_addr2str + j,"%02hx",this->_addr[i]);
    i++;
  }
  return this->_addr2str;
}
