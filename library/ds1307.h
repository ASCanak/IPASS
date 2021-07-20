#ifndef DS1307_HPP
#define DS1307_HPP
#include "hwlib.hpp"

#define address 0x68
#define seconds_Registry 0x00
#define minutes_Registry 0x01
#define hours_Registry 0x02
#define day_Registry 0x03 
#define date_Registry 0x04
#define month_Registry 0x05 
#define year_Registry 0x06

struct clockData{unsigned int hours = 0; unsigned int minutes = 0; unsigned int seconds = 0; unsigned int day = 1; unsigned int date = 1; unsigned int month = 1; unsigned int year = 21;};

const char *days[7] = { "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };
const char *months[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
const unsigned int size_Of_Months[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

class ds1307{
private:
    hwlib::i2c_bus &bus;

    uint8_t dec_Bcd(uint8_t x){return((x/10*16) + (x%10));}
    uint8_t bcd_Dec(uint8_t x){return((x/16*10) + (x%16));}

    unsigned int getClock(uint8_t registry){
        {
            auto write_Transaction = bus.write(address);
                write_Transaction.write(registry);        
        }
        return bcd_Dec(bus.read(address).read_byte());
    }

    void setClock(uint8_t registry, unsigned int value){
        {
            auto write_Transaction = bus.write(address);
                write_Transaction.write(registry);
                write_Transaction.write(dec_Bcd(value));
        }
    }

    void setTime(unsigned int hours, unsigned int minutes, unsigned int seconds){
        if(hours > 23)
            hours = 0;
        setClock(hours_Registry, hours);     //setHours

        if(minutes > 59)
            minutes = 0;
        setClock(minutes_Registry, minutes); //setMinutes

        if(seconds > 59)
            seconds = 0;
        setClock(seconds_Registry, seconds); //setSeconds
    }

    void setDate(unsigned int day, unsigned int date, unsigned int month, unsigned int year){
        if(year > 99)
            year = 0;
        setClock(year_Registry, year);   //setYear
        
        if(month > 12)
            month = 1;
        setClock(month_Registry, month); //setMonth

        if(date > size_Of_Months[month - 1])
            date = 1;
        setClock(date_Registry, date);   //setDate

        if(day < 1 || day > 7)
            day = 1;
        setClock(day_Registry, day);     //setDayoftheWeek
    }

public:
    ds1307(hwlib::i2c_bus_bit_banged_scl_sda &i2c_bus):
        bus(i2c_bus)
    {}

    bool read(clockData &time){
        if(getClock(seconds_Registry) == 165 )
            return false;
        else{
            time.hours   = getClock(hours_Registry);
            time.minutes = getClock(minutes_Registry);
            time.seconds = getClock(seconds_Registry);
            time.day     = getClock(day_Registry);
            time.date    = getClock(date_Registry);
            time.month   = getClock(month_Registry);
            time.year    = getClock(year_Registry) + 2000;
        }
        return true;
    }

    void write(const clockData time){
        setTime(time.hours, time.minutes, time.seconds);
        setDate(time.day, time.date, time.month, time.year);
    }    

    void resume(unsigned int seconds){
        setClock(seconds_Registry, seconds); // Writing 0 to 0x00 to resume the Clock
    }

    void stop(){
        setClock(seconds_Registry, 80); // Writing 80 to 0x00 stops the Clock
    }
};

#endif