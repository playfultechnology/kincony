/*
  This is a hardware abstraction layer, it tells the library how to use the
  serial port on an Arduino board (and which port).
*/
#ifdef ARDUINO
#include "DYPlayerArduino.h"

namespace DY
{
  Player::Player()
  {
    this->port = &Serial;
    this->isSoftSerial = false;
  }
#ifdef HAS_HARDWARE_SERIAL
  Player::Player(HardwareSerial *port)
  {
    this->port = (Stream *)port;
    this->isSoftSerial = false;
  }
#endif

#ifdef HAS_SOFTWARE_SERIAL
	#ifdef ESP32
	  Player::Player(EspSoftwareSerial::UART *port)
	  {
		this->port = (Stream *)port;
		this->isSoftSerial = true;
	  }
	#else
	  Player::Player(SoftwareSerial *port)
	  {
		this->port = (Stream *)port;
		this->isSoftSerial = true;
	  }
  #endif
#endif
  void Player::begin()
  {
    if (isSoftSerial)
    {
#ifdef HAS_SOFTWARE_SERIAL
	#ifdef ESP32
      ((EspSoftwareSerial::UART *)port)->begin(9600);
      while (!((SoftwareSerial *)port))
        ; // wait for port to be connected
	#else
      ((SoftwareSerial *)port)->begin(9600);
      while (!((SoftwareSerial *)port))
        ; // wait for port to be connected
	#endif
#endif
    }
    else
    {
#ifdef HAS_HARDWARE_SERIAL
      ((HardwareSerial *)port)->begin(9600);
      while (!((HardwareSerial *)port))
        ; // wait for port to be connected
#endif
    }
  }
  void Player::serialWrite(uint8_t *buffer, uint8_t len)
  {
    port->write(buffer, len);
  }
  bool Player::serialRead(uint8_t *buffer, uint8_t len)
  {
    // Serial.setTimeout(1000); // Default timeout 1000ms.
    if (port->readBytes(buffer, len) > 0)
    {
      return true;
    }
    return false;
  }
}
#endif
