/**
 * Kincony KC868-A8
 *
 * https://www.kincony.com/how-to-programming.html
 * See https://www.kincony.com/how-to-code-by-arduino-ide.html
 * Set Target board as NodeMCU-32S
 */

// INCLUDES
// See https://github.com/LennartHennigs/Button2
#include "src/Button2/Button2.h";
// See https://github.com/Arduino-IRremote/Arduino-IRremote
#include "src/Arduino-IRremote/IRremote.hpp";
// https://github.com/xreef/PCF8574_library
#include "PCF8574.h"

// CONSTANTS
// Relay Outputs
const byte relayPins[] = {2, 15, 5, 4};
const byte buzzerPin = 18;
const int TONE_PWM_CHANNEL = 0;
// Outputs in range 0-10V
const byte dac10VPins[] = {26, 25};
// Dry digital Inputs
const byte digitalInputPins[] = {36, 39, 27, 14};
// Analog inputs
const byte analogInputPins[] = {32, 33, 34, 35};
// 433 MHz RF Rx/Tx pins
const byte rf433Pins[] = {19, 21};
// IR Rx/Tx pins
const byte irPins[] = {23, 22};
// S2 Button
const byte s2buttonPin = 0;

// GLOBALS
// Input buttons 
Button2 inputButtons[4];
// S2 Button
Button2 s2button;

// CALLBACKS
void onPress(Button2& btn) {
  if(btn == s2button){
    Serial.print(F("S2 Button presed"));
  }
  for(int i=0; i<4; i++) {
    if(btn == inputButtons[i]){
      Serial.print(F("Button "));
      Serial.print(i);
      Serial.println(F(" pressed"));
      digitalWrite(relayPins[i], HIGH);
    }
  }

}
void onRelease(Button2& btn) {
  for(int i=0; i<4; i++) {
    if(btn == inputButtons[i]){
      digitalWrite(relayPins[i], LOW);
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println(F("Start Kincony KC868-A4 Test"));

  Serial.println(F("Configuring Inputs"));
  s2button.begin(s2buttonPin);
  s2button.setPressedHandler(onPress);
  for(int i=0; i<4; i++) {
    inputButtons[i].begin(digitalInputPins[i]);
    inputButtons[i].setPressedHandler(onPress);
    inputButtons[i].setReleasedHandler(onRelease);
  }

  Serial.println(F("Testing Relays"));
  for(int i=0; i<4; i++){
    pinMode(relayPins[i], OUTPUT);
    digitalWrite(relayPins[i], HIGH);
    delay(250);
  }
  for(int i=0; i<4; i++){
    digitalWrite(relayPins[i], LOW);
    delay(250);
  }


  Serial.println(F("Testing DAC outputs"));
  for(int i=0; i<2; i++){
    pinMode(dac10VPins[i], OUTPUT);
  }
  // Write to DAC pins
  int maxVal = 255; //255 = 10V
  for(int val=0; val<maxVal; val++){
    for(int i=0; i<2; i++){dacWrite(dac10VPins[i], val); }
    val++;
    delay(10);
  }
  for(int val=maxVal; val>0; val--){
    for(int i=0; i<2; i++){dacWrite(dac10VPins[i], val); }
    val--;
    delay(10);
  }


  Serial.println(F("Testing Buzzer"));
  // See https://espressif-docs.readthedocs-hosted.com/projects/arduino-esp32/en/latest/api/ledc.html
  ledcSetup(TONE_PWM_CHANNEL, 5000, 12);
  ledcAttachPin(buzzerPin, TONE_PWM_CHANNEL);
  ledcWriteNote(TONE_PWM_CHANNEL, NOTE_C, 4); 
  delay(50);
  ledcWriteNote(TONE_PWM_CHANNEL, NOTE_E, 4);
  delay(50);
  ledcWriteNote(TONE_PWM_CHANNEL, NOTE_G, 4); 
  delay(50);
  ledcWriteNote(TONE_PWM_CHANNEL, NOTE_C, 5); 
  delay(50);
  // To turn off tone, write out a PWM waveform with 0 frequency
  ledcWrite(TONE_PWM_CHANNEL, 0);
}

void loop() {
  for(int i=0; i<4; i++){
    inputButtons[i].loop();
  }
}