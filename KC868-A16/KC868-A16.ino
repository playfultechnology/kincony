/**
 * Kincony KC868-A16
 *
 * https://www.kincony.com/how-to-programming.html
 * See https://www.kincony.com/how-to-code-by-arduino-ide.html
 * https://www.kincony.com/arduino-esp32-16-channel-relay-module-2.html
 * https://www.kincony.com/download/KC868-A16-schematic.pdf
 * https://www.kincony.com/esp32-board-16-channel-relay-hardware.html
 * Set Target board as NodeMCU-32S

 * Note that uploading to the board will fail if the RF transmitter/receiver are plugged in!
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
#define ETH_ADDR        0
#define ETH_POWER_PIN  -1
#define ETH_MDC_PIN    23
#define ETH_MDIO_PIN  18
#define ETH_TYPE      ETH_PHY_LAN8720
#define ETH_CLK_MODE  ETH_CLOCK_GPIO17_OUT

// GLOBALS
TwoWire Wire_1 = TwoWire(1);
// Input buttons 
Button2 inputButtons[4];
// S2 Button
Button2 s2button;

// Note that input and outputs are separated - they are not general purporse GPIO pins, because
// input pins have optocoupler isolator. output pins connect with MOSFET.
// For inputs
PCF8574 pcfIn1(0x22, 4, 5); // Channels 1-8
PCF8574 pcfIn2(0x21, 4, 5); // Channels 9-16
// For MOSFET outputs
PCF8574 pcfOut1(0x24, 4, 5); // Channels 1-8
PCF8574 pcfOut2(0x25, 4, 5); // Channels 9-16
DisplaySSD1306_128x32_I2C display(-1, {1, 0x3C, SCL, SDA, 400000});

// CALLBACKS
void onPress(Button2& btn) {
  if(btn == s2button){
    Serial.println(F("S2 Button presed"));
    display.clear();
    display.printFixed(0,  0, "S2 Button pressed", STYLE_NORMAL);
    delay(250);
    display.clear();  
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
    display.clear();
    display.printFixed(0,  8, "Normal text", STYLE_NORMAL);

  Serial.println(F("Configuring Inputs"));
  s2button.begin(s2buttonPin);
  s2button.setPressedHandler(onPress);
  for(int i=0; i<8; i++){
    pcfIn1.pinMode(i, INPUT);
    pcfIn2.pinMode(i, INPUT);
  }  
  if (pcfIn1.begin()){Serial.println(F("Ok"));}
  else {Serial.println(F("Error"));}
  if (pcfIn2.begin()){Serial.println(F("Ok"));}
  else {Serial.println(F("Error"));}  

  Serial.println(F("Testing MOSFET outputs"));

  for(int i=0; i<8; i++){
    pcfOut1.pinMode(i, OUTPUT);
    pcfOut2.pinMode(i, OUTPUT);
  }  
  if (pcfOut1.begin()){Serial.println(F("Ok"));}
  else {Serial.println(F("Error"));}
  if (pcfOut2.begin()){Serial.println(F("Ok"));}
  else {Serial.println(F("Error"));}  
   display.clear();
  display.printFixed(0,  8, "Testing MOSFET 1 Outputs", STYLE_NORMAL);
  for(int i=0; i<8; i++){
    pcfOut1.digitalWrite(i, LOW);
    delay(250);
    pcfOut1.digitalWrite(i, HIGH);
  }
  
  display.clear();
  display.printFixed(0,  8, "Testing MOSFET 2 Outputs", STYLE_NORMAL);
  for(int i=0; i<8; i++){
    pcfOut2.digitalWrite(i, LOW);
    delay(250);
    pcfOut2.digitalWrite(i, HIGH);
  }

   display.clear();
  
}

void loop() {
  s2button.loop();
  //display.clear();

  display.printFixed(0,  8, "In: ", STYLE_NORMAL);  
  for(int i=0; i<8; i++){
    uint8_t val1 = pcfIn1.digitalRead(i);
    display.printFixed(24+6*i,  8, val1 == LOW ? "1" : "0", STYLE_NORMAL);
    // Turn on/off the corresponding MOSFET otput  
    pcfOut1.digitalWrite(i, val1);     
    uint8_t val2 = pcfIn2.digitalRead(i);
    display.printFixed(24+48+6*i,  8, val2 == LOW ? "1" : "0", STYLE_NORMAL); 
    // Turn on/off the corresponding MOSFET otput    
    pcfOut2.digitalWrite(i, val2);    
  }
  display.printFixed(0,  16, "Out:", STYLE_NORMAL);  
  
  delay(250);
  
}