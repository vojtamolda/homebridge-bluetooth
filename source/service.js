var Service, BluetoothCharacteristic;
var Chalk = require('chalk');

module.exports = function (service, bluetoothCharacteristic) {
  Service = service;
  BluetoothCharacteristic = bluetoothCharacteristic;

  return BluetoothService;
};


function BluetoothService(log, config, nobleService, parentBluetoothAccessory) {
  this.log = log;

  this.init(config, parentBluetoothAccessory);
  this.setup(nobleService, parentBluetoothAccessory);
}


BluetoothService.prototype.init = function (config, parentBluetoothAccessory) {
  if (!config.name) {
    throw new Error("Missing mandatory config 'name'");
  }
  this.name = config.name;
  this.prefix = parentBluetoothAccessory.prefix + " " + Chalk.magenta("[" + this.name + "]");

  if (!config.type) {
    throw new Error(this.prefix + " Missing mandatory config 'type'");
  }
  this.type = config.type;

  if (!config.UUID) {
    throw new Error(this.prefix + " Missing mandatory config 'UUID'");
  }
  this.UUID = config.UUID;

  if (!config.characteristics || !(config.characteristics instanceof Array)) {
    throw new Error(this.prefix + " Missing mandatory config 'characteristics'");
  }
  this.characteristicConfigs = {};
  this.bluetoothCharacteristics = {};
  for (var characteristicConfig of config.characteristics) {
    var characteristicUUID = trimUUID(characteristicConfig.UUID);
    this.characteristicConfigs[characteristicUUID] = characteristicConfig;
  }

  this.log.info(this.prefix, "Initialized | Service." + this.type + " - " + this.UUID);
};


BluetoothService.prototype.setup = function (nobleService, parentBluetoothAccessory) {
  var serviceType = Service[this.type]; // For example - Service.Lightbulb
  if (!serviceType) {
    throw new Error(this.prefix + " Service type '" + this.type + "' is not defined. See 'HAP-NodeJS/lib/gen/HomeKitType.js' for options.")
  }
  this.homebridgeService = parentBluetoothAccessory.homebridgeAccessory.addService(serviceType, this.name);

  this.nobleService = nobleService;
  this.nobleService.discoverCharacteristics([], this.discoverCharacteristics.bind(this));
};


BluetoothService.prototype.discoverCharacteristics = function (error, nobleCharacteristics) {
  if (error) {
    this.log.error(this.prefix, "Discover characteristics failed | " + error);
    return;
  }
  if (nobleCharacteristics.length == 0) {
    this.log.warn(this.prefix, "No characteristics discovered");
    return;
  }

  for (var nobleCharacteristic of nobleCharacteristics) {
    var characteristicUUID = trimUUID(nobleCharacteristic.uuid);
    var characteristicConfig = this.characteristicConfigs[characteristicUUID];
    if (!characteristicConfig) {
      this.log.info(this.prefix, "Ignored | Characteristic - " + nobleCharacteristic.uuid);
      continue;
    }
    this.bluetoothCharacteristics[characteristicUUID] = new BluetoothCharacteristic(this.log, characteristicConfig, nobleCharacteristic, this);
  }
};


BluetoothService.prototype.disconnect = function () {
  for (var characteristicUUID in this.bluetoothCharacteristics) {
    this.bluetoothCharacteristics[characteristicUUID].disconnect();
  }

  this.nobleService = null;
  this.homebridgeService = null;
  this.log.info(this.prefix, "Disconnected");
};


function trimUUID(uuid) {
  return uuid.toLowerCase().replace(/:/g, "").replace(/-/g, "");
}
