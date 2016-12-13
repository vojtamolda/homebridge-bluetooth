#include <SPI.h>
#include <Arduino.h>

#include <Adafruit_BLE.h>
#include <Adafruit_BLEGatt.h>
#include <Adafruit_BluefruitLE_SPI.h>

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

Adafruit_BluefruitLE_SPI ble(8, 7, 4); // Firmware > 0.7.0
Adafruit_BLEGatt gatt(ble);
int32_t humidityService;
int32_t humidityCharacteristic;
int32_t thermometerService;
int32_t temperatureCharacteristic;

union float_bytes {
  float value;
  uint8_t bytes[sizeof(float)];
};


void setup(void) {
  Serial.begin(115200);

  pinMode(Pin.VIN, OUTPUT);
  digitalWrite(Pin.VIN, HIGH);
  pinMode(Pin.GND, OUTPUT);
  digitalWrite(Pin.GND, LOW);
  sensor.begin();

  ble.begin(/* true - useful for debugging */);
  ble.factoryReset();
  ble.info();

  uint8_t humidityServiceUUID[] = {0x10,0x52,0x5F,0x60,0xCF,0x73,0x11,0xE6,0x95,0x98,0xF8,0xE2,0x25,0x1C,0x9A,0x69};
  humidityService = gatt.addService(humidityServiceUUID);
  uint8_t humidityCharacteristicUUID[] = {0x10,0x52,0x5F,0x61,0xCF,0x73,0x11,0xE6,0x95,0x98,0xF8,0xE2,0x25,0x1C,0x9A,0x69};
  humidityCharacteristic = gatt.addCharacteristic(humidityCharacteristicUUID,
                                                  GATT_CHARS_PROPERTIES_READ | GATT_CHARS_PROPERTIES_NOTIFY,
                                                  sizeof(float), sizeof(float), BLE_DATATYPE_BYTEARRAY);

  uint8_t thermometerServiceUUID[] = {0xF4,0x89,0xCB,0x00,0xC1,0x77,0x11,0xE6,0x95,0x98,0x98,0x54,0x24,0x9C,0x9A,0x66};
  thermometerService = gatt.addService(thermometerServiceUUID);
  uint8_t thermometerCharacteristicUUID[] = {0xF4,0x89,0xCB,0x01,0xC1,0x77,0x11,0xE6,0x95,0x98,0x98,0x54,0x24,0x9C,0x9A,0x66};
  temperatureCharacteristic = gatt.addCharacteristic(thermometerCharacteristicUUID,
                                                     GATT_CHARS_PROPERTIES_READ | GATT_CHARS_PROPERTIES_NOTIFY,
                                                     sizeof(float), sizeof(float), BLE_DATATYPE_BYTEARRAY);

  ble.reset();
  ble.setConnectCallback(centralConnect);
  ble.setDisconnectCallback(centralDisconnect);
  Serial.println("Bluetooth on");
}

/** Send randomized heart rate data continuously **/
void loop(void) {
  ble.update();

  static union float_bytes averageHumidity = { .value = sensor.readHumidity() };
  averageHumidity.value += (sensor.readHumidity() - averageHumidity.value) / 30.0;
  static union float_bytes previousHumidity = { .value = 0.0 };
  gatt.getChar(humidityCharacteristic, previousHumidity.bytes, sizeof(previousHumidity));

  if (abs(averageHumidity.value - previousHumidity.value) > 1.5) {
    gatt.setChar(humidityCharacteristic, averageHumidity.bytes, sizeof(averageHumidity));
    Serial.print("Update humidity | ");
    Serial.println(averageHumidity.value, 0);
  } else {
    Serial.print("Humidity | ");
    Serial.println(averageHumidity.value, 0);
  }

  static union float_bytes averageCelsius = { .value = sensor.readTemperature()};
  averageCelsius.value += (sensor.readTemperature() - averageCelsius.value) / 30.0;
  static union float_bytes previousCelsius = { .value = 0.0 };
  gatt.getChar(temperatureCharacteristic, previousCelsius.bytes, sizeof(previousCelsius));

  if (abs(averageCelsius.value - previousCelsius.value) > 0.20) {
    gatt.setChar(temperatureCharacteristic, averageCelsius.bytes, sizeof(averageCelsius));
    Serial.print("Update temperature | ");
    Serial.println(averageCelsius.value, 2);
  } else {
    Serial.print("Temperature | ");
    Serial.println(averageCelsius.value, 2);
  }

  delay(1000);
}


void centralConnect(void) {
  Serial.print("Central connected | ");
  if (ble.sendCommandCheckOK("AT+BLEGETPEERADDR")) {
    Serial.println(ble.buffer);
  }
}

void centralDisconnect(void) {
  Serial.print("Central disconnected | ");
  if (ble.sendCommandCheckOK("AT+BLEGETPEERADDR")) {
    Serial.println(ble.buffer);
  }
}
