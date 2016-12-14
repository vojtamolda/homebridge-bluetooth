#include <SPI.h>
#include <Arduino.h>
#include <Adafruit_BLE.h>
#include <Adafruit_BLEGatt.h>
#include <Adafruit_BluefruitLE_SPI.h>


Adafruit_BluefruitLE_SPI ble(8, 7, 4); // Firmware > 0.7.0
Adafruit_BLEGatt gatt(ble);
int32_t informationService;
int32_t modelCharacteristic;
int32_t manufacturerCharacteristic;
int32_t serialNumberCharacteristic;

int32_t thermometerService;
int32_t temperatureCharacteristic;


union float_bytes {
  float value;
  uint8_t bytes[sizeof(float)];
};


void setup(void) {
  Serial.begin(115200);

  ble.begin(/* true - useful for debugging */);
  ble.factoryReset();
  ble.info();

  informationService = gatt.addService(0x180A);
  char model[] = "Micro LE";
  modelCharacteristic = gatt.addCharacteristic(0x2A24, GATT_CHARS_PROPERTIES_READ,
                                               sizeof(model)+1, sizeof(model)+1, BLE_DATATYPE_STRING);
  gatt.setChar(modelCharacteristic, model);
  char manufacturer[] = "Bluefruit";
  manufacturerCharacteristic = gatt.addCharacteristic(0x2A29, GATT_CHARS_PROPERTIES_READ,
                                               sizeof(manufacturer), sizeof(manufacturer), BLE_DATATYPE_STRING);
  gatt.setChar(manufacturerCharacteristic, manufacturer);
  char serialNumber[] = "2.71828";
  serialNumberCharacteristic = gatt.addCharacteristic(0x2A25, GATT_CHARS_PROPERTIES_READ,
                                               sizeof(serialNumber), sizeof(serialNumber), BLE_DATATYPE_STRING);
  gatt.setChar(serialNumberCharacteristic, serialNumber);

  uint8_t thermometerServiceUUID[] = {0x1D,0x8A,0x68,0xE0,0xE6,0x8E,0x4F,0xED,0x94,0x3E,0x36,0x90,0x99,0xF5,0xB4,0x99};
  thermometerService = gatt.addService(thermometerServiceUUID);
  uint8_t thermometerCharacteristicUUID[] = {0x1D,0x8A,0x68,0xE1,0xE6,0x8E,0x4F,0xED,0x94,0x3E,0x36,0x90,0x99,0xF5,0xB4,0x99};
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

  static union float_bytes averageCelsius = { .value = 0.0 };
  if (ble.sendCommandCheckOK("AT+HWGETDIETEMP")) {
    float currentCelsius = atof(ble.buffer);
    averageCelsius.value += (currentCelsius - averageCelsius.value) / 30.0;
  }
  union float_bytes previousCelsius = { .value = 0.0 };
  gatt.getChar(temperatureCharacteristic, previousCelsius.bytes, sizeof(previousCelsius));

  if (abs(averageCelsius.value - previousCelsius.value) > 0.50) {
    gatt.setChar(temperatureCharacteristic, averageCelsius.bytes, sizeof(averageCelsius));
    Serial.print("Update temperature | ");
    Serial.println(averageCelsius.value);
  } else {
    Serial.print("Temperature | ");
    Serial.println(averageCelsius.value);
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
