#pragma once

//BUTTONS
struct button_response{
  bool status;
  bool updated;  
};

button_response button_handler(short int button_pin, bool display_input);

//LEDS
struct led_info{
  bool status;
  bool input;
  unsigned long light_duration;
  unsigned long turning_time;
  
  led_info();
};

void check_leds(int led_pins[],int n, bool force_off);
void turn_led(int led_pin,int time);
bool get_led_status(int led_pin);