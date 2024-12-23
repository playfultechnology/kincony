/**
 * Kincony KC868-A6
 *
 * https://www.kincony.com/how-to-programming.html
 * See https://www.kincony.com/how-to-code-by-arduino-ide.html
 * https://www.kincony.com/esp32-6-channel-relay-module-kc868-a6.html
 * Set Target board as NodeMCU-32S (or DOIT ESP32 DEVKIT V1)
 */
/*
DAC1:26
DAC2:25
Input_IIC_address 0x22
Relay_IIC_address 0x24
LoRA sx1278:
RST:21
DIO0:2
nRF24L01:
CE: 22
*/
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
#include <TaskScheduler.h>

// CONSTANTS
constexpr byte SPI_CLK = 18, SPI_MISO = 19, SPI_MOSI = 23, SPI_CS = 5;
constexpr byte RS485_TX = 27, RS485_RX = 14;
constexpr byte I2C_SDA = 4, I2C_SCL = 15;
// As exposed on 4-pin header near OLED
constexpr byte SERIAL_TX = 12, SERIAL_RX = 13;
constexpr byte RS232_TX = 17, RS232_RX = 16;
// This is pin DHT1 after removal of resistor R23, and bridging over R24
constexpr byte  WS2812_DATA_PIN = 32;
// Analog inputs
constexpr byte analogInputPins[] = {36, 39, 34, 35};
// S2 Button
constexpr byte s2buttonPin = 0;

constexpr byte numLeds = 8;
constexpr uint8_t numInputs = 6;
constexpr uint8_t numOutputs = 6;
constexpr byte PCF8574_IN_ADDRESS = 0x22;
constexpr byte PCF8574_OUT_ADDRESS = 0x24;

// GLOBALS
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
// S2 Button
Button2 s2button;
// LED strip hue
uint8_t ledHue = 0;
// I2C GPIO expander used for digital inputs
PCF8574 pcfIn(PCF8574_IN_ADDRESS, I2C_SDA, I2C_SCL);
// I2c GPIO expander for relay outputs
PCF8574 pcfOut(PCF8574_OUT_ADDRESS, I2C_SDA, I2C_SCL);
// RGB LED array
CRGB leds[numLeds];
// RFID
MFRC522 mfrc522(SPI_CS, -1);  // Create MFRC522 instance

HardwareSerial RS485Serial(1);
HardwareSerial RS232Serial(2);

void rs485PollCallback();
Task rs485PollTask(5000, TASK_FOREVER, &rs485PollCallback);
Scheduler ts;

void rs485PollCallback() {
    Serial.print("Polling at: ");
    Serial.println(millis());
    if (rs485PollTask.getRunCounter() & 1 ) {
      RS485Serial.write("X");
    }
    else {
      RS485Serial.write("Y");
    }
}


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
  delay(1000);
  Serial.println(F("Kincony KC868-A6 Setup"));

  Serial.print("Starting RS485 serial interface...");
  RS485Serial.begin(9600, SERIAL_8N1, RS485_RX, RS485_TX);
  Serial.print("Starting RS232 serial interface...");
  RS232Serial.begin(9600, SERIAL_8N1, RS232_RX, RS232_TX);
  
  Serial.print("Starting scheduler...");
  ts.init();
  ts.addTask(rs485PollTask);
  Serial.print("added pollTask...");
  rs485PollTask.enable();
  Serial.println("pollTask enabled");

  Serial.print("Starting I2C interface...");
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(400000); // 400kHz speed
  Serial.println(F("done."));

  // OLED display
  Serial.print("Starting OLED display...");
  // Note that 7-bit I2C addresses (like 0x2F, 0x3C) need to be shifted left
  u8g2.setI2CAddress(0x03c << 1);
  u8g2.begin();
  u8g2.clearBuffer();					// clear the internal memory
  u8g2.setFont(u8g2_font_helvR08_tr);	// choose a suitable font
  u8g2.drawStr(0, 10, "KC868-A6");	// write something to the internal memory

  char formattedDate[9];
  getFormattedDate(formattedDate);
  u8g2.drawStr(80,10,formattedDate);	// write something to the internal memory


  u8g2.sendBuffer();					// transfer internal memory to the display
  Serial.println(F("done."));

  // WS2812 strip
  Serial.print("Starting Programmable LED interface...");
  FastLED.addLeds<WS2812, WS2812_DATA_PIN, GRB>(leds, numLeds);
  fill_rainbow(leds, numLeds, 0, 10);
  FastLED.show();
  Serial.println(F("done."));

  // Digital Inputs
  Serial.print(F("Configuring Inputs..."));
  s2button.begin(s2buttonPin);
  s2button.setPressedHandler(onPress);
  // Note need to set pinmodes before calling begin()
  for(int i=0; i<numInputs; i++){
    pcfIn.pinMode(i, INPUT);
    delay(250);
  }
  if(pcfIn.begin()){
    Serial.println(F("done."));
  }
  else {
    Serial.println(F("Error initialising PCF8574 input!"));
  }

  // Outputs
  Serial.print(F("Initialising Outputs..."));
  for(int i=0; i<6; i++){
    pcfOut.pinMode(i, OUTPUT);
  }
  if (pcfOut.begin()){
    for(int i=0; i<numOutputs; i++){
        pcfOut.digitalWrite(i, LOW);
        delay(250);
      }
    for(int i=0; i<numOutputs; i++){
      pcfOut.digitalWrite(i, HIGH);
      delay(250);
    }
    Serial.println(F("done."));
  }
  else {
    Serial.println(F("Error initialising PCF8574 output!"));
  }

  // SPI Interface
	SPI.begin(18, 19, 23, 5); // Init SPI bus
	mfrc522.PCD_Init();		// Init MFRC522
	delay(10);				// Optional delay. Some board do need more time after init to be ready, see Readme
	mfrc522.PCD_DumpVersionToSerial();	// Show details of PCD - MFRC522 Card Reader details
}

void loop() {
  // Service task scheduler
  ts.execute();

	// Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
	if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
	  // Dump debug info about the card; PICC_HaltA() is automatically called
	  mfrc522.PICC_DumpToSerial(&(mfrc522.uid));
	}

  if(RS485Serial.available()){
    char c = RS485Serial.read();
    Serial.write(c);
    u8g2.setDrawColor(0);
    u8g2.drawBox(0, 32, 10, 10);
    u8g2.setCursor(0, 40);
    u8g2.setDrawColor(1);
    u8g2.print(c);
  }

  //u8g2.clearBuffer();					// clear the internal memory
  u8g2.setFont(u8g2_font_helvR08_tr);	// choose a suitable font
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
  for(int i=0; i<numInputs; i++){
    pcfIn.digitalRead(i);
  }
}

// Helper function to format __DATE__
void getFormattedDate(char *buff) {
  char const *date = __DATE__;
  int month, day, year;
  static const char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
  sscanf(date, "%s %d %d", buff, &day, &year);
  month = (strstr(month_names, buff)-month_names)/3+1;
  sprintf(buff, "%d%02d%02d", year, month, day);
}

