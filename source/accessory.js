var Accessory, BluetoothService, UUIDGen;
var Chalk = require('chalk');

module.exports = function (accessory, bluetoothService, uuidGen) {
  Accessory = accessory;
  BluetoothService = bluetoothService;
  UUIDGen = uuidGen;

  return BluetoothAccessory;
};


function BluetoothAccessory(log, config, nobleAccessory, parentBluetoothPlatform) {
  this.log = log;

  this.init(config, parentBluetoothPlatform);
  this.setup(nobleAccessory, parentBluetoothPlatform);
}


BluetoothAccessory.prototype.init = function (config, parentBluetoothPlatform) {
  if (!config.name) {
    throw new Error("Missing mandatory config 'name'");
  }
  this.name = config.name;
  this.prefix = Chalk.blue("[" + config.name + "]");

  if (!config.address) {
    throw new Error(this.prefix + " Missing mandatory config 'address'");
  }
  this.address = config.address;

  if (!config.services || !(config.services instanceof Array)) {
    throw new Error(this.prefix + " Missing mandatory config 'services'");
  }
  this.serviceConfigs = {};
  this.bluetoothServices = {};
  for (var serviceConfig of config.services) {
    var serviceUUID = trimUUID(serviceConfig.UUID);
    this.serviceConfigs[serviceUUID] = serviceConfig;
  }

  this.log.info(this.prefix, "Connected | " + this.name + " - " + this.address);
};


BluetoothAccessory.prototype.setup = function (nobleAccessory, parentBluetoothPlatform) {
  this.homebridgeAccessory = new Accessory(this.name, UUIDGen.generate(this.name));
  this.homebridgeAccessory.updateReachability(true);
  this.bluetoothPlatform = parentBluetoothPlatform;
  this.bluetoothPlatform.homebridgeAPI.registerPlatformAccessories("homebridge-bluetooth", "Bluetooth", [this.homebridgeAccessory]);

  this.nobleAccessory = nobleAccessory;
  this.nobleAccessory.discoverServices([], this.discoverServices.bind(this));
};


BluetoothAccessory.prototype.discoverServices = function (error, nobleServices) {
  if (error) {
    this.log.error(this.prefix, "Discover services failed | " + error);
    return;
  }
  if (nobleServices.length == 0) {
    this.log.warn(this.prefix, "No services discovered");
    return;
  }

  for (var nobleService of nobleServices) {
    var serviceUUID = trimUUID(nobleService.uuid);
    var serviceConfig = this.serviceConfigs[serviceUUID];
    if (!serviceConfig) {
      this.log.info(this.prefix, "Ignored | Service - " + nobleService.uuid);
      continue;
    }
    this.bluetoothServices[serviceUUID] = new BluetoothService(this.log, serviceConfig, nobleService, this);
  }
};


BluetoothAccessory.prototype.identify = function (paired, callback) {
  this.log.info(this.prefix, "Identify | " + paired);
  /*  if (this.nobleAccessory) {
   this.alertCharacteristic.write(new Buffer([0x02]), true);
   setTimeout(function() {
   this.alertCharacteristic.write(new Buffer([0x00]), true);
   }.bind(this), 250);
   callback();
   } else {
   callback(new Error("not connected"));
   } */
  callback();
};


BluetoothAccessory.prototype.disconnect = function () {
  for (var serviceUUID in this.bluetoothServices) {
    this.bluetoothServices[serviceUUID].disconnect();
  }
  this.homebridgeAccessory.removeAllListeners('identify');
  this.homebridgeAccessory.updateReachability(false);
  this.bluetoothPlatform.homebridgeAPI.unregisterPlatformAccessories("homebridge-bluetooth", "Bluetooth", [this.homebridgeAccessory]);

  this.nobleAccessory = null;
  this.homebridgeAccessory = null;
  this.log.info(this.prefix, "Disconnected");
};


function trimUUID(uuid) {
  return uuid.toLowerCase().replace(/:/g, "").replace(/-/g, "");
}
