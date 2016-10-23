var Service, BluetoothCharacteristic;
var Chalk = require('chalk');

module.exports = function (service, bluetoothCharacteristic) {
  Service = service;
  BluetoothCharacteristic = bluetoothCharacteristic;

  return BluetoothService;
};


function BluetoothService(log, config, bluetoothAccessory) {
  this.log = log;
  this.config = config;
  this.name = config.name;
  this.prefix = bluetoothAccessory.prefix + " " + Chalk.magenta("[" + config.name + "]");

  this.nobleService = null;
  if (config.type in Service) {
    this.type = config.type;
    var ServiceType = Service[this.type]; // For example - Service.Lightbulb
    this.homebridgeService = bluetoothAccessory.homebridgeAccessory.addService(ServiceType, this.name);
  } else {
    throw new Error("Type Service." + this.type + " is not available. See 'HAP-NodeJS/lib/gen/HomeKitType.js' for options.")
  }

  if (config.UUID) {
    this.UUID = trimUUID(config.UUID);
  } else {
    throw new Error("Missing bluetooth UUID for Service." + this.type + ".");
  }

  this.bluetoothCharacteristics = [];
  for (var characteristicConfig of config.characteristics) {
    var bluetoothCharacteristic = new BluetoothCharacteristic(log, characteristicConfig, this);
    this.bluetoothCharacteristics.push(bluetoothCharacteristic);
  }
  
}


BluetoothService.prototype.onDiscover = function (nobleService) {
  this.log.info(this.prefix, "Discovered | Service." + this.type + " - " + this.UUID);
  this.nobleService = nobleService;

  var nobleCharacteristicUUIDs = [];
  for (var bluetoothCharacteristic of this.bluetoothCharacteristics) {
    nobleCharacteristicUUIDs.push(bluetoothCharacteristic.UUID);
  }
  this.nobleService.discoverCharacteristics(nobleCharacteristicUUIDs, this.onDiscoverCharacteristics.bind(this));
};


BluetoothService.prototype.onDiscoverCharacteristics = function (error, nobleCharacteristics) {
  if (error) {
    this.log.error(this.prefix, "Discover characteristics failed | " + error);
    return;
  }
  if (nobleCharacteristics.length == 0) {
    this.log.warn(this.prefix, "No characteristics discovered");
    return;
  }

  for (var nobleCharacteristic of nobleCharacteristics) {
    for (var bluetoothCharacteristic of this.bluetoothCharacteristics) {
      if (bluetoothCharacteristic.UUID == nobleCharacteristic.uuid) {
        bluetoothCharacteristic.onDiscover(nobleCharacteristic);
      }
    }
  }
};


BluetoothService.prototype.onDisconnect = function () {
  this.log.info(this.prefix, "Disconnected | " + this.UUID);
  for (var bluetoothCharacteristic of this.bluetoothCharacteristics) {
    bluetoothCharacteristic.onDisconnect();
  }
  this.nobleService = null;
};


function trimUUID(uuid) {
  return uuid.toLowerCase().replace(/:/g, "").replace(/-/g, "");
}
