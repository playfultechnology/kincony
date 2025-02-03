/**
 * Kincony KC868-A6
 *
 * https://www.kincony.com/how-to-programming.html
 * See https://www.kincony.com/how-to-code-by-arduino-ide.html
 * https://www.kincony.com/esp32-6-channel-relay-module-kc868-a6.html
 * Set Target board as NodeMCU-32S (or DOIT ESP32 DEVKIT V1)
 */

// INCLUDES
#include <WiFi.h>
// I2C
#include <Wire.h>
// See https://github.com/LennartHennigs/Button2
#include "src/Button2/Button2.h";
// See https://github.com/Arduino-IRremote/Arduino-IRremote
#include "src/Arduino-IRremote/IRremote.hpp";
// https://github.com/RobTillaart/PCF8574
#include <PCF8574.h>
// See https://github.com/SnijderC/dyplayer
#include "src/DYPlayer/DYPlayerArduino.h";
// OLED display. See https://github.com/olikraus/u8g2
#include <U8g2lib.h>
// For WS2812 LED strip. See https://fastled.io/
#include <FastLED.h>
// For MFRC522 connected to SPI interface (intended for nRF24L01)
#include <MFRC522.h>
#include <TaskScheduler.h>
// Using the ESP32 built-in bluetooth, so no need for serial Bluetooth connection to, e.g. HC-05
#include "BluetoothSerial.h"

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
/* Unused pins in this sketch, pins exposed near SPI connector
LoRA sx1278 RST:21, DIO0:2
nRF24L0: CE: 22
*/

// NOTE ESP32 defines DAC1 25, DAC2 26, but Kincony has them labelled the other way around...
//constexpr byte DAC1 = 26, DAC2 = 25;

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
PCF8574 pcfIn(PCF8574_IN_ADDRESS, &Wire);
// I2c GPIO expander for relay outputs
PCF8574 pcfOut(PCF8574_OUT_ADDRESS, &Wire);
// RGB LED array
CRGB leds[numLeds];
// RFID
MFRC522 mfrc522(SPI_CS, -1);  // Create MFRC522 instance
uint8_t digitalInputs;

char cmdBuffer[128];

HardwareSerial RS485Serial(1);
HardwareSerial RS232Serial(2);
BluetoothSerial BTSerial;

Scheduler ts;

void rs485PollCallback();
Task rs485PollTask(5000, TASK_FOREVER, &rs485PollCallback);
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

