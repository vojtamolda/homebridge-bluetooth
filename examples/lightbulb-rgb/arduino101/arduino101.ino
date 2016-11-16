#include <math.h>
#include <Arduino.h>
#include <CurieBLE.h>

const int pinRedLED = 5;
const int pinGreenLED = 6;
const int pinBlueLED = 9;
const int pinSwitch = 2;


BLEPeripheral ble;
BLEService informationService("180A");
BLECharacteristic modelCharacteristic("2A24", BLERead, "101");
BLECharacteristic manufacturerCharacteristic("2A29", BLERead, "Arduino");
BLECharacteristic serialNumberCharacteristic("2A25", BLERead, "2.71828");

BLEService lightbulbService("57E54BF0-8574-47BE-9C1D-A0DBFC8FA183");
BLECharCharacteristic onCharacteristic("57E54BF1-8574-47BE-9C1D-A0DBFC8FA183", BLERead | BLEWrite | BLENotify);
BLEFloatCharacteristic hueCharacteristic("57E54BF2-8574-47BE-9C1D-A0DBFC8FA183", BLERead | BLEWrite | BLENotify);
BLEFloatCharacteristic saturationCharacteristic("57E54BF3-8574-47BE-9C1D-A0DBFC8FA183", BLERead | BLEWrite | BLENotify);
BLEIntCharacteristic brightnessCharacteristic("57E54BF4-8574-47BE-9C1D-A0DBFC8FA183", BLERead | BLEWrite | BLENotify);
bool lastSwitch = HIGH;


void setup() {
  Serial.begin(115200);
  pinMode(pinRedLED, OUTPUT);
  pinMode(pinGreenLED, OUTPUT);  
  pinMode(pinBlueLED, OUTPUT);
  pinMode(pinSwitch, INPUT_PULLUP);

  ble.setLocalName("Light");
  ble.setAdvertisedServiceUuid(lightbulbService.uuid());
  ble.addAttribute(informationService);
  ble.addAttribute(modelCharacteristic);
  ble.addAttribute(manufacturerCharacteristic);
  ble.addAttribute(serialNumberCharacteristic);

  ble.addAttribute(lightbulbService);
  ble.addAttribute(onCharacteristic);
  ble.addAttribute(hueCharacteristic);
  ble.addAttribute(saturationCharacteristic);
  ble.addAttribute(brightnessCharacteristic);

  bool on = true;
  float hue = 0.0;
  float saturation = 0.0;
  int brightness = 100;
  onCharacteristic.setValueLE(on);
  hueCharacteristic.setValueLE(hue);
  saturationCharacteristic.setValueLE(saturation);
  brightnessCharacteristic.setValueLE(brightness);
  setLED(on, hue, saturation, brightness);

  ble.setEventHandler(BLEConnected, centralConnect);
  ble.setEventHandler(BLEDisconnected, centralDisconnect);
  onCharacteristic.setEventHandler(BLEWritten, characteristicWrite);
  hueCharacteristic.setEventHandler(BLEWritten, characteristicWrite);
  saturationCharacteristic.setEventHandler(BLEWritten, characteristicWrite);
  brightnessCharacteristic.setEventHandler(BLEWritten, characteristicWrite);

  ble.begin();
  Serial.println("Bluetooth on");
  lastSwitch = digitalRead(pinSwitch);
}

void loop() {
  ble.poll();

  if ((digitalRead(pinSwitch) == LOW) && (lastSwitch == HIGH)) {
    bool on = (bool) onCharacteristic.valueLE();
    float hue = (float) hueCharacteristic.valueLE();
    float saturation = (float) saturationCharacteristic.valueLE();
    int brightness = (int) brightnessCharacteristic.valueLE();
    setLED(!on, hue, saturation, brightness);
    onCharacteristic.setValue(!on);
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
  float hue = (float) hueCharacteristic.valueLE();
  float saturation = (float) saturationCharacteristic.valueLE();
  int brightness = (int) brightnessCharacteristic.valueLE();
  setLED(on, hue, saturation, brightness);
}

void setLED(bool on, float hue, float saturation, int brightness) {
  hue = max(0.0, min(360.0, hue));
  saturation = max(0.0, min(100.0, saturation));
  brightness = max(0, min(100, brightness));

  unsigned int red, green, blue;
  hsv2rgb(hue, saturation/100.0, brightness/100.0, &red, &green, &blue);
  analogWrite(pinRedLED, on * red);
  analogWrite(pinGreenLED, on * green);
  analogWrite(pinBlueLED, on * blue);
  
  Serial.print("RGB LED | ");
  Serial.print(on);
  Serial.print(" HSB(");
  Serial.print(hue);
  Serial.print(",");
  Serial.print(saturation);
  Serial.print(",");
  Serial.print(brightness);
  Serial.print(") -> RGB(");
  Serial.print(red);
  Serial.print(",");
  Serial.print(green);
  Serial.print(",");
  Serial.print(blue);
  Serial.println(")");
}

void hsv2rgb(float hue, float saturation, float value,
             unsigned int* red, unsigned int* green, unsigned int* blue) {
  if (saturation == 0.0) {
    *red   = (unsigned int) round(value * 255.0);
    *green = (unsigned int) round(value * 255.0);
    *blue  = (unsigned int) round(value * 255.0);
  } else {
    int h = ((int) floor(hue / 60.0) % 6);
    float f = (hue / 60.0) - floor(hue / 60.0);
    float p = value * (1.0 - saturation);
    float q = value * (1.0 - saturation * f);
    float t = value * (1.0 - (saturation * (1.0 - f)));
    switch (h) {
      case 0:
        *red   = (unsigned int) round(value * 255.0);
        *green = (unsigned int) round(t * 255.0);
        *blue  = (unsigned int) round(p * 255.0);
        break;
      case 1:
        *red   = (unsigned int) round(q * 255.0);
        *green = (unsigned int) round(value * 255.0);
        *blue  = (unsigned int) round(p * 255.0);
        break;
      case 2:
        *red   = (unsigned int) round(p * 255.0);
        *green = (unsigned int) round(value * 255.0);
        *blue  = (unsigned int) round(t * 255.0);
        break;
      case 3:
        *red   = (unsigned int) round(p * 255.0);
        *green = (unsigned int) round(q * 255.0);
        *blue  = (unsigned int) round(value * 255.0);
        break;
      case 4:
        *red   = (unsigned int) round(t * 255.0);
        *green = (unsigned int) round(p * 255.0);
        *blue  = (unsigned int) round(value * 255.0);
        break;
      case 5:
        *red   = (unsigned int) round(value * 255.0);
        *green = (unsigned int) round(p * 255.0);
        *blue  = (unsigned int) round(q * 255.0);
        break;
    }
  }
}
