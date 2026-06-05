#include <Arduino.h>
#include "handlers.hpp"
#include "configuration.hpp"

    // MAIN SETUP
void setup()
{
    //arduino led
    pinMode(LED_BUILTIN,OUTPUT);
    digitalWrite(LED_BUILTIN,LOW);
    
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
int sequence_length = 0 ; /// length of LEDS_SEQUENCE vector
int sequence_pos = 0; /// vector index of the current led in sequence

int LED_PINS[] = {RED_LED,YELLOW_LED,GREEN_LED,BLUE_LED}; /// used leds
int n_pins = sizeof(LED_PINS) / sizeof(int); /// number of leds used
int current_led = 0; /// current led index in led_pins array
unsigned long last_time = millis();

//SETTINGS
int start_led_time = 200; ///led light duration in start animation
int disp_led_time = 500;
unsigned int start_disp_delay = 1000;

//FUNCTIONS
void start_led(int led_idx,int duration){
    noTone(BUZZER_PIN);
    turn_led(LED_PINS[led_idx],duration);
    int freq_idx = led_idx < n_frequencies ? led_idx : 0;
    tone(BUZZER_PIN,BUZZER_FREQUENCIES[freq_idx],duration);
}

struct play_response{
  bool ended;
  bool next_led;  
};
play_response play_led_sequence(int first_led,int next_led,bool run_condition,int led_duration = 100){
    play_response response{false,false};
    
    if(current_led == -1){
        current_led = first_led;
        start_led(current_led,led_duration);
    }
    else{
        bool current_led_status = get_led_status(LED_PINS[current_led]);
    
        if (run_condition && !current_led_status){
            current_led = next_led;
            start_led(current_led,led_duration);
            response.next_led = true;
        }
        
        response.ended = !run_condition && !current_led_status;
        if(response.ended){
            current_led = -1;
        }
    }

    
    return response;
}

void set_power(bool set_on)
{
    if (set_on){
        current_led = -1;
        digitalWrite(LED_BUILTIN,HIGH); // BUILDIN LED for debugging
    }
    else{
        digitalWrite(LED_BUILTIN,LOW); // BUILDIN LED for debugging
        digitalWrite(POWER_PIN,LOW);
        noTone(BUZZER_PIN); // disable the buzzer
        sequence_length = 0; // clear the sequence
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
    
    //if(power.updated){
    //    Serial.println(String(power.updated) + String(" ") + String(power.status));
    //}
    
    if (power.updated && power.status){
        bool set_on = CURRENT_STATE == OFF;
        set_power(set_on);
        CURRENT_STATE = set_on ? START : OFF;
    }
    
    //LEDS HANDLER
    check_leds(LED_PINS,n_pins,CURRENT_STATE == OFF);
    
    // STATE SWITCH
    switch(CURRENT_STATE)
    {
    case OFF:
        break;
    case START:
        {
            //START ANIMATION
            play_response response = play_led_sequence(0,current_led + 1,current_led < n_pins-1,start_led_time);
            if(response.ended){
                Serial.println("START_END");
                CURRENT_STATE = DISP;
                sequence_pos = 0;
                increase_sequence();
                last_time = millis();
            }
            break;
        }
    case DISP:
        {
            if(millis() - last_time > start_disp_delay){
                if(current_led == -1){
                    Serial.println("DISP_BEGIN");
                }
                
                //display the sequence
                play_response response = play_led_sequence(LEDS_SEQUENCE[0],LEDS_SEQUENCE[sequence_pos + 1],sequence_pos < sequence_length - 1,,disp_led_time);
                if(response.next_led){
                    sequence_pos++;
                }
                if(response.ended){
                    Serial.println("DISP_END");
                    CURRENT_STATE = CHECK_INPUT;
                }
            }
        break;
        }
    case CHECK_INPUT:
        {
            break;
        }
    default:
        break;
    };
}