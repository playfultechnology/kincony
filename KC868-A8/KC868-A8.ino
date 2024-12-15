/**
 * Kincony KC868-A6
 *
 * https://www.kincony.com/how-to-programming.html
 * See https://www.kincony.com/how-to-code-by-arduino-ide.html
 * https://www.kincony.com/esp32-6-channel-relay-module-kc868-a6.html
 * Set Target board as NodeMCU-32S (or DOIT ESP32 DEVKIT V1)
 */

#define SDA 4
#define SCL 5

// INCLUDES
// See https://github.com/LennartHennigs/Button2
#include "src/Button2/Button2.h";
// See https://github.com/Arduino-IRremote/Arduino-IRremote
#include "src/Arduino-IRremote/IRremote.hpp";
// https://github.com/xreef/PCF8574_library
#include "PCF8574.h"
// OLED display. See https://github.com/lexus2k/lcdgfx
#include <lcdgfx.h>

// CONSTANTS
// Analog inputs
const byte analogInputPins[] = {36, 34, 35, 39};
// 433 MHz RF Rx/Tx pins
const byte rf433Pins[] = {2, 15};
// S2 Button
const byte s2buttonPin = 0;
// RS485 Tx 13, Rx 16

// GLOBALS
TwoWire Wire_1 = TwoWire(1);
// Input buttons 
Button2 inputButtons[4];
// S2 Button
Button2 s2button;
// For inputs
PCF8574 pcfIn1(0x21, 4, 5);
PCF8574 pcfIn2(0x22, 4, 5);
For relay outputs
PCF8574 pcfOut1(0x24, 4, 5);
PCF8574 pcfOut2(0x25, 4, 5);
DisplaySSD1306_128x32_I2C display(-1, {1, 0x3C, SCL, SDA, 400000});

// CALLBACKS
void onPress(Button2& btn) {
  if(btn == s2button){
    Serial.println(F("S2 Button presed"));
  }
}
void onRelease(Button2& btn) {
}

void setup() {
  Serial.begin(115200);
  Serial.println(F("Start Kincony KC868-A8 Test"));

  Wire_1.begin(4, 5);  // custom i2c port on ESP
  Wire_1.setClock(400000); // standard 400kHz speed

 /* Select the font to use with menu and all font functions */
    display.setFixedFont( ssd1306xled_font6x8 );
    display.begin();
    display.printFixed(0,  8, "Normal text", STYLE_NORMAL);

  Serial.println(F("Configuring Inputs"));
  s2button.begin(s2buttonPin);
  s2button.setPressedHandler(onPress);

  Serial.println(F("Testing Relays"));
  for(int i=0; i<8; i++){
    pcf.pinMode(i, OUTPUT);
  }  
  if (pcf.begin()){Serial.println(F("Ok"));}
  else {Serial.println(F("Error"));}
  for(int i=0; i<8; i++){
    pcf.digitalWrite(i, HIGH);
    delay(250);
  }
  for(int i=0; i<8; i++){
    pcf.digitalWrite(i, LOW);
    delay(250);
  }
}

void loop() {
  s2button.loop();
}