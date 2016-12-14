#include "motor.h"

// #define DEBUG(message) Serial.print(message)
#define DEBUG(message)


Motor::Motor(const uint32_t minusPin_, const uint32_t plusPin_,
             const uint32_t pwmPin_, const uint8_t pwm_) :
  minusPin(minusPin_),
  plusPin(plusPin_),
  pwmPin(pwmPin_),
  pwm(pwm_)
{ }

void Motor::begin() {
  pinMode(minusPin, OUTPUT);
  pinMode(plusPin, OUTPUT);
  pinMode(pwmPin, OUTPUT);
}

void Motor::start(bool clockwise) {
  analogWrite(pwmPin, pwm);
  if (clockwise) {
    digitalWrite(minusPin, HIGH);
    digitalWrite(plusPin, LOW);
  } else {
    digitalWrite(minusPin, LOW);
    digitalWrite(plusPin, HIGH);
  }
  DEBUG("Motor::start(clkws="); DEBUG(clockwise); DEBUG(")\n");
}

void Motor::stop() {
  analogWrite(pwmPin, 0);
  digitalWrite(minusPin, LOW);
  digitalWrite(plusPin, LOW);
  DEBUG("Motor::stop()\n");
}

Motor::~Motor() {
  stop();
  DEBUG("Motor::destructor()\n");
}

