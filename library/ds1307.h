#ifndef DS1307_HPP
#define DS1307_HPP
#include "hwlib.hpp"

#define address 0x68 //DS1307 chip-address
#define seconds_Registry 0x00
#define minutes_Registry 0x01
#define hours_Registry   0x02
#define day_Registry     0x03 
#define date_Registry    0x04
#define month_Registry   0x05 
#define year_Registry    0x06

/// \brief
/// clockData struct
/// \details
/// This struct has no mandatory parameters. Though the user can provide the time and date, as specified in the datasheet.
struct clockData{unsigned int hours = 0; unsigned int minutes = 0; unsigned int seconds = 0; unsigned int day = 1; unsigned int date = 1; unsigned int month = 1; unsigned int year = 21;};

/// \brief
/// size_Of_Months Array
/// \details
/// This array specifies the size of each month
const unsigned int size_Of_Months[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

/// \brief
/// DS1307 library 
/// \details
/// This is a library for the DS1307 RTC(Realtime-Clock) that makes it easy to retrieve the current time, date and availability.
/// It also simplifies setting the current time and date which also simplifies pausing and resuming the clock.
class ds1307{
private:
    /// \brief
    /// Serial bus
    /// \details
    /// A hwlib serial communication bus to transact with the chip.
    hwlib::i2c_bus &bus;

    /// \brief
    /// Decimal to binary-coded decimal
    /// \details
    /// This function converts a given decimal value into the corresponding binary-coded decimal value.
    uint8_t dec_Bcd(uint8_t x){return((x/10*16) + (x%10));}
    
    /// \brief
    /// Binary-coded decimal to decimal
    /// \details
    /// This function converts a given binary-coded decimal value into the corresponding decimal value.
    uint8_t bcd_Dec(uint8_t x){return((x/16*10) + (x%16));}

    /// \brief
    /// Readtransaction with the chip
    /// \details
    /// This function has one mandatory parameter; the register that you're trying to read from. It will use this register to retrieve the time
    /// which it will then return after converting it from a bcd to dec value.
    unsigned int getClock(uint8_t registry){
        {
            auto write_Transaction = bus.write(address);
                write_Transaction.write(registry);        
        }
        return bcd_Dec(bus.read(address).read_byte());
    }

    /// \brief
    /// Writetransaction with the chip
    /// \details
    /// This function has two mandatory parameters; the register that you're trying to write to and the data you're trying to write to that register.
    void setClock(uint8_t registry, unsigned int value){
        {
            auto write_Transaction = bus.write(address);
                write_Transaction.write(registry);
                write_Transaction.write(dec_Bcd(value));
        }
    }

    /// \brief
    /// set Time
    /// \details
    /// This function has three mandatory parameters; hours, minutes and seconds. The specified data will be checked
    /// for validity, it will correct the data if needed and then write it to the clock by calling the setClock function.
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

    /// \brief
    /// set Date
    /// \details
    /// This function has four mandatory parameters; day of the week, date, month and year. The specified data will be checked
    /// for validity, it will correct the data if needed and then write it to the clock by calling the setClock function.
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

    /// \brief
    /// is Running 
    /// \details
    /// This function checks if the RTC clock is available by checking for invalid data.
    bool isRunning(){
        if(getClock(seconds_Registry) == 165 && getClock(minutes_Registry) == 165 && getClock(hours_Registry) == 165 && getClock(day_Registry) == 165 && getClock(date_Registry) == 165 && getClock(month_Registry) == 165 && getClock(year_Registry) == 165)
            return true;
        return false;
    }

public:

    /// \brief
    /// Constructor
    /// \details
    /// This constructor has one mandatory parameter; an i2c-bus.
    ds1307(hwlib::i2c_bus_bit_banged_scl_sda &i2c_bus):
        bus(i2c_bus)
    {}

    /// \brief
    /// reads the time and date from the clock
    /// \details
    /// This function has one mandatory parameter; a clockData variable. The function will check if the clock is available and if it is it will
    /// update the clockData variable with new clockData and then return true meaning it has succesfully updated and 
    /// if it is not available it will return false signalling that the clock is not currently available.
    bool read(clockData &time){
        if(isRunning())
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

    /// \brief
    /// writes the time and date to the clock
    /// \details
    /// This function has one mandatory parameter; a clockData variable. The function will use the specified structs data to write 
    /// the time and date to the clock.
    void write(const clockData time){
        setTime(time.hours, time.minutes, time.seconds);
        setDate(time.day, time.date, time.month, time.year);
    }    

    /// \brief
    /// resumes the clock
    /// \details
    /// This function checks if the clock is halted and if it is it will resume the clock.
    void resume(){
        if(getClock(seconds_Registry) >= 80)
            setClock(seconds_Registry, getClock(seconds_Registry) - 80); // Setting the 0x00 registry below 80 resumes the Clock
    }

    /// \brief
    /// halts the clock
    /// \details
    /// This function checks if the clock is ticking and if it is it will halt the clock.
    void stop(){
        if(getClock(seconds_Registry) < 60)
            setClock(seconds_Registry, getClock(seconds_Registry) + 80); // Setting the 0x00 registry above 80 halts the Clock
    }
};

#endif