#include <Arduino.h>
#include <CurieBLE.h>

const int pinStateLED = 13;

BLEPeripheral ble;
BLEService informationService("180A");
BLECharacteristic modelCharacteristic("2A24", BLERead, "101");
BLECharacteristic manufacturerCharacteristic("2A29", BLERead, "Arduino");
BLECharacteristic serialNumberCharacteristic("2A25", BLERead, "Dummy Testing Lock");

BLEService lockMechanismService("43AAF900-5FF0-4633-95A2-FE4189EE103B");
BLEUnsignedCharCharacteristic targetStateCharacteristic("43AAF901-5FF0-4633-95A2-FE4189EE103B", BLEWrite | BLERead | BLENotify);
BLEUnsignedCharCharacteristic currentStateCharacteristic("43AAF902-5FF0-4633-95A2-FE4189EE103B", BLERead | BLENotify);


void setup() {
  Serial.begin(115200);
  pinMode(pinStateLED, OUTPUT);

  ble.setLocalName("Lock");
  ble.setAdvertisedServiceUuid(lockMechanismService.uuid());

  ble.addAttribute(lockMechanismService);
  ble.addAttribute(targetStateCharacteristic);
  ble.addAttribute(currentStateCharacteristic);
  targetStateCharacteristic.setValueLE(0);
  currentStateCharacteristic.setValueLE(0);
  digitalWrite(pinStateLED, 0);

  ble.setEventHandler(BLEConnected, centralConnect);
  ble.setEventHandler(BLEDisconnected, centralDisconnect);

  ble.begin();
  Serial.println("Bluetooth on");
}

void loop() {
  ble.poll();

  if (targetStateCharacteristic.valueLE() != currentStateCharacteristic.valueLE()) {
    unsigned char state = targetStateCharacteristic.valueLE();
    delay(500);
    digitalWrite(pinStateLED, (bool) state);
    currentStateCharacteristic.setValueLE(state);
    Serial.print("Lock state | ");
    Serial.println(state);    
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

