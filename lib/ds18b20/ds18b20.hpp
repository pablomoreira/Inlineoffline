#ifndef ds18b20_h
#define ds18b20_h
#include <OneWire.h>
#include <Arduino.h>


class Ds18b20
{
    public:
        Ds18b20(uint8_t pin);
        void search();
        uint8_t getNum();
        char* addr2str();
        bool crc8();
        float getTemp();
        void update();
        uint32 _mark_time;

    private:
        int _pin;
        boolean _state;
        uint8 _num;
        uint8 _index;
        byte _data[12];
        byte _addr[8];
        OneWire _ds;
        char _addr2str[17];
        void __addr2str();
        //uint32 _mark_time;
        float temp;
};
#endif
