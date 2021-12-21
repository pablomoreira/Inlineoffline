#include "Arduino.h"
#include "digital.h"

Signal::Signal(uint8 pin,bool state)
{
  pinMode(pin, OUTPUT);
  _pin = pin;
  _state = state;
  digitalWrite(_pin,state);
}
void Signal::hight()
{

}
void Signal::low()
{
    _state = LOW;
    digitalWrite(_pin,LOW);
}
void Signal::change(){
    _state = ! _state;
    digitalWrite(_pin,_state);
}
void Signal::setblink(uint8 num){
    if(_num != num){
    _num = num * 2;
    _index = 0;
    low();
    }
}
void Signal::blink(){
    if(_index > 49) _index = 0;
   if(_index < _num){
       change();
   }
   _index++;
}
