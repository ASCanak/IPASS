#include "../library/ds1307.h"

int main(){
    namespace target = hwlib::target;
    hwlib::wait_ms(10);

    const char *days[7] = { "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };

    auto scl = target::pin_oc(target::pins::scl);
    auto sda = target::pin_oc(target::pins::sda);
    auto i2c_bus = hwlib::i2c_bus_bit_banged_scl_sda(scl, sda);

    auto clock = ds1307(i2c_bus);
    
    clockData time; 
    time.hours = 23; time.minutes = 59; time.seconds = 57; 
    time.day = 1; time.date = 31; time.month = 12; time.year = 21;
    clock.write(time);                    //Writing (23:59:57 Mon 31-12-2021)

    for(unsigned int i = 0; i < 15; i++){ //Printing every second for 15 seconds to prove that the clock.read() works
        if(clock.read(time)){
            hwlib::cout << time.hours << ':' << time.minutes << ':' << time.seconds << ' ' << days[time.day - 1] << ' ' << time.date << "-" << time.month << "-" << time.year << '\n';
        }
        else{
            hwlib::cout << "RTC not available check wiring\n";
            while(!clock.read(time))
                hwlib::wait_ms(1000);
        }

        if(i == 4)
            clock.stop();    //halting the clock
        else if(i == 9)
            clock.resume();  //resuming the clock

        hwlib::wait_ms(900); //The wait isn't a full second because the c-outs take approximately 100 ms
    }

    hwlib::cout << "\n\n";

    time.hours = 25; time.minutes = 65; time.seconds = 88; 
    time.day = 99; time.date = 36; time.month = 35; time.year = 1253;
    clock.write(time);                    //Writing invalid values to the clock (25:65:88 99 36-35-3253) which the library will reset to (00:00:00 Mon 1-1-2000)

    for(unsigned int i = 0; i < 17; i++){ //Printing every second for 15 seconds to prove that the clock.read() works
        if(clock.read(time)){
            hwlib::cout << time.hours << ':' << time.minutes << ':' << time.seconds << ' ' << days[time.day - 1] << ' ' << time.date << "-" << time.month << "-" << time.year << '\n';
        }
        else{
            hwlib::cout << "RTC not available check wiring\n";
            while(!clock.read(time))
                hwlib::wait_ms(1000);
        }

        if(i == 4 || i == 13)
            clock.resume();  //Resuming the clock whilst it isn't halted
        else if(i == 7 || i == 10)
            clock.stop();    //Halting the clock whilst it's already halted

        hwlib::wait_ms(900); //The wait isn't a full second because the c-outs take approximately 100 ms
    }
}