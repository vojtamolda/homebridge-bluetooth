
# homebridge-bluetooth

[Homebridge](https://github.com/nfarina/homebridge) plugin for exposing services and
characteristics of nearby [BLE](https://www.bluetooth.com/what-is-bluetooth-technology/bluetooth-technology-basics/low-energy) peripherals as [HomeKit](https://www.apple.com/ios/home/) accesories. Ideal for DIY home automation projects if you'd like to control them comfortably with Siri on any Apple device.

<img src="images/overview.jpg">

Homebridge implements the HomeKit protocol and provides the API between your Apple device and your home automation server. This [plugin](https://www.npmjs.com/package/homebridge-bluetooth) relays the communication from the home automation server to the BLE peripheral.



## Installation


### Prerequisites

#### macOS (10.10 or newer)
Check [this link](http://www.imore.com/how-tell-if-your-mac-has-bluetooth-40) to see if your mac has built-in Bluetooth 4.0 support. All macs newer that 2012 are generally fine.
- Install [XCode](https://itunes.apple.com/ca/app/xcode/id497799835?mt=12)
- Install [node.js](https://nodejs.org/en/download/)

#### Linux (Debian Based, Kernel 3.6 or newer)
A supported Bluetooth 4.0 USB adapter is required, if your device doesn't have it built-in.
 - Install [node.js](https://nodejs.org/en/download/)

   [node.js](https://nodejs.org) is being developed so quickly that `apt-get` repositories of most distributions contain a very old version. Getting latest from the official website is recommended.)

 - Install `libbluetooth-dev` and `libavahi-compat-libdnssd-dev`

   ```bash
   sudo apt-get install bluetooth bluez libbluetooth-dev libudev-dev libavahi-compat-libdnssd-dev
   ```

#### Windows (8.1 or newer)
Pull request is welcomed here... Both [homebridge](https://github.com/nfarina/homebridge) and [noble](https://github.com/sandeepmistry/noble) should run on windows, but I don't have a machine to test.


### Install Homebridge & Noble

[Homebridge](https://github.com/nfarina/homebridge) is a lightweight [node.js](https://nodejs.org/) server that provides the HomeKit bridge for your Apple devices to connect to. [noble](https://github.com/sandeepmistry/noble) is BLE central module library for [node.js](https://nodejs.org/) that abstracts away intricacies of each OS BLE stack implementation and provides a nice universal API.

Depending on your privileges `-g` flag may need root permissions to install to the global `npm` module directory.
```bash
[sudo] npm install -g noble
[sudo] npm install -g --unsafe-perm homebridge node-gyp
[sudo] npm install -g homebridge-bluetooth
```


### Configure Homebridge
TO-DO This will is likely to change:
```json
"accessories": [
  {
    "accessory": "BLE Accessory",
    "name": "Arduino 101",
    "address": "01:23:45:67:89:ab",
    "led_service": "19B10010-E8F2-537E-4F6C-D104768A1214",
    "led_characteristic": "19B10011-E8F2-537E-4F6C-D104768A1214",
  }
]
```
The easiest way to find the address of a BLE peripheral is to start the plugin without a random/default address and then check the logs for `Ignored (ADR|01:23:45:67:89:ab, TXP|-47dB, UUUDs|...)` Alternatively you can run `[sudo] hcitool lescan`


### Run Homebridge

Depending on your privileges, communicating with the BLE device may need root permissions.

```bash
[sudo] homebridge
```

 See [this section of noble readme](https://github.com/sandeepmistry/noble#running-without-rootsudo) for more details about running without `sudo`.



## FAQ


### Can I contribute my own example?
Sure thing! All contributions are welcome. Just do a pull-request or open a new issue.


### On what devices was this library tested?
- Apple Device
  - [iPhone](https://en.wikipedia.org/wiki/IPhone) 5S & 6 running [iOS](https://en.wikipedia.org/wiki/IOS) 10

- Homebridge Server
  - [Raspberry Pi](https://en.wikipedia.org/wiki/Raspberry_Pi) 3 & 2 (with USB dongle) running [Raspbian](https://www.raspberrypi.org/downloads/raspbian/)  Jessie Lite
  - [Macbook Air](https://en.wikipedia.org/wiki/MacBook_Air) (2012), [iMac](https://en.wikipedia.org/wiki/IMac) (2012) running [macOS](https://en.wikipedia.org/wiki/MacOS) 10.12

- Bluetooth Peripheral
  - nRF51 Based Boards
    - [Arduino 101](https://www.arduino.cc/en/Main/ArduinoBoard101), [RFDuino](http://www.rfduino.com/), [Bluefruit Micro](https://www.adafruit.com/product/2661)
  - nRF8001 Based Boards
    - [Arduino UNO](https://www.arduino.cc/en/Main/ArduinoBoardUno) + [nRF8001 Breakout](https://www.adafruit.com/products/1697),


### Is this thing secure?
No. Not even close. Connection from your Apple device to Homebridge server is encrypted. However, currently all BLE communication is unencrypted and anyone with the right spoofing equipment can listen to it. Moreover, once the attacker has figured out what devices you have, he can also connect to any of your peripherals and control them directly. So don't use this if you're paranoid or if NSA is after you. You wouldn't sleep well.

[MFi](https://developer.apple.com/programs/mfi/) certified HomeKit BLE encryption uses quite-strong Ed25519 elliptic cypher. BLE has built-in support for encryption, but as usual it seems that Apple has decided to build their own thing. It makes some sense in this case since a lot is at stake - once HomeKit becomes widespread a security bug literally means open doors to your house.

Moreover, from bits and pieces available on the Internet, it seems that Apple has changed the specs several times and caused a lot of trouble for the manufacturers, since slower microprocessors tend to have a hard time doing all the involved encryption math. Initial pairing can take literally minutes. Pairing procedure uses SRP (Secure Remote Password (3072-bit) protocol with an 8-digit code (the number you have to type in when first pairing with Homebridge). After pairing, per session communication always uses unique keys derived by HKDF-SHA-512 and encrypted by the ChaCha20-Poly1305.

Theoretically, one should be able to get rid of the Homebridge 'middle-man' since HomeKit over BLE allows direct connection to any Apple device. While there are many good [HomeKit IP stacks around](https://github.com/KhaosT/HAP-NodeJS) BLE implementations are few and far between. There's only one implementation I'm aware of [here](https://github.com/aanon4/HomeKit), but it can't be easily ported to other boards.

BTW, I think it might be possible to re-write the BLE implementation above with [BLEPeripheral](https://github.com/sandeepmistry/arduino-BLEPeripheral) library instead of Nordic Semi's SoftDevice to make it run essentially everywhere. If anyone is interested in this project, please, let me know and maybe we can figure a plan and come up with something useful and fun to use.

Implementing a HomeKit over BLE stack correctly requires access to [MFi] internal documentation, which isn't publicly available, unless you're a registered develper at a company with big $$bucks$$. Making BLE accessories is simple, but making them secure is very hard.



## License
This work is licensed under the MIT license. See [license](license.txt) for more details.
