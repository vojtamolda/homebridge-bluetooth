#include "lock.h"

// #define DEBUG(message) Serial.print(message)
#define DEBUG(message)


Lock::Lock(Sensor& sensor1A_, Sensor& sensor1B_, Sensor& sensor2A_, Sensor& sensor2B_,
           Motor& motor_, const bool lockClockwise_) :
  working(false), sensor1A(sensor1A_), sensor1B(sensor1B_), sensor2A(sensor2A_), sensor2B(sensor2B_),
  motor(motor_), lockClockwise(lockClockwise_)
{ }

void Lock::begin() {
  DEBUG("Lock::begin()\n");
  motor.begin();
  rotate(Position.Neutral, Position.Any, false, false);
  lastState = Position.Unlocked;
}

uint8_t Lock::state() {
  uint8_t orng = orange();
  if (orng == Position.Jammed) {
    lastState = Position.Jammed;
    return State.Jammed;
  } else if (orng == Position.Unlocked) {
    if (lastState != Position.Unlocked) {
      DEBUG("Lock::state() = Unsecured (Manually)\n");
      lastState = Position.Unlocked;
    }
    return State.Unsecured;
  } else if (orng == Position.Locked) {
    if (lastState != Position.Locked) {
      DEBUG("Lock::state() = Secured (Manually)\n");
      lastState = Position.Locked;
    }
    return State.Secured;
  } else if (orng == Position.Unknown) {
    return lastState;
  }
}

bool Lock::set(uint8_t state) {
  if (state == State.Secured) {
    DEBUG("Lock::set(Secured)\n");
    if (!rotate(Position.Any, Position.Locked, lockClockwise)) {
      DEBUG("Full lock failed\n");
      return false;
    }
    delay(Delay.DirectionChange);
    if (!rotate(Position.Neutral, Position.Any, !lockClockwise)) {
      DEBUG("Parking back to neutral failed\n");
      return false;
    }
    lastState = state;
    return true;
  } else if (state == State.Unsecured) {
    DEBUG("Lock::set(Unsecured)\n");
    if (!rotate(Position.Any, Position.Unlocked, !lockClockwise)) {
      DEBUG("Full unlock failed\n");
      return false;
    }
    delay(Delay.DirectionChange);
    if (!rotate(Position.Neutral, Position.Any, lockClockwise)) {
      DEBUG("Parking back to neutral failed\n");
      return false;
    }
    lastState = state;
    return true;
  } else {
    DEBUG("Lock::set(Neutral)\n");
    rotate(Position.Neutral, Position.Any, false, false);
    lastState = state;
    return true;
  }
}

bool Lock::rotate(uint8_t blck, uint8_t orng, bool clockwise, bool sure) {
  DEBUG("Lock::rotate(blck="); DEBUG(blck); DEBUG(", orng="); DEBUG(orng);
  DEBUG(", clkws="); DEBUG(clockwise); DEBUG(")\n");
  static bool recursion = false;

  working = true;
  timeout = millis();
  motor.start(clockwise);

  do {
    if ((!sure) && (blck == Position.Neutral)) {
      if ((black() == Position.Locked) || (black() == Position.Unlocked)) {
        /* Wrong direction while trying to reach mid-point, back out */
        DEBUG("Reverse\n");
        motor.stop();
        working = false;
        delay(Delay.DirectionChange);
        if (!recursion) {
          bool direction = (black() == Position.Unlocked);
          bool retval = rotate(Position.Neutral, Position.Any, direction);
          return retval;
        }
      }
    }
    if (black() == Position.Jammed) {
      /* Ooooppss... */
      DEBUG("Jammed!!!\n");
      motor.stop();
      delay(Delay.DirectionChange);
      if (!recursion) {
        recursion = true;
        uint32_t keep = timeout;
        rotate(Position.Neutral, Position.Any, !clockwise);
        recursion = false;
        timeout = keep;
        working = true;
      }
      return false;
    }
    /* Keep cranking... */
  } while ( ((black() != blck) || (blck == Position.Any)) &&
           ((orange() != orng) || (orng == Position.Any)) );
  
  if ((orng == Position.Locked) || (orng == Position.Unlocked)) {
    /* Keep cranking for a bit longer when locking/unlocking to fully extend/retract the bolt */
    delay(Delay.Override);
  }

  motor.stop();
  working = false;
  return true;
}

uint8_t Lock::black() {
  DEBUG("Lock:black(2A="); DEBUG(sensor2A); DEBUG(", 2B="); DEBUG(sensor2B); DEBUG(")");

  if (working && ((millis() - timeout) > Delay.Timeout)) {
    DEBUG(" = Jammed\n");
    return Position.Jammed;
  }

  if (lockClockwise) {
    if ((sensor2A == HIGH) && (sensor2B == HIGH)) {
      /* Black ring parked at neutral mid-point */
      DEBUG(" = Neutral\n");
      return Position.Neutral;
    }
    if ((sensor2A == LOW ) && (sensor2B == HIGH)) {
      /* Black ring rotated all the way clockwise */
      DEBUG(" = Locked\n");
      return Position.Locked;
    }
    if ((sensor2A == HIGH) && (sensor2B == LOW )) {
      /* Black ring rotated all the way counter-clockwise */
      DEBUG(" = Unlocked\n");
      return Position.Unlocked;
    }
    DEBUG(" = Unknown\n");
    return Position.Unknown;
  } else {
    if ((sensor2A == HIGH) && (sensor2B == HIGH)) {
      /* Black ring parked at neutral mid-point */
      DEBUG(" = Neutral\n");
      return Position.Neutral;
    }
    if ((sensor2A == LOW ) && (sensor2B == HIGH)) {
      /* Black ring rotated all the way clockwise */
      DEBUG(" = Unlocked\n");
      return Position.Unlocked;
    }
    if ((sensor2A == HIGH) && (sensor2B == LOW )) {
      /* Black ring rotated all the way counter-clockwise */
      DEBUG(" = Locked\n");
      return Position.Locked;
    }
    DEBUG(" = Unknown\n");
    return Position.Unknown;
  }
}

uint8_t Lock::orange() {
  if (working && ((millis() - timeout) > Delay.Timeout)) {
    return Position.Jammed;
  }

  if (lockClockwise) {
    if ((sensor1A == HIGH) && (sensor1B == LOW )) {
      /* Orange ring rotated all the way clockwise relative to the black ring */
      return Position.Unlocked;
    }
    if ((sensor1A == LOW ) && (sensor1B == HIGH)) {
      /* Orange ring rotated all the way counter-clockwise relative to the black ring */
      return Position.Locked;
    }
    return Position.Unknown;
  } else {
    if ((sensor1A == HIGH) && (sensor1B == LOW )) {
      /* Orange ring rotated all the way clockwise relative to the black ring */
      return Position.Locked;
    }
    if ((sensor1A == LOW ) && (sensor1B == HIGH)) {
      /* Orange ring rotated all the way counter-clockwise relative to the black ring */
      return Position.Unlocked;
    }
    return Position.Unknown;
  }
}

