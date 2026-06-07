#include <Arduino.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>
#include "handlers.hpp"
#include "configuration.hpp"

    // MAIN SETUP
                //RS , E , D4, D5, D6, D7
LiquidCrystal lcd(13 , 12 , 11 , 10 , 9 , A1);    
    
void setup()
{
    // init EEPROM highest score
    //EEPROM.write(0,0);
    
    //random seed
    srand(analogRead(A0));
    
    //lcd setup
    lcd.begin(16,2);
    pinMode(LCD_PIN,OUTPUT);
    digitalWrite(LCD_PIN,LOW);
     
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
//states
GameState CURRENT_STATE = OFF; /// current state of the game
bool STATES_STATUS[STATES_COUNT]; /// status of every state
//sequence
int8_t LEDS_SEQUENCE[100]; /// stores leds idx in the sequence
int sequence_length = 0 ; /// length of LEDS_SEQUENCE vector
int sequence_pos = 0; /// vector index of the current led in sequence
//led pins
int LED_PINS[] = {RED_LED,YELLOW_LED,GREEN_LED,BLUE_LED}; /// used led pins
const int n_pins = sizeof(LED_PINS) / sizeof(int); /// number of leds used
int current_led = 0; /// current led index in led_pins array
button_response LED_INPUT_RESPONSES[n_pins]; /// button response for led pins
//other
unsigned long last_time = millis();
uint8_t score = 0;
uint8_t highest = EEPROM.read(0);
char disp_buffer[17];

    //FUNCTIONS
// SET STATE
bool set_state(GameState state,bool status){
    bool updated = STATES_STATUS[state] != status;
    STATES_STATUS[state] = status;
    
    return updated;
}    

// GET BUZZER FREQUENCY FUNCTION
int get_buzzer_frequency(int idx){
    int freq_idx = idx < n_frequencies ? idx : 0;
    return BUZZER_FREQUENCIES[freq_idx];
}

// START LED FUNCTION
unsigned long last_led_start = millis();
void start_led(int led_idx,unsigned int duration){
    noTone(BUZZER_PIN);
    turn_led(LED_PINS[led_idx],duration);
    tone(BUZZER_PIN,get_buzzer_frequency(led_idx),duration);
    last_led_start = millis();
}

// PLAY LED SEQUENCE FUNCTION

struct play_response{
  bool ended;
  bool next_led;  
};
play_response play_led_sequence(bool start_sequence,int first_led,int next_led,bool run_condition,unsigned int led_duration = 100,unsigned int led_delay = 0){
    play_response response{false,false};
    bool current_led_status = get_led_status(LED_PINS[current_led]);
    
    if(start_sequence){
        current_led = first_led;
        start_led(current_led,led_duration);
    }
    else{
        if (run_condition && !current_led_status && millis() - last_led_start > led_delay + led_duration){
            current_led = next_led;
            start_led(current_led,led_duration);
            response.next_led = true;
        }
        
        response.ended = !run_condition && !current_led_status;
    }
    
    return response;
}

// SEQUENCE FUNCTIONS
void clear_sequence(){
    sequence_length = 0;
}

void increase_sequence(){
    LEDS_SEQUENCE[sequence_length++] = rand() % n_pins;
}

// SET POWER FUNCTION
void set_power(bool set_on)
{
    if (set_on){
        Serial.println("OFF_END");
        CURRENT_STATE = START;
        
        score = 0;
        digitalWrite(LCD_PIN,HIGH);
        digitalWrite(LED_BUILTIN,HIGH); // BUILDIN LED for debugging
    }
    else{
        set_state(CURRENT_STATE,false);
        Serial.println("OFF_BEGIN");
        CURRENT_STATE = OFF;
        
        digitalWrite(LCD_PIN,LOW);
        digitalWrite(LED_BUILTIN,LOW); // BUILDIN LED for debugging
        digitalWrite(POWER_PIN,LOW);
        tone(BUZZER_PIN,get_buzzer_frequency(n_frequencies - 1),500); // off sound
        clear_sequence(); // clear the sequence
    }
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
                
                //lcd
                
                lcd.setCursor(0,0);
                sprintf(disp_buffer,"Loading... %-6s","");
                lcd.print(disp_buffer);
            }
            
            //START ANIMATION
            play_response response = play_led_sequence(updated,0,current_led + 1,current_led < n_pins-1,start_led_time);
            if(response.ended){ // START state ENDED
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
                current_led = -1;
                increase_sequence();
                last_time = millis();
                
                //lcd
                lcd.setCursor(0,0);
                sprintf(disp_buffer,"Highest: %-7d",highest);
                lcd.print(disp_buffer);
                sprintf(disp_buffer,"Score: %-9d",score);
                lcd.setCursor(0,1);
                lcd.print(disp_buffer);
            }
            
            //DISPLAY THE SEQUENCE
            if(millis() - disp_enter_delay > last_time){
                play_response response = play_led_sequence(current_led == -1,LEDS_SEQUENCE[0],LEDS_SEQUENCE[sequence_pos + 1],sequence_pos < sequence_length - 1,disp_led_time,disp_led_delay);
                if(response.next_led){
                    sequence_pos++;
                }
                if(response.ended){ // DISP state ENDED
                    Serial.println("DISP_END");
                    set_state(DISP,false);
                    CURRENT_STATE = CHECK_INPUT;
                }
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
                sequence_pos = 0;
            }
            
            bool state_ended = false;
            if(sequence_pos < sequence_length){
                bool wrong_button_pressed = false;
                bool button_press_begin = false;
                bool button_press_end= false;
                current_led = LEDS_SEQUENCE[sequence_pos];
                
                // CHECK EVERY LED BUTTON
                for(int i = 0;i<n_pins; i++){
                    LED_INPUT_RESPONSES[i] = button_handler(LED_PINS[i],true,true);
                    button_response& input_response = LED_INPUT_RESPONSES[i];
                    
                    if(input_response.updated){ // BUTTON STATUS CHANGED
                        if (i == current_led){ // CORRECT BUTTON WAS PRESSED
                            if(input_response.status){ // PRESS BEGAN
                                button_press_begin = true;
                            }
                            else{ // PRESS ENDED
                                button_press_end = true;
                            }
                        }
                        else{ // WRONG BUTTON WAS PRESSED
                            wrong_button_pressed = true;
                            break; // SKIP OTHER BUTTONS IF LEFT
                        }
                    }
                }
                
                //BUTTON PRESS LOGIC
                if(wrong_button_pressed){ // WRONG BUTTON WAS PRESSED
                    state_ended = true;
                }
                else{ // CORRECT BUTTON OR NO BUTTON WAS PRESSED
                    if(button_press_begin){ // CORRECT BUTTON PRESS BEGAN
                        tone(BUZZER_PIN,get_buzzer_frequency(current_led),disp_led_time);
                    }
                    if(button_press_end){ // CORRECT BUTTON PRESS ENDED
                        sequence_pos++;
                    }
                }
            }
            else{
                state_ended = true;
            }
            
            if(state_ended){ // CHECK_INPUT state ENDED
                Serial.println("CHECK_INPUT_END");
                set_state(CHECK_INPUT,false);
                digitalWrite(POWER_PIN,false);
                set_leds_mode(LED_PINS,n_pins,OUTPUT);
                force_button_status(LED_PINS,n_pins,0); // force every button to off
                
                CURRENT_STATE = SEQUENCE_END;
            }
            
            break;
        }
    case SEQUENCE_END:
        {
            bool updated = set_state(SEQUENCE_END,true);
            bool state_ended = false;
            
            if (updated){ // SEQUENCE_END state BEGAN
                Serial.println("SEQUENCE_END_BEGIN");
                
                if(sequence_pos == sequence_length){ // SEQUENCE COMPLETED
                    state_ended = true;
                    current_led = 0;
                    score++;
                    if(score > highest){
                        tone(BUZZER_PIN,get_buzzer_frequency(n_frequencies),300);
                        EEPROM.write(0,score);
                        highest = score;
                    }
                }
                else{ // GAME LOST
                    score = 0;
                    current_led = n_pins;
                    last_time = millis();
                    clear_sequence();
                }
            }
            
            if(current_led > 2){ // PLAY LOSE SOUND
                if(millis() - last_time > lose_sound_time){
                    current_led--;
                    tone(BUZZER_PIN,get_buzzer_frequency(current_led),lose_sound_time);
                    last_time = millis();
                }
            }
            else{
                state_ended = true;
            }
            
            if(state_ended){ // SEQUENCE_END state ENDED 
                Serial.println("SEQUENCE_END");
                set_state(SEQUENCE_END,false);
                CURRENT_STATE = DISP;
            }
            
            break;
        }
    default:
        break;
    };
}