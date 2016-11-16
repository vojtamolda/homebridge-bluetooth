#include <Arduino.h>
#include <CurieBLE.h>

const int pinSwitch = 2;


BLEPeripheral ble;
BLEService informationService("180A");
BLECharacteristic modelCharacteristic("2A24", BLERead, "101");
BLECharacteristic manufacturerCharacteristic("2A29", BLERead, "Arduino");
BLECharacteristic serialNumberCharacteristic("2A25", BLERead, "2.71828");

BLEService switchService("5ECEC5A0-7F71-41DA-9B8C-6252FE7EFB7E");
BLECharCharacteristic onCharacteristic("5ECEC5A1-7F71-41DA-9B8C-6252FE7EFB7E", BLEWrite | BLERead | BLENotify);
bool lastSwitch = HIGH;


void setup() {
  Serial.begin(115200);
  pinMode(pinSwitch, INPUT_PULLUP);

  ble.setLocalName("Switch");
  ble.setAdvertisedServiceUuid(switchService.uuid());
  ble.addAttribute(informationService);
  ble.addAttribute(modelCharacteristic);
  ble.addAttribute(manufacturerCharacteristic);
  ble.addAttribute(serialNumberCharacteristic);

  ble.addAttribute(informationService);
  ble.addAttribute(modelCharacteristic);
  ble.addAttribute(manufacturerCharacteristic);
  ble.addAttribute(serialNumberCharacteristic);

  ble.addAttribute(switchService);
  ble.addAttribute(onCharacteristic);
  onCharacteristic.setValueLE(false);

  ble.setEventHandler(BLEConnected, centralConnect);
  ble.setEventHandler(BLEDisconnected, centralDisconnect);
  onCharacteristic.setEventHandler(BLEWritten, characteristicWrite);

  ble.begin();
  Serial.println("Bluetooth on");
  lastSwitch = digitalRead(pinSwitch);
}

void loop() {
  ble.poll();

  if ((digitalRead(pinSwitch) == LOW) && (lastSwitch == HIGH)) {
    bool on = (bool) onCharacteristic.valueLE();
    onCharacteristic.setValue(!on);
    Serial.print("Switch | ");
    Serial.println(!on);
    ble.poll();
    lastSwitch = LOW;
    delay(1);
  } else if ((digitalRead(pinSwitch) == HIGH) && (lastSwitch == LOW)) {
    lastSwitch = HIGH;
    delay(1);
  }
}


void centralConnect(BLECentral& central) {
  Serial.print("Central connected | ");
  Serial.println(central.address());
}

void centralDisconnect(BLECentral& central) {
  Serial.print("Central disconnected | ");
  Serial.println(central.address());
}

void characteristicWrite(BLECentral& central, BLECharacteristic& characteristic) {
  Serial.print("Characteristic written | ");
  Serial.println(characteristic.uuid());

  bool on = (bool) onCharacteristic.valueLE();
  Serial.print("Switch | ");
  Serial.println(on);
}

