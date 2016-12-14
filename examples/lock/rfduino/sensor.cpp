#include "sensor.h"

// #define DEBUG(message) Serial.print(message)
#define DEBUG(message)


Sensor::Sensor(uint32_t pin_) :
  pin(pin_)
{
  pinMode(pin, INPUT);
}

Sensor::operator int() {
  int retval = digitalRead(pin);
  DEBUG("Sensor::int(pin="); DEBUG(pin); DEBUG(", val="); DEBUG(retval); DEBUG(")\n");
  return retval;
}

