
# RGB Lightbulb


Turn a bunch of LEDs connected to a BLE capable microprocessor into a wireless HomeKit light. Use the Home app or Siri on your Apple device to switch it on, change color or reduce brightness.

<img src="images/lightbulb-rgb.jpg">

This example uses [Arduino 101](https://www.arduino.cc/en/Main/ArduinoBoard101) and [Raspberry Pi 3](https://www.raspberrypi.org/). Generally, any programmable BLE peripheral and a box capable of running [Node.js](https://nodejs.org) with [Noble](https://github.com/sandeepmistry/noble) will work.


## BLE Peripheral (Arduino 101 or Other BLE Board)

Download and install the latest version of the [Arduino IDE](https://www.arduino.cc/en/Main/Software). If you're totally new to microcontrollers take some time to go through an introductory tutorial and learn how to make a LED blink. This will help you to understand how to use the IDE, how to upload a sketch and what is the code actually doing.

### Wiring
Connect 3 LEDs to pins 9, 6 and 5. These pins support PWM and therefore can be programmed to dim the LEDs. Each LED needs to have a resistor to limit the current passing through - max current per I/O pin is 20 mA. Anything between 100 Ohms and 1k Ohms will do.

The tactile switch is connected to pin 2 and when pushed connects the pin to the ground. The sketch code activates the internal 10k Ohm pull-up resitor to keep the pin high when the switch isn't pressed.

<img src="arduino101/arduino101.png" width="30%">

**Note** _Alternatively, you can use any of the many BLE boards available on the market ([BlueBean](https://punchthrough.com/bean/), [RedBearLabs BLE Nano](http://redbearlab.com/blenano), ...) as long as you keep UUIDs of the services and characteristics in sync with your `config.json` file, everything will work just fine._

### Running the Sketch
Compile, run and upload the [arduino101.ino sketch](arduino101/arduino101.ino) using the [Arduino IDE](https://www.arduino.cc/en/Main/Software).
The sketch creates a BLE service with 4 characteristics. There's one characteristic for on/off characteristic (type `BOOL`), two for hue & saturation (type `FLOAT`) and the fourth one for brightness (type `INT`). All charactersitis have read, write and notify permissions.

```cpp
BLEService lightbulbService("57E54BF0-8574-47BE-9C1D-A0DBFC8FA183");
BLECharCharacteristic onCharacteristic("57E54BF1-8574-47BE-9C1D-A0DBFC8FA183", BLERead | BLEWrite | BLENotify);
BLEFloatCharacteristic hueCharacteristic("57E54BF2-8574-47BE-9C1D-A0DBFC8FA183", BLERead | BLEWrite | BLENotify);
BLEFloatCharacteristic saturationCharacteristic("57E54BF3-8574-47BE-9C1D-A0DBFC8FA183", BLERead | BLEWrite | BLENotify);
BLEIntCharacteristic brightnessCharacteristic("57E54BF4-8574-47BE-9C1D-A0DBFC8FA183", BLERead | BLEWrite | BLENotify);
```

When the tactile switch is toggled the LEDs turn on (or turn off if they were on) and the BLE subscribe-notification mechanism cases the an update update on the Homebridge. This way the information about switching propagates through callbacks to the Apple device without any polling.

Take a look into [this file](https://github.com/KhaosT/HAP-NodeJS/blob/master/lib/gen/HomeKitTypes.js#L1147) to see the full definition of the _Lightbulb_ service.

Once the BLE central device is setup, it connects to this characteristic and exposes it via Homebridge as a HomeKit accessory of type _Lightbulb_. The sketch also contains some logic to convert HSV colors to RGB values for each LED.

Leave the device powered on and the sketch running while you setup the Homebridge server. The sketch has some built-in logging, so keeping the Serial monitor open may be helpful for debugging.


## BLE Central & Homebridge Server (Raspberry Pi 3 or Other Compatible Box)

For help installing an operating system on your new Pi, the official documentation contains a couple of [nice videos](https://www.raspberrypi.org/help/videos/).

### Wiring
No wiring except for the micro-USB cable providing power is needed. The Pi needs to be connected to the same router (subnet) as the Apple device you plan to use. It doesn't matter whether via Wifi or Ethernet. Otherwise, you won't be able discover and connect to the Homebridge server running on the Pi.

<img src="images/raspberry.png" width="30%">

**Note** _Alternatively, you can use a Raspberry Pi 2 with a supported USB BLE dongle instead of the Pi 3._

### Running Homebridge
Running Homebridge on a Raspberry Pi is straightforward. Follow [this guide](https://github.com/nfarina/homebridge/wiki/Running-HomeBridge-on-a-Raspberry-Pi) to install Homebridge server and then run the following command to install the homebridge-bluetooth plugin:

```sh
[sudo] npm install -g homebridge-bluetooth
```

Edit the `~/.homebridge/config.json`, name your Homebridge server and add a new accessory to allow the plugin to connect to the BLE service running on the Arduino:

```js
"name": "Arduino",
"address": "01:23:45:67:89:AB",
"services": [ {
    "name": "RGB LED",
    "type": "Lightbulb",
    "UUID": "57E54BF0-8574-47BE-9C1D-A0DBFC8FA183",
    "characteristics": [ {
            "type": "On",
            "UUID": "57E54BF1-8574-47BE-9C1D-A0DBFC8FA183"
        }, {
            "type": "Brightness",
            "UUID": "57E54BF2-8574-47BE-9C1D-A0DBFC8FA183"
        }, {
            "type": "Saturation",
            "UUID": "857E54BF3-8574-47BE-9C1D-A0DBFC8FA183"
          }, {
            "type": "Hue",
            "UUID": "857E54BF4-8574-47BE-9C1D-A0DBFC8FA183"
      } ]
} ]
```

Finally, start the Homebridge server. If you use Linux you may need to run with higher privileges in order to have access to the BLE hardware layer. See [this link](https://github.com/sandeepmistry/noble#running-without-rootsudo) for more details about running without `sudo`.

```
[sudo] homebridge -D
```

**Note** _Running with `-D` turns on additional debugging output that is very helpful for getting addresses and UUIDs of your BLE devices that needs to match with the `config.json` file._

<img src="images/homebridge.png">

**Note** _Homebridge server doesn't run only on Linux. MacOS and Windows machines are also supported given they have a built-in BLE adapter or an USB dongle. For more details see supported platforms of [Homebridge](https://github.com/nfarina/homebridge) and [Noble](https://github.com/sandeepmistry/noble)._


## Apple Device

### Pairing
Open Home app and tap the '+' button to add new accessory. When you attempt to add the 'Raspberry Pi 3' bridge, it will ask for a "PIN" from the `config.json` file. Once you are paired with your new Rapsberry Homebridge server all the Arduino accesory can be added the same way as the bridge.

### Interacting
Once your BLE accessory has been added to HomeKit database, besides using the Home app or Control Center at the bottom of the screen, you should be able to tell Siri to control any HomeKit accessory. Try _"Hey Siri, dim RGB LED to 50%"_. However, Siri is a cloud service and iOS may need some time to synchronize your HomeKit database to iCloud.

<img src="images/ios-accesories.png" width="30%">
<img src="images/ios-brightness.png" width="30%">
<img src="images/ios-color.png" width="30%">

<img src="images/ios-hsv.png" width="30%">
<img src="images/ios-siri-brightness.png" width="30%">
<img src="images/ios-siri-color.png" width="30%">
