#ifndef _SI7021_H_
#define _SI7021_H_

/**************************************************************************
  @file   Si7021.cpp
  @author   Limor Fried (Adafruit Industries), Vojta Molda
  @license  BSD (see license.txt)
 
  This is a library for the Adafruit Si7021 breakout board
  ----> https://www.adafruit.com/products/3251
 
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!
 
  @section  HISTORY
   v1.0  - First release
   v1.1  - Modified to use <SoftWire.h>
**************************************************************************/

#include <Arduino.h>
#include <SoftwareWire.h>


class Si7021 {
  
  public:
  
  Si7021(SoftwareWire& Wire_);
  bool begin(void);
  void reset(void);
  void readSerialNumber(void);
  float readTemperature(void);
  float readHumidity(void);
  uint32_t sernum_a, sernum_b;
  
  
  private:

  uint8_t readRegister8(uint8_t reg);
  uint16_t readRegister16(uint8_t reg);
  void writeRegister8(uint8_t reg, uint8_t value);

  SoftwareWire& Wire;
  const static struct WireCommands {
    static const uint8_t ADDRESS = 0x40;
    static const uint8_t MEASRH_HOLD = 0xE5;
    static const uint8_t MEASRH_NOHOLD = 0xF5;
    static const uint8_t MEASTEMP_HOLD = 0xE3;
    static const uint8_t MEASTEMP_NOHOLD = 0xF3;
    static const uint8_t READPREVTEMP = 0xE0;
    static const uint8_t RESET = 0xFE;
    static const uint8_t WRITERHT_REG = 0xE6;
    static const uint8_t READRHT_REG = 0xE7;
    static const uint8_t WRITEHEATER_REG = 0x51;
    static const uint8_t READHEATER_REG = 0x11;
    static const uint16_t ID1 = 0xFA0F;
    static const uint16_t ID2 = 0xFCC9;
    static const uint16_t FIRMVERS = 0x84B8;
  } Cmd;
  
};

#endif

