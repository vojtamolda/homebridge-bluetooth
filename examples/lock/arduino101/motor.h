#ifndef _MOTOR_H_
#define _MOTOR_H_

#include <Arduino.h>


class Motor {
  public:
    Motor(const uint32_t minusPin_, const uint32_t plusPin_,
          const uint32_t pwmPin_, const uint8_t pwm_);
    ~Motor();

    void begin();
    void start(bool clockwise = true);
    void stop();

  private:
    const uint32_t minusPin;
    const uint32_t plusPin;
    const uint32_t pwmPin;
    const uint8_t pwm;
};

#endif

