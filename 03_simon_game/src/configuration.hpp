#pragma once

//SETTINGS
const unsigned int start_led_time = 200; ///led light duration in start animation
const unsigned int disp_led_time = 400;
const unsigned int disp_led_delay = 200;
const unsigned int disp_enter_delay = 1000;

// PINS
#define POWER_PIN 2
#define BUZZER_PIN 3
#define POWER_BUTTON 4

// LED PI
#define RED_LED 5
#define YELLOW_LED 6
#define GREEN_LED 7
#define BLUE_LED 8

// STATES
enum GameState {
    OFF,
    START,
    DISP,
    CHECK_INPUT,
    SEQUENCE_END,
    STATES_COUNT
};

// BUZZER SOUNDS
const int BUZZER_FREQUENCIES[] = {
    300, // led_1
    350, // led_2
    400, // led_3
    450, // led_4
    200 // off
};
const int n_frequencies = sizeof(BUZZER_FREQUENCIES) / sizeof(int); // length of buzzer_frequencies vector