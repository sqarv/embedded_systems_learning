#pragma once

//BUTTONS
struct button_response{
  bool status;
  bool updated;  
};

button_response button_handler(uint8_t button_pin, bool display_input);

//LEDS
struct led_info{
  bool status;
  bool input;
  unsigned int light_duration;
  unsigned long turning_time;
  
  led_info();
};

void set_leds_mode(int led_pins[],int n,int pin_mode);
void check_leds(int led_pins[],int n, bool force_off);
void turn_led(int led_pin,unsigned int time);
bool get_led_status(int led_pin);