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
bool STATES_STATUS[STATES_COUNT];
int8_t LEDS_SEQUENCE[100];
int sequence_length = 0 ; /// length of LEDS_SEQUENCE vector
int sequence_pos = 0; /// vector index of the current led in sequence

int LED_PINS[] = {RED_LED,YELLOW_LED,GREEN_LED,BLUE_LED}; /// used leds
int n_pins = sizeof(LED_PINS) / sizeof(int); /// number of leds used
int current_led = 0; /// current led index in led_pins array
unsigned long last_time = millis();

    //SETTINGS
unsigned int start_led_time = 200; ///led light duration in start animation
unsigned int disp_led_time = 400;
unsigned int disp_led_delay = 200;
unsigned int start_disp_delay = 1000;

    //FUNCTIONS
// SET STATE
bool set_state(GameState state,bool status){
    bool updated = STATES_STATUS[state] != status;
    STATES_STATUS[state] = status;
    
    return updated;
}    

// START LED FUNCTION
unsigned long last_led_start = millis();
void start_led(int led_idx,unsigned int duration){
    noTone(BUZZER_PIN);
    turn_led(LED_PINS[led_idx],duration);
    int freq_idx = led_idx < n_frequencies ? led_idx : 0;
    tone(BUZZER_PIN,BUZZER_FREQUENCIES[freq_idx],duration);
    last_led_start = millis();
}

// PLAY LED SEQUENCE FUNCTION

struct play_response{
  bool ended;
  bool next_led;  
};
play_response play_led_sequence(bool start_sequence,int first_led,int next_led,bool run_condition,unsigned int led_duration = 100,unsigned int led_delay = 0){
    play_response response{false,false};
    
    if(start_sequence){
        current_led = first_led;
        start_led(current_led,led_duration);
    }
    else{
        bool current_led_status = get_led_status(LED_PINS[current_led]);
    
        if (run_condition && !current_led_status && millis() - last_led_start > led_delay){
            current_led = next_led;
            start_led(current_led,led_duration);
            response.next_led = true;
        }
        
        response.ended = !run_condition && !current_led_status;
    }
    
    return response;
}

// SET POWER FUNCTION
void set_power(bool set_on)
{
    if (set_on){
        Serial.println("OFF_END");
        CURRENT_STATE = START;
        
        digitalWrite(LED_BUILTIN,HIGH); // BUILDIN LED for debugging
    }
    else{
        set_state(CURRENT_STATE,false);
        Serial.println("OFF_BEGIN");
        CURRENT_STATE = OFF;
        
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
    
    if (power.updated && power.status){
        bool set_on = CURRENT_STATE == OFF;
        set_power(set_on);
    }
    
    //LEDS HANDLER
    check_leds(LED_PINS,n_pins,CURRENT_STATE == OFF);
    
    // STATE SWITCH
    switch(CURRENT_STATE)
    {
    case START:
        {
            bool updated = set_state(START,true);
            
            if (updated){ // START state BEGAN
                Serial.println("START_BEGIN");
                set_leds_mode(LED_PINS,n_pins,OUTPUT); // set leds to output mode
            }
            
            //START ANIMATION
            play_response response = play_led_sequence(updated,0,current_led + 1,current_led < n_pins-1,start_led_time);
            if(response.next_led){
                last_time = millis();
            }
            if(response.ended && (millis() - last_time) > (start_disp_delay + start_led_time)){ // START state ENDED
                Serial.println("START_END");
                set_state(START,false);
                CURRENT_STATE = DISP;
            }
            
            break;
        }
    case DISP:
        {
            bool updated = set_state(DISP,true);
            
            if (updated){ // DISP state BEGAN
                Serial.println("DISP_BEGIN");
                set_leds_mode(LED_PINS,n_pins,OUTPUT); // set leds to output mode
                sequence_pos = 0;
                increase_sequence();
            }
            
            //DISPLAY THE SEQUENCE
            play_response response = play_led_sequence(updated,LEDS_SEQUENCE[0],LEDS_SEQUENCE[sequence_pos + 1],sequence_pos < sequence_length - 1,disp_led_time,disp_led_delay);
            if(response.next_led){
                sequence_pos++;
            }
            if(response.ended){ // DISP state ENDED
                Serial.println("DISP_END");
                set_state(DISP,false);
                CURRENT_STATE = CHECK_INPUT;
            }
            
            break;
        }
    case CHECK_INPUT:
        {
            bool updated = set_state(CHECK_INPUT,true);
            
            if (updated){ // CHECK_INPUT state BEGAN
                Serial.println("CHECK_INPUT_BEGIN");
                set_leds_mode(LED_PINS,n_pins,INPUT); // set leds to input mode
                digitalWrite(POWER_PIN,HIGH);
                
            }
            
            break;
        }
    default:
        break;
    };
}