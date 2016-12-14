#include <Arduino.h>
#include <BLEPeripheral.h>
#include "sensor.h"
#include "motor.h"
#include "lock.h"


static struct BoardPins {
  const uint8_t Motor1 = 0;
  const uint8_t Motor2 = 1;
  const uint8_t MotorPWM = 2;
  const uint8_t Sensor1A = 3;
  const uint8_t Sensor1B = 4;
  const uint8_t Sensor2A = 5;
  const uint8_t Sensor2B = 6;
} Pin;


BLEPeripheral ble;
BLEService informationService("180A");
BLECharacteristic modelCharacteristic("2A24", BLERead, "RFduino");
BLECharacteristic manufacturerCharacteristic("2A29", BLERead, "Lockitron");
BLECharacteristic serialNumberCharacteristic("2A25", BLERead, "V1");

BLEService lockMechanismService("43AAF900-5FF0-4633-95A2-FE4189EE103B");
BLEUnsignedCharCharacteristic targetStateCharacteristic("43AAF901-5FF0-4633-95A2-FE4189EE103B",
                                                        BLEWrite | BLERead | BLENotify);
BLEUnsignedCharCharacteristic currentStateCharacteristic("43AAF902-5FF0-4633-95A2-FE4189EE103B",
                                                         BLERead | BLENotify);

Sensor sensor1A(Pin.Sensor1A);
Sensor sensor1B(Pin.Sensor1B);
Sensor sensor2A(Pin.Sensor2A);
Sensor sensor2B(Pin.Sensor2B);
Motor motor(Pin.Motor1, Pin.Motor2, Pin.MotorPWM, 128);
Lock lock = Lock(sensor1A, sensor1B, sensor2A, sensor2B, motor, true);
volatile bool targetStateUpdate = false;


void targetStateWritten(BLECentral& /*central*/, BLECharacteristic& /*characteristic*/) {
   targetStateUpdate = true;
}


void setup() {
  // DEBUG Serial.begin(115200);

  ble.setLocalName("Lock");
  ble.addAttribute(informationService);
  ble.addAttribute(modelCharacteristic);
  ble.addAttribute(manufacturerCharacteristic);
  ble.addAttribute(serialNumberCharacteristic);

  ble.addAttribute(lockMechanismService);
  ble.addAttribute(targetStateCharacteristic);
  ble.addAttribute(currentStateCharacteristic);
  targetStateCharacteristic.setEventHandler(BLEWritten, targetStateWritten);

  ble.begin();
  lock.begin();
}

void loop() {
  ble.poll();
  RFduino_ULPDelay(200);

  if (targetStateUpdate) {
    lock.set(targetStateCharacteristic.value());
    currentStateCharacteristic.setValue(lock.state());
    targetStateUpdate = false;
  }

  if (lock.state() != currentStateCharacteristic.value()) {
    targetStateCharacteristic.setValue(lock.state());
    currentStateCharacteristic.setValue(lock.state());
  }
}

