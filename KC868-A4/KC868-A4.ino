/**
 * Kincony KC868-A4
 *
 * https://www.kincony.com/arduino-esp32-4-channel-relay-module.html
 * See https://www.kincony.com/how-to-code-by-arduino-ide.html
 * Set Target board as NodeMCU-32S
 */

// INCLUDES
// See https://github.com/LennartHennigs/Button2
#include "src/Button2/Button2.h";
// See https://github.com/Arduino-IRremote/Arduino-IRremote
#include "src/Arduino-IRremote/IRremote.hpp";

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
  // Write 5V to DAC pins
  int value = 127;  // 255 = 10V
  for(int i=0; i<2; i++){
    pinMode(dac10VPins[i], OUTPUT);
    dacWrite(dac10VPins[i], value);
    delay(1000);
    dacWrite(dac10VPins[i], 0);
  }


  Serial.println(F("Testing Buzzer"));  
  pinMode(buzzerPin, OUTPUT);
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