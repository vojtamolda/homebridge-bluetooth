#include <limits.h>
#include <CurieBLE.h>


const int pinLED = LED_BUILTIN;

BLEPeripheral ble;
BLEService lightService("8E76F000-690E-472E-88C3-051277686A73");
BLECharCharacteristic stateCharacteristic("8E76F001-690E-472E-88C3-051277686A73", BLERead | BLEWrite);
BLEFloatCharacteristic brightnessCharacteristic("8E76F002-690E-472E-88C3-051277686A73", BLERead | BLEWrite);


void setup() {
  Serial.begin(115200);
  pinMode(pinLED, OUTPUT);

  ble.setLocalName("Arduino");
  ble.setAdvertisedServiceUuid(lightService.uuid());

  ble.addAttribute(lightService);
  ble.addAttribute(stateCharacteristic);
  ble.addAttribute(brightnessCharacteristic);

  ble.setEventHandler(BLEConnected, onCentralConnect);
  ble.setEventHandler(BLEDisconnected, onCentralDisconnect);

  stateCharacteristic.setValue(true);
  brightnessCharacteristic.setValue(100.0);
  setLED(true, 100.0);

  stateCharacteristic.setEventHandler(BLEWritten, onCharacteristicWrite);
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
  float brightness = brightnessCharacteristic.value();
  setLED(state, brightness);
}

void setLED(bool state, float brightness) {
  brightness = max(0.0, min(100.0, brightness));
  Serial.print("LED state: ");
  Serial.print(state);
  Serial.print("|");
  Serial.println(brightness);

  digitalWrite(pinLED, state);
  analogWrite(pinLED, (int) (brightness / 100.0 * INT_MAX));
}

