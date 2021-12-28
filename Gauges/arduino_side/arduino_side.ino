#include <Servo.h>
#include "common.h"

Servo speedmeter;
Servo tachometer;

#define GREEN_PIN 9
#define RED_PIN 11

void setup() {
  Serial.begin(FREQ);
  speedmeter.attach(5);
  speedmeter.write(180);
  tachometer.attach(3);
  tachometer.write(180);
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
}

void loop() {
   if (Serial.available() >= 4){
      if (Serial.read() == SYNC_CODE){
        speedmeter.write(Serial.read());
        tachometer.write(Serial.read());
        switch (Serial.read()){
        case LED_CODE::OFF:
        digitalWrite(GREEN_PIN, 0);
        digitalWrite(RED_PIN, 0);
        break;
        case LED_CODE::GREEN:
        digitalWrite(GREEN_PIN, 1);
        digitalWrite(RED_PIN, 0);
        break;
        case LED_CODE::ORANGE:
        analogWrite(RED_PIN, 32);
        digitalWrite(GREEN_PIN, 1);
        break;
        case LED_CODE::RED:
        digitalWrite(GREEN_PIN, 0);
        digitalWrite(RED_PIN, 1);
        break;
        default:
        break;
        }
      }

      /*if (Serial.read() == '1'){
          speedmeter.write(180);
          tachometer.write(180);
        }
        else {
          speedmeter.write(0);
          tachometer.write(0);
      }*/
   }
}