void rs232PollCallback();
Task rs232PollTask(5000, TASK_FOREVER, &rs232PollCallback);
void rs232PollCallback() {
    Serial.print("Sending RS232 at: ");
    Serial.println(millis());
    if (rs232PollTask.getRunCounter() & 1 ) {
      RS232Serial.write("X");
    }
    else {
      RS232Serial.write("Y");
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

void readMacAddress(){
  // Variable to store the MAC address
  uint8_t baseMac[6];
  
  // Get MAC address of the WiFi station interface
  esp_read_mac(baseMac, ESP_MAC_WIFI_STA);
  Serial.print("Station MAC: ");
  for (int i = 0; i < 5; i++) {
    Serial.printf("%02X:", baseMac[i]);
  }
  Serial.printf("%02X\n", baseMac[5]);
  
  // Get the MAC address of the Wi-Fi AP interface
  esp_read_mac(baseMac, ESP_MAC_WIFI_SOFTAP);
  Serial.print("SoftAP MAC: ");
  for (int i = 0; i < 5; i++) {
    Serial.printf("%02X:", baseMac[i]);
  }
  Serial.printf("%02X\n", baseMac[5]);
  
  // Get the MAC address of the Bluetooth interface
  esp_read_mac(baseMac, ESP_MAC_BT);
  Serial.print("Bluetooth MAC: ");
  for (int i = 0; i < 5; i++) {
    Serial.printf("%02X:", baseMac[i]);
  }
  Serial.printf("%02X\n", baseMac[5]);

  // Get the MAC address of the Ethernet interface
  esp_read_mac(baseMac, ESP_MAC_ETH);
  Serial.print("Ethernet MAC: ");
  for (int i = 0; i < 5; i++) {
    Serial.printf("%02X:", baseMac[i]);
  }
  Serial.printf("%02X\n", baseMac[5]);
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println(F("Kincony KC868-A6 Setup"));

  readMacAddress();

  pinMode(DAC2, OUTPUT);

  Serial.println(F("Testing DAC Outputs"));
  dacWrite(DAC1, 255);
  delay(1000);
  dacWrite(DAC1, 0);


  Serial.print("Starting RS485 serial interface...");
  RS485Serial.begin(9600, SERIAL_8N1, RS485_RX, RS485_TX);
  RS485Serial.write("X");

  Serial.print("Starting RS232 serial interface...");
  RS232Serial.begin(9600, SERIAL_8N1, RS232_RX, RS232_TX);
  RS232Serial.write("Y");

  Serial.print("Starting Bluetooth serial interface...");
  BTSerial.begin("KC868-A6");

  Serial.print("Starting scheduler...");
  ts.init();
  ts.addTask(rs485PollTask);
  Serial.print("added RS485 pollTask...");
  rs485PollTask.enable();
  Serial.println("pollTask enabled");

  ts.addTask(rs232PollTask);
  Serial.print("added RS232 pollTask...");
  rs232PollTask.enable();
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

  u8g2.drawStr(0,30,"RS232:");
  u8g2.drawStr(0,40,"RS485:");
  u8g2.drawStr(0,50,"BT:");


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
  if(pcfIn.begin()){
    Serial.println(F("done."));
  }
  else {
    Serial.println(F("Error initialising PCF8574 input!"));
  }

  // Outputs
  Serial.print(F("Initialising Outputs..."));
  if (pcfOut.begin()){
    for(int i=0; i<numOutputs; i++){
        pcfOut.write(i, LOW);
        delay(250);
    }
    for(int i=0; i<numOutputs; i++){
      pcfOut.write(i, HIGH);
      delay(250);
    }
    Serial.println(F("done."));
  }
  else {
    Serial.println(F("Error initialising PCF8574 output!"));
  }


/*
  // SPI Interface
	SPI.begin(18, 19, 23, 5); // Init SPI bus
	mfrc522.PCD_Init();		// Init MFRC522
	delay(10);				// Optional delay. Some board do need more time after init to be ready, see Readme
	mfrc522.PCD_DumpVersionToSerial();	// Show details of PCD - MFRC522 Card Reader details
*/
}

void loop() {

  int val = beatsin8( 6, 0, 255 );
  dacWrite(DAC1, val);

  // Service task scheduler
  ts.execute();
/*
	// Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
	if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
	  // Dump debug info about the card; PICC_HaltA() is automatically called
	  mfrc522.PICC_DumpToSerial(&(mfrc522.uid));
	}
*/
  if(RS485Serial.available()){
    Serial.print("RS485 Data Received!");
    uint8_t bufferIndex = 0;
    memset(cmdBuffer, 0, sizeof cmdBuffer);
    while(RS485Serial.available()){
      char c = RS485Serial.read();
      cmdBuffer[bufferIndex] = c;
      bufferIndex++;
      //Serial.write(c);
      //u8g2.setDrawColor(0);
      //u8g2.drawBox(40, 32, 10, 10);
      //u8g2.setCursor(40, 40);
      //u8g2.setDrawColor(1);
      //u8g2.print(c);
    }
    cmdBuffer[bufferIndex] = '\0';
    Serial.println(cmdBuffer);
  }

  if(RS232Serial.available()){
    Serial.print("RS232 Data Received!");
    char c = RS232Serial.read();
    Serial.write(c);
    u8g2.setDrawColor(0);
    u8g2.drawBox(40, 22, 10, 10);
    u8g2.setCursor(40, 30);
    u8g2.setDrawColor(1);
    u8g2.print(c);
  }

  if(BTSerial.available()){
    Serial.print("Bluetooth Data Received!");
    char c = BTSerial.read();
    Serial.write(c);
    u8g2.setDrawColor(0);
    u8g2.drawBox(40, 42, 10, 10);
    u8g2.setCursor(40, 50);
    u8g2.setDrawColor(1);
    u8g2.print(c);
    switch(c){
      case '1':
      pcfOut.toggle(0);
      break;
      case '2':
      pcfOut.toggle(1);
      break;
      case '3':
      pcfOut.toggle(2);
      break;
      case '4':
      pcfOut.toggle(3);
      break;
      case '5':
      pcfOut.toggle(4);
      break;
      case '6':
      pcfOut.toggle(5);
      break;
    }
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

  uint8_t tempDigitalInputs = pcfIn.read8();
  if(tempDigitalInputs != digitalInputs){
    digitalInputs = tempDigitalInputs;
    Serial.print("Inputs:");
    Serial.println(digitalInputs, BIN);
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

