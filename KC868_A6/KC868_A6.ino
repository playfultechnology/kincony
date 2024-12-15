/**
 * Kincony KC868-A6
 *
 * https://www.kincony.com/how-to-programming.html
 * See https://www.kincony.com/how-to-code-by-arduino-ide.html
 * https://www.kincony.com/esp32-6-channel-relay-module-kc868-a6.html
 * Set Target board as NodeMCU-32S (or DOIT ESP32 DEVKIT V1)
 */
/*
#define ANALOG_A1  36
#define ANALOG_A2  39
#define ANALOG_A3  34
#define ANALOG_A4  35

DS18B20/DHT11/DHT21/LED strip -1: 32
DS18B20/DHT11/DHT21/LED strip -2: 33

DAC1:26
DAC2:25

IIC SDA:4
IIC SCL:15

Input_IIC_address 0x22

Relay_IIC_address 0x24

RS485 RXD: 14
RS485 TXD: 27

SPI_Bus: (For LoRA/nRF24L01)
CS: 5
MOSI: 23
MISO: 19
SCK: 18

LoRA sx1278:
RST:21
DIO0:2

nRF24L01:
CE: 22

RS485:
TXD:27
RXD:14

RS232:
TXD:17
RXD:16

Extend serial port on PCB:
TXD(define by yourself):12
RXD (define by yourself) :13 


32/33 For WS2812?

*/

#define I2C_SDA 4
#define I2C_SCL 15

#define RS485_RX 14
#define RS485_TC 27


// INCLUDES
// See https://github.com/LennartHennigs/Button2
#include "src/Button2/Button2.h";
// See https://github.com/Arduino-IRremote/Arduino-IRremote
#include "src/Arduino-IRremote/IRremote.hpp";
// https://github.com/xreef/PCF8574_library
#include "src/PCF8574/PCF8574.h"
// OLED display. See https://github.com/lexus2k/lcdgfx
#include <lcdgfx.h>
#include <FastLED.h>


// CONSTANTS
// Analog inputs
const byte analogInputPins[] = {36, 34, 35, 39};
// 433 MHz RF Rx/Tx pins
//const byte rf433Pins[] = {2, 15};
// S2 Button
const byte s2buttonPin = 0;
// RS485 Tx 13, Rx 16

#define NUM_LEDS 8

//#define WS2812_DATA_PIN 32

// GLOBALS
TwoWire Wire_1 = TwoWire(1);
// Input buttons 
Button2 inputButtons[4];
// S2 Button
Button2 s2button;
// For inputs
PCF8574 pcfIn(0x22, I2C_SDA, I2C_SCL);
// For relay outputs
PCF8574 pcfOut(0x24, I2C_SDA, I2C_SCL);
DisplaySSD1306_128x32_I2C display(-1, {1, 0x3C, I2C_SCL, I2C_SDA, 400000});

CRGB leds[NUM_LEDS];


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
  Serial.println(F("Start Kincony KC868-A6 Test"));
/*
  Wire_1.begin(I2C_SDA, I2C_SCL);  // custom i2c port on ESP
  Wire_1.setClock(400000); // standard 400kHz speed


  pinMode(33, OUTPUT);
*/

  FastLED.addLeds<WS2812, 33, GRB>(leds, NUM_LEDS);
  fill_rainbow(leds, NUM_LEDS, 0, 7);
  FastLED.show();



 /* Select the font to use with menu and all font functions */
 /*
    display.setFixedFont( ssd1306xled_font6x8 );
    display.begin();
    display.printFixed(0,  8, "Normal text", STYLE_NORMAL);

  Serial.println(F("Configuring Inputs"));
  s2button.begin(s2buttonPin);
  s2button.setPressedHandler(onPress);
  if(pcfIn.begin()){Serial.println(F("Ok"));}
  else {Serial.println(F("Error"));}
  for(int i=0; i<8; i++){
    pcfIn.pinMode(i, INPUT);
    delay(250);
  }


  Serial.println(F("Testing Relays"));
  for(int i=0; i<6; i++){
    pcfOut.pinMode(i, OUTPUT);
  }  
  if (pcfOut.begin()){Serial.println(F("Ok"));}
  else {Serial.println(F("Error"));}
  for(int i=0; i<6; i++){
    pcfOut.digitalWrite(i, HIGH);
    delay(250);
  }
  for(int i=0; i<6; i++){
    pcfOut.digitalWrite(i, LOW);
    delay(250);
  }
  */
}

void loop() {

 // digitalWrite(33, HIGH);

  s2button.loop();
    fill_rainbow(leds, NUM_LEDS, 0, 7);
  FastLED.show();
  //delay(500);
  //digitalWrite(33, LOW);
  //delay(500);
}