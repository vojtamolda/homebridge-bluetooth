#include <limits.h>
#include <CurieBLE.h>


const int pinRedLED = 13;
const int pinGreenLED = 12;
const int pinBlueLED = 11;
const int pinSwitch = 3;

BLEPeripheral ble;
BLEService lightService("A4C84000-C265-4C20-B06B-EAEB9133B6B0");
BLECharCharacteristic stateCharacteristic("A4C84001-C265-4C20-B06B-EAEB9133B6B04", BLERead | BLEWrite | BLENotify);
BLEFloatCharacteristic hueCharacteristic("A4C84002-C265-4C20-B06B-EAEB9133B6B0", BLERead | BLEWrite);
BLEFloatCharacteristic saturationCharacteristic("A4C84003-C265-4C20-B06B-EAEB9133B6B0", BLERead | BLEWrite);
BLEFloatCharacteristic brightnessCharacteristic("A4C84004-C265-4C20-B06B-EAEB9133B6B0", BLERead | BLEWrite);


void setup() {
  Serial.begin(115200);
  pinMode(pinRedLED, OUTPUT);
  pinMode(pinGreenLED, OUTPUT);
  pinMode(pinBlueLED, OUTPUT);
  pinMode(pinSwitch, INPUT_PULLUP);

  ble.setLocalName("Arduino");
  ble.setAdvertisedServiceUuid(lightService.uuid());

  ble.addAttribute(lightService);
  ble.addAttribute(stateCharacteristic);
  ble.addAttribute(hueCharacteristic);
  ble.addAttribute(saturationCharacteristic);
  ble.addAttribute(brightnessCharacteristic);

  ble.setEventHandler(BLEConnected, onCentralConnect);
  ble.setEventHandler(BLEDisconnected, onCentralDisconnect);

  stateCharacteristic.setValue(true);
  hueCharacteristic.setValue(100.0);
  saturationCharacteristic.setValue(100.0);
  brightnessCharacteristic.setValue(100.0);
  setLED(true, 100.0);

  stateCharacteristic.setEventHandler(BLEWritten, onCharacteristicWrite);
  hueCharacteristic.setEventHandler(BLEWritten, onCharacteristicWrite);
  saturationCharacteristic.setEventHandler(BLEWritten, onCharacteristicWrite);
  brightnessCharacteristic.setEventHandler(BLEWritten, onCharacteristicWrite);
  
  ble.begin();
  Serial.println("Bluetooth on");
}

void loop() {
  ble.poll();
}


void onCentralConnect(BLECentral& central) {
  Serial.print("Central device connected: ");
  Serial.println(central.address());
}

void onCentralDisconnect(BLECentral& central) {
  Serial.print("Central device disconnected: ");
  Serial.println(central.address());
}

void onCharacteristicWrite(BLECentral& central, BLECharacteristic& characteristic) {
  Serial.print("Characteristic written: ");
  Serial.println(characteristic.uuid());

  bool state = (bool) stateCharacteristic.value();
  float hue = hueCharacteristic.value();
  float saturation = hueCharacteristic.value();
  float brightness = brightnessCharacteristic.value();
  setLED(state, hue, saturation, brightness);
}

void setLED(bool state, float hue, float saturation, float brightness) {
  hue = max(0.0, min(360.0, hue));
  saturation = max(0.0, min(100.0, saturation));
  brightness = max(0.0, min(100.0, brightness));

  // HSB to RGB conversion here

  Serial.print("LED state: ");
  Serial.print(state);
  Serial.print("|");
  Serial.println(brightness);

  digitalWrite(pinLED, state);
  digitalWrite(pinLED, state);
  digitalWrite(pinLED, state);
  // RGB analogWrite(pin, value);
}

