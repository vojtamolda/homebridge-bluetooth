#ifndef _LOCK_H_
#define _LOCK_H_

#include <Arduino.h>

#include "sensor.h"
#include "motor.h"


class Lock {
  public:
    Lock(Sensor& sensor1A_, Sensor& sensor1B_, Sensor& sensor2A_, Sensor& sensor2B_,
           Motor& motor_, const bool lockClockwise_);

    void begin();
    uint8_t state(void);
    bool set(uint8_t state);

    const static struct LockState { // HAP-NodeJS/lib/gen/HomeKitTypes.js #897
      static const uint8_t Unsecured = 0;
      static const uint8_t Secured = 1;
      static const uint8_t Jammed = 2;
      static const uint8_t Unknown = 3;
    } State;

  private:
    bool rotate(uint8_t blck, uint8_t orng, bool clockwise = true, bool sure = true);
    uint8_t orange();
    uint8_t black();

    const static struct RingPosition {
      static const uint8_t Unlocked = 0;
      static const uint8_t Locked = 1;
      static const uint8_t Jammed = 2;
      static const uint8_t Unknown = 3;
      static const uint8_t Neutral = 4;
      static const uint8_t Any = ~0;
    } Position;

    const static struct LockTimeDelay {
      static const uint32_t Timeout = 3000; // milliseconds
      static const uint32_t DirectionChange = 300; // milliseconds
      static const uint32_t Override = 500; // milliseconds - needs to be tuned for each lock
    } Delay;

    bool working;
    uint32_t timeout; // milliseconds - overflows after ~50 days
    uint8_t lastState;

    Sensor& sensor1A;
    Sensor& sensor1B;
    Sensor& sensor2A;
    Sensor& sensor2B;
    Motor& motor;

    const bool lockClockwise;
};

#endif
