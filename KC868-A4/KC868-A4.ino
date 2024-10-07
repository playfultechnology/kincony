/**
 * Kincony KC868-A4
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

// Storage for the received IR code
struct storedIRDataStruct {
    IRData receivedIRData;
    uint8_t rawCode[RAW_BUFFER_LENGTH]; // The durations if raw
    uint8_t rawCodeLength; // The length of the code
} sStoredIRData;

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
constexpr byte RF433Rx = 19;
constexpr byte RF433Tx = 21;
// IR Rx/Tx pins
constexpr byte irRx = 23;
constexpr byte irTx = 22;
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

  if(btn == inputButtons[2]){
    Serial.println("Transmitting code");
    IrReceiver.stop();
    sendCode(&sStoredIRData);
  }
}
void onRelease(Button2& btn) {
  for(int i=0; i<4; i++) {
    if(btn == inputButtons[i]){
      digitalWrite(relayPins[i], LOW);
    }
  }
  if(btn == inputButtons[2]){
    IrReceiver.start();
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

  Serial.println(F("Starting IR receiver"));
  IrReceiver.begin(irRx, ENABLE_LED_FEEDBACK);

  Serial.println(F("Starting IR sender"));
  IrSender.begin(irTx, ENABLE_LED_FEEDBACK, USE_DEFAULT_FEEDBACK_LED_PIN);

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
  // If an IR transmission has been received
  if(IrReceiver.decode()) {
    Serial.println("Storing code");
    storeCode();
    IrReceiver.resume();
  }
}

void storeCode() {
  // Check data is valid
  if(IrReceiver.decodedIRData.rawDataPtr->rawlen < 4) {
    Serial.print(F("Ignore data with rawlen="));
    Serial.println(IrReceiver.decodedIRData.rawDataPtr->rawlen);
    return;
  }
  if(IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT) {
    Serial.println(F("Ignore repeat"));
    return;
  }
  if(IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_AUTO_REPEAT) {
    Serial.println(F("Ignore autorepeat"));
    return;
  }
  if(IrReceiver.decodedIRData.flags & IRDATA_FLAGS_PARITY_FAILED) {
    Serial.println(F("Ignore parity error"));
    return;
  }

  // Store data
  sStoredIRData.receivedIRData = IrReceiver.decodedIRData;

  // Logging
  if (sStoredIRData.receivedIRData.protocol == UNKNOWN) {
    Serial.print(F("Received unknown code and store "));
    Serial.print(IrReceiver.decodedIRData.rawDataPtr->rawlen - 1);
    Serial.println(F(" timing entries as raw "));
    IrReceiver.printIRResultRawFormatted(&Serial, true); // Output the results in RAW format
    sStoredIRData.rawCodeLength = IrReceiver.decodedIRData.rawDataPtr->rawlen - 1;
    // Store the current raw data in a dedicated array for later usage
    IrReceiver.compensateAndStoreIRResultInArray(sStoredIRData.rawCode);
  }
  else {
    IrReceiver.printIRResultShort(&Serial);
    IrReceiver.printIRSendUsage(&Serial);
    sStoredIRData.receivedIRData.flags = 0; // clear flags -esp. repeat- for later sending
    Serial.println();
  }
}

void sendCode(storedIRDataStruct *aIRDataToSend) {
  if (aIRDataToSend->receivedIRData.protocol == UNKNOWN /* i.e. raw */) {
    // Assume 38 KHz
    IrSender.sendRaw(aIRDataToSend->rawCode, aIRDataToSend->rawCodeLength, 38);
    Serial.print(F("raw "));
    Serial.print(aIRDataToSend->rawCodeLength);
    Serial.println(F(" marks or spaces"));
  } 
  else {
    // Write function switches for appropriate protocol
    IrSender.write(&aIRDataToSend->receivedIRData);
    printIRResultShort(&Serial, &aIRDataToSend->receivedIRData, false);
  }
}

