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

#include "Si7021.h"


Si7021::Si7021(SoftwareWire& Wire_) :
   sernum_a(0), sernum_b(0), Wire(Wire_)
{ }

bool Si7021::begin(void) {
  Wire.begin();
  
  reset();
  if (readRegister8(Si7021::Cmd.READRHT_REG) != 0x3A) return false;
  readSerialNumber();
  return true;
}

float Si7021::readHumidity(void) {
  Wire.beginTransmission(Si7021::Cmd.ADDRESS);
  Wire.write(Si7021::Cmd.MEASRH_NOHOLD);
  Wire.endTransmission(false);
  delay(25);
  
  Wire.requestFrom(Si7021::Cmd.ADDRESS, (uint8_t)3);
  uint16_t hum = Wire.read();
  hum <<= 8;
  hum |= Wire.read();
  uint8_t chxsum = Wire.read();
  
  float humidity = hum;
  humidity *= 125;
  humidity /= 65536;
  humidity -= 6;
  
  return humidity;
}

float Si7021::readTemperature(void) {
  Wire.beginTransmission(Si7021::Cmd.ADDRESS);
  Wire.write(Si7021::Cmd.MEASTEMP_NOHOLD);
  Wire.endTransmission(false);
  delay(25);
  
  Wire.requestFrom(Si7021::Cmd.ADDRESS, (uint8_t)3);
  uint16_t temp = Wire.read();
  temp <<= 8;
  temp |= Wire.read();
  uint8_t chxsum = Wire.read();
  
  float temperature = temp;
  temperature *= 175.72;
  temperature /= 65536;
  temperature -= 46.85;
  
  return temperature;
}

void Si7021::reset(void) {
  Wire.beginTransmission(Si7021::Cmd.ADDRESS);
  Wire.write(Si7021::Cmd.RESET);
  Wire.endTransmission();
  delay(50);
}

void Si7021::readSerialNumber(void) {
  Wire.beginTransmission(Si7021::Cmd.ADDRESS);
  Wire.write(Si7021::Cmd.ID1 >> 8);
  Wire.write(Si7021::Cmd.ID1 & 0xFF);
  Wire.endTransmission();
  
  Wire.requestFrom(Si7021::Cmd.ADDRESS, (uint8_t)8);
  sernum_a = Wire.read();
  Wire.read();
  sernum_a <<= 8;
  sernum_a |= Wire.read();
  Wire.read();
  sernum_a <<= 8;
  sernum_a |= Wire.read();
  Wire.read();
  sernum_a <<= 8;
  sernum_a |= Wire.read();
  Wire.read();
  
  Wire.beginTransmission(Si7021::Cmd.ADDRESS);
  Wire.write(Si7021::Cmd.ID2 >> 8);
  Wire.write(Si7021::Cmd.ID2 & 0xFF);
  Wire.endTransmission();
  
  Wire.requestFrom(Si7021::Cmd.ADDRESS, (uint8_t)8);
  sernum_b = Wire.read();
  Wire.read();
  sernum_b <<= 8;
  sernum_b |= Wire.read();
  Wire.read();
  sernum_b <<= 8;
  sernum_b |= Wire.read();
  Wire.read();
  sernum_b <<= 8;
  sernum_b |= Wire.read();
  Wire.read();
}


void Si7021::writeRegister8(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(Si7021::Cmd.ADDRESS);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

uint8_t Si7021::readRegister8(uint8_t reg) {
  uint8_t value;
  Wire.beginTransmission(Si7021::Cmd.ADDRESS);
  Wire.write(reg);
  Wire.endTransmission(false);
  
  Wire.requestFrom(Si7021::Cmd.ADDRESS, (uint8_t)1);
  value = Wire.read();
  
  return value;
}

uint16_t Si7021::readRegister16(uint8_t reg) {
  uint16_t value;
  Wire.beginTransmission(Si7021::Cmd.ADDRESS);
  Wire.write(reg);
  Wire.endTransmission();
  
  Wire.requestFrom(Si7021::Cmd.ADDRESS, (uint8_t)2);
  value = Wire.read();
  value <<= 8;
  value |= Wire.read();
  
  return value;
}
