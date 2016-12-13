#include <Arduino.h>
#include <CurieBLE.h>
#include <SoftwareWire.h>
#include "Si7021.h"

static struct BoardPins {
  const uint8_t VIN = A0;
  const uint8_t VO3 = A1;
  const uint8_t GND = A2;
  const uint8_t SCL = A3;
  const uint8_t SDA = A4;
} Pin;

SoftwareWire wire = SoftwareWire(Pin.SDA, Pin.SCL);
Si7021 sensor = Si7021(wire);

BLEPeripheral ble;
BLEService informationService("180A");
BLECharacteristic modelCharacteristic("2A24", BLERead, "101");
BLECharacteristic manufacturerCharacteristic("2A29", BLERead, "Arduino");
BLECharacteristic serialNumberCharacteristic("2A25", BLERead, "2.71828");

BLEService humidityService("10525F60-CF73-11E6-9598-F8E2251C9A69");
BLEFloatCharacteristic humidityCharacteristic("10525F61-CF73-11E6-9598-F8E2251C9A69", BLERead | BLENotify);
BLEService thermometerService("F489CB00-C177-11E6-9598-9854249C9A66");
BLEFloatCharacteristic temperatureCharacteristic("F489CB01-C177-11E6-9598-9854249C9A66", BLERead | BLENotify);


void setup() {
  Serial.begin(115200);

  pinMode(Pin.VIN, OUTPUT);
  digitalWrite(Pin.VIN, HIGH);
  pinMode(Pin.GND, OUTPUT);
  digitalWrite(Pin.GND, LOW);
  sensor.begin();

  ble.setLocalName("Humidity");
  ble.setAdvertisedServiceUuid(thermometerService.uuid());
  ble.addAttribute(informationService);
  ble.addAttribute(modelCharacteristic);
  ble.addAttribute(manufacturerCharacteristic);
  ble.addAttribute(serialNumberCharacteristic);

  ble.addAttribute(humidityService);
  ble.addAttribute(humidityCharacteristic);
  humidityCharacteristic.setValueLE(0.0);
  ble.addAttribute(thermometerService);
  ble.addAttribute(temperatureCharacteristic);
  temperatureCharacteristic.setValueLE(0.0);

  ble.setEventHandler(BLEConnected, centralConnect);
  ble.setEventHandler(BLEDisconnected, centralDisconnect);

  ble.begin();
  Serial.println("Bluetooth on");
}

void loop() {
  ble.poll();

  static float averageHumidity = sensor.readHumidity();
  averageHumidity += (sensor.readTemperature() - averageHumidity) / 30.0;
  float previousHumidity = humidityCharacteristic.valueLE();
  if (abs(averageHumidity - previousHumidity) > 1.50) {
    humidityCharacteristic.setValueLE(averageHumidity);
    Serial.print("Update temperature | ");
    Serial.println(averageHumidity, 0);
  } else {
    Serial.print("Temperature | ");
    Serial.println(averageHumidity, 0);
  }

  static float averageCelsius = sensor.readTemperature();
  averageCelsius += (sensor.readTemperature() - averageCelsius) / 30.0;
  float previousCelsius = temperatureCharacteristic.valueLE();
  if (abs(averageCelsius - previousCelsius) > 0.20) {
    temperatureCharacteristic.setValueLE(averageCelsius);
    Serial.print("Update temperature | ");
    Serial.println(averageCelsius, 2);
  } else {
    Serial.print("Temperature | ");
    Serial.println(averageCelsius, 2);
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
