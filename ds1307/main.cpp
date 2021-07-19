#include "../library/ds1307.h"

struct doubleDecimal{unsigned int lhs; unsigned int rhs;};

doubleDecimal convert(unsigned int x){return {(x/10%10), (x%10)};}

int main(){
    namespace target = hwlib::target;
    hwlib::wait_ms(10);

    

    auto scl = target::pin_oc(target::pins::scl);
    auto sda = target::pin_oc(target::pins::sda);
    auto i2c_bus = hwlib::i2c_bus_bit_banged_scl_sda(scl, sda);

    auto clock = ds1307(i2c_bus);
    
    clockData time = clock.init();
    // clock.stop();

    auto oled = hwlib::glcd_oled(i2c_bus, 0x3c); 
    auto small_Font = hwlib::font_default_8x8();
    auto upper_Window = hwlib::window_part_t(oled, hwlib::xy(0, 0), hwlib::xy(128, 32));
    auto lower_Window = hwlib::window_part_t(oled, hwlib::xy(0, 32), hwlib::xy(128, 16));
    auto upper_Screen = hwlib::terminal_from(upper_Window, small_Font);
    auto lower_Screen = hwlib::terminal_from(lower_Window, small_Font);

    // clockData time2 = time; 
    

    for(;;){
        if(clock.read(time)){
            upper_Screen << "\f" << convert(time.hours).lhs << convert(time.hours).rhs << ":" << convert(time.minutes).lhs << convert(time.minutes).rhs << ":" << convert(time.seconds).lhs << convert(time.seconds).rhs << "\n\n" << days[time.day - 1];

            // hwlib::cout << convert(time.date).lhs << convert(time.date).rhs << "/" << convert(time.month).lhs << convert(time.month).rhs << "/" << time.year << '\n'; // if you prefer a decimal print over a char print uncomment this print and comment the print below

            lower_Screen << "\f" << time.date << " " << months[time.month - 1] << " " << time.year << '\n';

            oled.flush();
        }
        else{
            hwlib::cout << "RTC not available check wiring\n";
        }
        hwlib::wait_ms(700);
    }
}