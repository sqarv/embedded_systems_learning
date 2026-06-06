#include <Arduino.h>
#include "handlers.hpp"
#include "configuration.hpp"

//BUTTONS HANDLER
const uint8_t n = 20;
bool button_status[n];
unsigned long button_debounce[n];
unsigned int debounce_time_ms = 100;

button_response button_handler(uint8_t button_pin, bool display_input)
{
    bool pressed = !digitalRead(button_pin);
    bool updated = false;
    
    if (pressed != button_status[button_pin] && millis() - button_debounce[button_pin] > debounce_time_ms)
    {
        button_debounce[button_pin] = millis();
        updated = true;
        button_status[button_pin] = pressed;
    }
    else{
        display_input = false;
    }

    if (display_input)
    {
        String msg = String(String("Button ") + button_pin + String(" status: ") + button_status[button_pin]);
        Serial.println(msg);
    }
    
    return button_response{button_status[button_pin],updated};
}

//LEDS HANDLER
led_info::led_info()
{
    status = false;
    input = false;
    turning_time = 0;
    light_duration = 0;
};


led_info leds[n];

void set_leds_mode(int led_pins[], int n, int pin_mode)
{
    for(int i = 0;i<n;i++){
        pinMode(led_pins[i],pin_mode);
    }
}

void check_leds(int led_pins[], int n, bool force_off)
{
    led_info* info;
    
    for(int i = 0; i < n;i++){
        info = leds + led_pins[i];
        
        if(info->status && (force_off || info->turning_time + info->light_duration < millis())){
            digitalWrite(led_pins[i],LOW);
            info->status = false;
        };
    }
}

void turn_led(int led_pin,unsigned int time)
{
    if (led_pin < n){
        led_info& info = leds[led_pin];
        
        if (!info.input){
            info.status = true;
            info.turning_time = millis();
            info.light_duration = time;
            pinMode(led_pin,OUTPUT);
            digitalWrite(led_pin,HIGH);
        }
    }
}

bool get_led_status(int led_pin)
{
    
    return (led_pin < n && leds[led_pin].status);
}
