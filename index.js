var Noble, Accessory, Service, Characteristic, UUIDGen;

module.exports = function (homebridge) {
  console.log("Homebridge API version: " + homebridge.version);

  Noble = require('noble');
  Accessory = homebridge.platformAccessory;
  Service = homebridge.hap.Service;
  Characteristic = homebridge.hap.Characteristic;
  UUIDGen = homebridge.hap.uuid;

  BluetoothCharacteristic = require("./source/characteristic.js")(Characteristic);
  BluetoothService = require("./source/service.js")(Service, BluetoothCharacteristic);
  BluetoothAccessory = require("./source/accessory.js")(Accessory, BluetoothService, UUIDGen);
  BluetoothPlatform = require("./source/platform.js")(Noble, BluetoothAccessory);

  homebridge.registerPlatform("homebridge-bluetooth", "Bluetooth", BluetoothPlatform, true);
};
