#include <Arduino.h>
#include "handlers.hpp"
#include "configuration.hpp"

    // MAIN SETUP
void setup()
{
    // power setup
    pinMode(POWER_PIN, OUTPUT);
    digitalWrite(POWER_PIN, LOW);

    // buttons setup
    pinMode(POWER_BUTTON, INPUT_PULLUP);

    Serial.begin(9600);
}

    // MAIN LOOP
//GLOBAL
GameState CURRENT_STATE = OFF;
int8_t LEDS_SEQUENCE[100];
int sequence_length = 0;
int sequence_pos = 0;

int led_pins[] = {RED_LED,YELLOW_LED,GREEN_LED,BLUE_LED}; /// used leds
int n_pins = sizeof(led_pins) / sizeof(int); /// number of leds used
int current_led = 0; /// current led index in led_pins array
unsigned long last_time = millis();

//SETTINGS
int start_animation_led_time_ms = 100; ///led light duration in start animation
int start_disp_delay = 1000;

//FUNCTIONS
void start_led(int frequency){
    noTone(BUZZER_PIN);
    turn_led(led_pins[current_led],start_animation_led_time_ms);
    tone(BUZZER_PIN,frequency,start_animation_led_time_ms);
}

bool play_led_sequence(int next_led,bool run_condition){
    bool current_led_status = get_led_status(led_pins[current_led]);
    if (run_condition && !current_led_status){
        current_led = next_led;
        start_led(BUZZER_FREQUENCIES[current_led]);
    }
    return !run_condition && !current_led_status;
}

void set_power(bool set_on)
{
    if (set_on){
        current_led = 0;
        start_led(BUZZER_FREQUENCIES[current_led]);
    }
    else{
        digitalWrite(POWER_PIN,LOW);
        noTone(BUZZER_PIN);
    }
}

void increase_sequence(){
    LEDS_SEQUENCE[sequence_length++] = rand() % n_pins;
}

//LOOP
void loop()
{
    //POWER BUTTON
    button_response power = button_handler(POWER_BUTTON, false);
    if (power.updated && power.status){
        bool set_on = CURRENT_STATE == OFF;
        set_power(set_on);
        CURRENT_STATE = set_on ? START : OFF;
    }
    
    //LEDS HANDLER
    check_leds(led_pins,n_pins,CURRENT_STATE == OFF);
    
    Serial.println(CURRENT_STATE);
    
    // state switch
    switch (CURRENT_STATE)
    {
    case START:
        //START ANIMATION
        bool finished = play_led_sequence(current_led + 1,current_led < n_pins-1);
        if(finished){
            CURRENT_STATE = DISP;
            sequence_pos = 0;
            increase_sequence();
            last_time = millis();
        }
        
        break;
    case DISP:
        if(millis() - last_time > start_disp_delay){
            //display the sequence
            bool finished = play_led_sequence(LEDS_SEQUENCE[sequence_pos + 1],sequence_pos < sequence_length - 1);
        }
    
        break;
    default:
        break;
    };
}