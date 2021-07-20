#include "../library/ds1307.h"

#define idle_State 0
#define hours_State 1
#define minutes_State 2
#define date_State 3
#define month_State 4
#define year_State 5
#define pauze_State 10

struct doubleDecimal{unsigned int lhs; unsigned int rhs;};

doubleDecimal convert(unsigned int x){return {(x/10%10), (x%10)};}

int main(){
    namespace target = hwlib::target;
    hwlib::wait_ms(10);

    const char *suffix[31] = { "st", "nd", "rd", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "st", "nd", "rd", "th", "th", "th", "th", "th", "th", "th", "st"};

    auto scl = target::pin_oc(target::pins::scl);
    auto sda = target::pin_oc(target::pins::sda);
    auto i2c_bus = hwlib::i2c_bus_bit_banged_scl_sda(scl, sda);

    auto black_Switch = hwlib::target::pin_in( hwlib::target::pins::d8  );
    auto white_Switch = hwlib::target::pin_in( hwlib::target::pins::d9  );
    auto green_Switch = hwlib::target::pin_in( hwlib::target::pins::d10 );    
    auto red_Switch   = hwlib::target::pin_in( hwlib::target::pins::d11 );

    auto oled = hwlib::glcd_oled(i2c_bus, 0x3c); 
    auto small_Font = hwlib::font_default_8x8();
    auto big_Font = hwlib::font_default_16x16();
    auto upper_Window = hwlib::window_part_t(oled, hwlib::xy(0, 0), hwlib::xy(128, 24));
    auto lower_Window = hwlib::window_part_t(oled, hwlib::xy(0, 24), hwlib::xy(128, 40));
    auto upper_Screen = hwlib::terminal_from(upper_Window, big_Font);
    auto lower_Screen = hwlib::terminal_from(lower_Window, small_Font);   

    auto clock = ds1307(i2c_bus); 
    clockData time; // use the clockData struct format {hours, minutes, seconds, dayoftheWeek, date, month, year} or use the format down below
    // time.hours = 0; time.minutes = 0; time.seconds = 0; 
    // time.day = 0; time.date = 0; time.month = 0; time.year = 0;
    clock.write(time);

    unsigned int state = idle_State;
    bool blinking = false;
    unsigned int pauzed_seconds = 0;

    for(;;){
        if(!black_Switch.read() && (state != idle_State || state != pauze_State)){
            if(state == hours_State)
                time.hours   += 1;
            else if(state == minutes_State)
                time.minutes += 1;
            else if(state == month_State)
                time.month   += 1;
            else if(state == year_State)
                time.year    += 1;
            else if(state == date_State){
                time.day     += 1;
                time.date    += 1;
            }

            time.year -= 2000;
            clock.write(time);
        }
        if(!white_Switch.read() && state != idle_State){
            if(state == hours_State){
                if(time.hours != 0)
                    time.hours   -= 1;
                else 
                    time.hours    = 23;
            }
            else if(state == minutes_State){
                if(time.minutes != 0)
                    time.minutes -= 1;
                else
                    time.minutes  = 59;
            }
            else if(state == month_State){
                if(time.month != 1)
                    time.month   -= 1;
                else 
                    time.month    = 12;
            }
            else if(state == year_State){
                if(time.year != 0)
                    time.year    -= 1;
                else
                    time.year     = 99;
            }
            else if(state == date_State){
                if(time.day != 1)
                    time.day     -= 1;
                else
                    time.day      = 7;
                if(time.date != 1)
                    time.date    -= 1;
                else
                    time.date     = size_Of_Months[time.month - 1];
            }

            time.year -= 2000;
            clock.write(time);
        }
        if(!green_Switch.read() && state != pauze_State){
            if(state != year_State)
                state += 1;
            else
                state = idle_State;
        }
        if(!red_Switch.read()){
            if(time.seconds == 80){
                clock.resume(pauzed_seconds);
                state = idle_State;
            }
            else{
                pauzed_seconds = time.seconds;
                clock.stop();
                state = pauze_State;
            }
            hwlib::wait_ms(250);
        }

        if(clock.read(time) && state != pauze_State){
            upper_Screen << "\f";
            
            if(blinking || state != hours_State){
                upper_Screen << convert(time.hours).lhs << convert(time.hours).rhs << ':';
                if(state == hours_State)
                    blinking = !blinking;
            }
            else{
                upper_Screen << "  :";
                blinking = !blinking;
            }
            
            if(blinking || state != minutes_State){
                upper_Screen << convert(time.minutes).lhs << convert(time.minutes).rhs << ':';
                if(state == minutes_State)
                    blinking = !blinking;
            }
            else{
                upper_Screen << "  :"; 
                blinking = !blinking;
            }
            
            upper_Screen << convert(time.seconds).lhs << convert(time.seconds).rhs;

            lower_Screen << "\f" << days[time.day - 1] << " ";
            
            if(blinking || state != month_State){
                lower_Screen << months[time.month - 1] << " "; 
                if(state == month_State)
                    blinking = !blinking;
            }
            else{
                lower_Screen << "    ";
                blinking = !blinking;
            }
            
            if(blinking || state != date_State){
                lower_Screen << time.date << suffix[time.date - 1] << " ";
                if(state == date_State)
                    blinking = !blinking;            }
            else{
                lower_Screen << "    ";
                blinking = !blinking;
            }
            
            if(blinking || state != year_State){
                lower_Screen << time.year;
                if(state == year_State)
                    blinking = !blinking;
            }
            else
                blinking = !blinking;
        }
        else if(state != pauze_State)
            lower_Screen << "\f" << "RTC not working \n\ncheck wiring";
        
        oled.flush();
    }
}