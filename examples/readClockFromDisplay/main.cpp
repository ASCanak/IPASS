#include "../library/ds1307.h"

struct doubleDecimal{unsigned int lhs; unsigned int rhs;};

doubleDecimal convert(unsigned int x){return {(x/10%10), (x%10)};}

int main(){
    namespace target = hwlib::target;
    hwlib::wait_ms(10);

    const char *suffix[31] = { "st", "nd", "rd", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "st", "nd", "rd", "th", "th", "th", "th", "th", "th", "th", "st"};

    auto scl = target::pin_oc(target::pins::scl);
    auto sda = target::pin_oc(target::pins::sda);
    auto i2c_bus = hwlib::i2c_bus_bit_banged_scl_sda(scl, sda);

    auto clock = ds1307(i2c_bus);
    
    clockData time; // use the clockData struct format {hours, minutes, seconds, dayoftheWeek, date, month, year} or use the formate down below
    // time.hours = 0; time.minutes = 0; time.seconds = 0; 
    // time.day = 0; time.date = 0; time.month = 0; time.year = 0;
    // clock.write(time);
    // clock.stop();

    auto oled = hwlib::glcd_oled(i2c_bus, 0x3c); 
    auto small_Font = hwlib::font_default_8x8();
    auto big_Font = hwlib::font_default_16x16();
    auto upper_Window = hwlib::window_part_t(oled, hwlib::xy(0, 0), hwlib::xy(128, 32));
    auto lower_Window = hwlib::window_part_t(oled, hwlib::xy(0, 32), hwlib::xy(128, 32));
    auto upper_Screen = hwlib::terminal_from(upper_Window, big_Font);
    auto lower_Screen = hwlib::terminal_from(lower_Window, small_Font);    

    bool test = false;

    bool change_Hour = false;

    for(;;){
        if(clock.read(time)){
            upper_Screen << "\f";
            
            if(test || !change_Hour){
                upper_Screen << convert(time.hours).lhs << convert(time.hours).rhs << ':';
                if(!change_Hour)
                    test = !test;
            }
            else{
                upper_Screen << "  :";
                test = !test;
            }
            
            upper_Screen << convert(time.minutes).lhs << convert(time.minutes).rhs;
            
            upper_Screen << ':'; 
            
            upper_Screen << convert(time.seconds).lhs << convert(time.seconds).rhs;

            // lower_Screen << "\f" << convert(time.date).lhs << convert(time.date).rhs << "/" << convert(time.month).lhs << convert(time.month).rhs << "/" << time.year; // if you prefer a decimal print over a char print uncomment this print and comment the print below
            lower_Screen << "\f" << days[time.day - 1] << ' ' << months[time.month - 1] << " " << time.date << suffix[time.date - 1] << " " << time.year;

            oled.flush();
        }
        else{
            hwlib::cout << "RTC not available check wiring\n";
        }
        hwlib::wait_ms(250);
    }
}