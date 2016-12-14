#ifndef _SENSOR_H_
#define _SENSOR_H_

#include <Arduino.h>


class Sensor {
  public:
    Sensor(uint32_t pin_);

    operator int();

  private:
    const uint32_t pin;
};

#endif

