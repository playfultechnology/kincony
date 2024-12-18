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
#define RS485_TX 27

// This is pin DHT1 after removal of resistor R23, and bridging over R24
#define WS2812_DATA_PIN 32

#define SPI_CS 5

// INCLUDES
// I2C
#include <Wire.h>
// See https://github.com/LennartHennigs/Button2
#include "src/Button2/Button2.h";
// See https://github.com/Arduino-IRremote/Arduino-IRremote
#include "src/Arduino-IRremote/IRremote.hpp";
// https://github.com/xreef/PCF8574_library
#include "src/PCF8574/PCF8574.h"
// OLED display. See https://github.com/olikraus/u8g2
#include <U8g2lib.h>
// For WS2812 LED strip. See https://fastled.io/
#include <FastLED.h>
// For MFRC522 connected to SPI interface (intended for nRF24L01)
#include <MFRC522.h>

// CONSTANTS
// Analog inputs
//const byte analogInputPins[] = {36, 34, 35, 39};
// 433 MHz RF Rx/Tx pins
//const byte rf433Pins[] = {2, 15};
// S2 Button
constexpr byte s2buttonPin = 0;
// RS485 Tx 13, Rx 16
constexpr byte numLeds = 8;

// GLOBALS
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
// S2 Button
Button2 s2button;
// LED strip hue
uint8_t ledHue = 0;
// I2C GPIO expander used for digital inputs
PCF8574 pcfIn(0x22, I2C_SDA, I2C_SCL);
// I2c GPIO expander for relay outputs
PCF8574 pcfOut(0x24, I2C_SDA, I2C_SCL);
// RGB LED array
CRGB leds[numLeds];
// RFID
MFRC522 mfrc522(SPI_CS, -1);  // Create MFRC522 instance


// CALLBACKS
void onPress(Button2& btn) {
  if(btn == s2button){
    Serial.println(F("S2 Button pressed"));
    ledHue+=32;
  }
}
void onRelease(Button2& btn) {
}

void setup() {
  Serial.begin(115200);
  Serial.println(F("Start Kincony KC868-A6 Test"));

  Serial.print("Starting I2C interface...");
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(400000); // 400kHz speed
  Serial.println("done.");

  // OLED display
  Serial.print("Starting OLED display...");
  // Note that 7-bit I2C addresses (like 0x2F, 0x3C) need to be shifted left
  u8g2.setI2CAddress(0x03c << 1);
  u8g2.begin();
  u8g2.clearBuffer();					// clear the internal memory
  u8g2.setFont(u8g2_font_ncenB08_tr);	// choose a suitable font
  u8g2.drawStr(0,10,"KC868-A6");	// write something to the internal memory
  u8g2.sendBuffer();					// transfer internal memory to the display
  Serial.println("done.");

  // WS2812 strip
  Serial.print("Starting Programmable LED interface...");
  FastLED.addLeds<WS2812, WS2812_DATA_PIN, GRB>(leds, numLeds);
  fill_rainbow(leds, numLeds, 0, 10);
  FastLED.show();
  Serial.println("done.");

  // Digital Inputs
  Serial.print(F("Configuring Inputs..."));
  s2button.begin(s2buttonPin);
  s2button.setPressedHandler(onPress);
  if(pcfIn.begin()){
    for(int i=0; i<8; i++){
      pcfIn.pinMode(i, INPUT);
      delay(250);
    }
    Serial.println(F("done."));
  }
  else {
    Serial.println(F("error initialising PCF8574 input!"));
  }

  // Outputs
  Serial.print(F("initialising output relays"));
  for(int i=0; i<6; i++){
    pcfOut.pinMode(i, OUTPUT);
  }
  if (pcfOut.begin()){
    for(int i=0; i<6; i++){
        pcfOut.digitalWrite(i, LOW);
        delay(250);
      }
    for(int i=0; i<6; i++){
      pcfOut.digitalWrite(i, HIGH);
      delay(250);
    }
    Serial.println("done.");
  }
  else {
    Serial.println(F("Error initialising PCF8574 output!"));
  }

	SPI.begin(18, 19, 23, 5);			// Init SPI bus
	mfrc522.PCD_Init();		// Init MFRC522
	delay(4);				// Optional delay. Some board do need more time after init to be ready, see Readme
	mfrc522.PCD_DumpVersionToSerial();	// Show details of PCD - MFRC522 Card Reader details


}

void loop() {

	// Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
	if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
	  // Dump debug info about the card; PICC_HaltA() is automatically called
	  mfrc522.PICC_DumpToSerial(&(mfrc522.uid));
	}



  u8g2.clearBuffer();					// clear the internal memory
  u8g2.setFont(u8g2_font_ncenB08_tr);	// choose a suitable font
  u8g2.setCursor(0, 20);
  u8g2.print(millis());	// write something to the internal memory
  u8g2.sendBuffer();					// transfer internal memory to the display 

  // Update LEDs
  fadeToBlackBy(leds, numLeds, 30);
  int pos = beatsin16( 30, 0, numLeds-1 );
  leds[pos] += CHSV(ledHue, 255, 192);
  FastLED.show();

  // Inputs
  s2button.loop();
}