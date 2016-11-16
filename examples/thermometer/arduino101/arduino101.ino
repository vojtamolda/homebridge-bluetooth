#include <Arduino.h>
#include <CurieBLE.h>
#include <CurieIMU.h>


BLEPeripheral ble;
BLEService informationService("180A");
BLECharacteristic modelCharacteristic("2A24", BLERead, "101");
BLECharacteristic manufacturerCharacteristic("2A29", BLERead, "Arduino");
BLECharacteristic serialNumberCharacteristic("2A25", BLERead, "2.71828");

BLEService thermometerService("1D8A68E0-E68E-4FED-943E-369099F5B499");
BLEFloatCharacteristic temperatureCharacteristic("1D8A68E1-E68E-4FED-943E-369099F5B499", BLERead | BLENotify);


void setup() {
  Serial.begin(115200);

  ble.setLocalName("Thermo");
  ble.setAdvertisedServiceUuid(thermometerService.uuid());
  ble.addAttribute(informationService);
  ble.addAttribute(modelCharacteristic);
  ble.addAttribute(manufacturerCharacteristic);
  ble.addAttribute(serialNumberCharacteristic);

  ble.addAttribute(thermometerService);
  ble.addAttribute(temperatureCharacteristic);
  temperatureCharacteristic.setValueLE(0.0);

  ble.setEventHandler(BLEConnected, centralConnect);
  ble.setEventHandler(BLEDisconnected, centralDisconnect);

  ble.begin();
  CurieIMU.begin();
  Serial.println("Bluetooth on");
}

void loop() {
  ble.poll();

  float currentCelsius = (float) CurieIMU.readTemperature() / 32767.0 + 23.0;
  float previousCelsius = temperatureCharacteristic.valueLE();
  if (abs(currentCelsius - previousCelsius) > 0.05) {
    temperatureCharacteristic.setValueLE(currentCelsius);
    Serial.print("Update temperature | ");
    Serial.println(currentCelsius);
  } else {
    Serial.print("Temperature | ");
    Serial.println(currentCelsius);
  }
  delay(1000);
}


void centralConnect(BLECentral& central) {
  Serial.print("Central connected | ");
  Serial.println(central.address());
}

void centralDisconnect(BLECentral& central) {
  Serial.print("Central disconnected | ");
  Serial.println(central.address());
}

