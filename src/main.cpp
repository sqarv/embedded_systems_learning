#include <Arduino.h>
#include <SR04.h>

//pins
#define TRIGGER 12
#define ECHO 11
#define BUZZER 10

SR04 sensor(ECHO,TRIGGER);
void setup(){
  pinMode(BUZZER,OUTPUT);
}

int volume = 0,min_wait = 50,max_wait = 1000,min_dist = 3,max_dist = 500,min_vol = 5,max_vol = 15;
int distance = 0;
unsigned int wait_time = 0;
unsigned long current_time = 0,last_time = 0;
bool active = false;
void loop(){
  distance = sensor.Distance();
  
  wait_time = map(distance,min_dist,max_dist,min_wait,max_wait);
  wait_time = constrain(wait_time,min_wait,max_wait);
  volume = map(distance,min_dist,max_dist,min_vol,max_vol);
  volume = constrain(volume,min_vol,max_vol);
  
  current_time = millis();
  if (current_time - last_time > wait_time){
    last_time = current_time;
    
    if (active){
      //turns off
      analogWrite(BUZZER,0);
    }
    else if(distance <= max_dist){
      //turns on
      analogWrite(BUZZER,volume);
    }
    
    active = !active;
  }
}