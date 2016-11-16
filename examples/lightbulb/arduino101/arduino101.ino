#include <Arduino.h>
#include <CurieBLE.h>

const int pinLED = 13;


BLEPeripheral ble;
BLEService informationService("180A");
BLECharacteristic modelCharacteristic("2A24", BLERead, "101");
BLECharacteristic manufacturerCharacteristic("2A29", BLERead, "Arduino");
BLECharacteristic serialNumberCharacteristic("2A25", BLERead, "2.71828");

BLEService lightbulbService("8E76F000-690E-472E-88C3-051277686A73");
BLECharCharacteristic onCharacteristic("8E76F001-690E-472E-88C3-051277686A73", BLEWrite | BLERead | BLENotify);


void setup() {
  Serial.begin(115200);
  pinMode(pinLED, OUTPUT);

  ble.setLocalName("Light");
  ble.setAdvertisedServiceUuid(lightbulbService.uuid());
  ble.addAttribute(informationService);
  ble.addAttribute(modelCharacteristic);
  ble.addAttribute(manufacturerCharacteristic);
  ble.addAttribute(serialNumberCharacteristic);

  ble.addAttribute(lightbulbService);
  ble.addAttribute(onCharacteristic);

  bool on = true;
  onCharacteristic.setValueLE(on);
  setLED(on);

  ble.setEventHandler(BLEConnected, centralConnect);
  ble.setEventHandler(BLEDisconnected, centralDisconnect);
  onCharacteristic.setEventHandler(BLEWritten, characteristicWrite);

  ble.begin();
  Serial.println("Bluetooth on");
}

void loop() {
  ble.poll();
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
  setLED(on);
}

void setLED(bool on) {
  Serial.print("LED | ");
  Serial.print(on);
  digitalWrite(pinLED, on);
}
