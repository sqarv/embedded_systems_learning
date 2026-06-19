#include <Arduino.h>
#include <LiquidCrystal.h>

#define ANALOG_PIN A0
#define Rf 10000.0f
#define R0 10000.0f
#define Beta 3380.0f
#define T0 298.15f

LiquidCrystal lcd(8,9,10,11,12,13);

void setup(){
    pinMode(ANALOG_PIN,INPUT);
    lcd.begin(16,2);
    
    Serial.begin(9600);
}

char buffer[17];
char fbuffer[6];

void loop(){
    float val = analogRead(ANALOG_PIN);
    float R = val * Rf / (1023 - val);
    float tempK = 1 / (1 / T0 + 1 / Beta * log(R/R0));
    double temp = tempK - 273.15;
    
    delay(1000);
    sprintf(buffer,"Temp: %s%-5s",dtostrf(temp,2,2,fbuffer),"");
    lcd.setCursor(0,0);
    lcd.print(buffer);
}