var Service, BluetoothCharacteristic;
var Chalk = require('chalk');

module.exports = function (service, bluetoothCharacteristic) {
  Service = service;
  BluetoothCharacteristic = bluetoothCharacteristic;

  return BluetoothService;
};


function BluetoothService(log, config, prefix) {
  this.log = log;

  if (!config.name) {
    throw new Error("Missing mandatory config 'name'");
  }
  this.name = config.name;
  this.prefix = prefix + " " + Chalk.magenta("[" + this.name + "]");

  if (!config.type) {
    throw new Error(this.prefix + " Missing mandatory config 'type'");
  }
  this.type = config.type;
  if (!Service[this.type]) {
    throw new Error(this.prefix + " Service type '" + this.type + "' is not defined. " +
                    "See 'HAP-NodeJS/lib/gen/HomeKitType.js' for options.")
  }
  this.class = Service[this.type]; // For example - Service.Lightbulb

  if (!config.UUID) {
    throw new Error(this.prefix + " Missing mandatory config 'UUID'");
  }
  this.UUID = config.UUID;

  if (!config.characteristics || !(config.characteristics instanceof Array)) {
    throw new Error(this.prefix + " Missing mandatory config 'characteristics'");
  }

  this.log.info(this.prefix, "Initialized | Service." + this.type + " - " + this.UUID);
  this.bluetoothCharacteristics = {};
  for (var characteristicConfig of config.characteristics) {
    var characteristicUUID = trimUUID(characteristicConfig.UUID);
    this.bluetoothCharacteristics[characteristicUUID] =
        new BluetoothCharacteristic(this.log, characteristicConfig, this.prefix);
  }

  this.homebridgeService = null;
  this.nobleService = null;
}


BluetoothService.prototype.connect = function (nobleService, homebridgeService) {
  this.log.info(this.prefix, "Connected | Service." + this.type + " - " + this.UUID);
  this.homebridgeService = homebridgeService;

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
    var bluetoothCharacteristic = this.bluetoothCharacteristics[characteristicUUID];
    if (!bluetoothCharacteristic) {
      this.log.info(this.prefix, "Ignored | Characteristic - " + nobleCharacteristic.uuid);
      continue;
    }

    var homebridgeCharacteristic =
        this.homebridgeService.getCharacteristic(bluetoothCharacteristic.class);
    bluetoothCharacteristic.connect(nobleCharacteristic, homebridgeCharacteristic);
  }
};


BluetoothService.prototype.disconnect = function () {
  for (var characteristicUUID in this.bluetoothCharacteristics) {
    this.bluetoothCharacteristics[characteristicUUID].disconnect();
  }
  if (this.nobleCharacteristic && this.homebridgeCharacteristic) {
    this.homebridgeService = null;
    this.nobleService = null;
    this.log.info(this.prefix, "Disconnected");
  }
};


function trimUUID(uuid) {
  return uuid.toLowerCase().replace(/:/g, "").replace(/-/g, "");
}
