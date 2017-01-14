var Accessory, BluetoothService;
var Chalk = require('chalk');

module.exports = function (accessory, bluetoothService) {
  Accessory = accessory;
  BluetoothService = bluetoothService;

  return BluetoothAccessory;
};


function BluetoothAccessory(log, config) {
  this.log = log;

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

  this.log.debug(this.prefix, "Initialized | " + this.name + " (" + this.address + ")");
  this.bluetoothServices = {};
  for (var serviceConfig of config.services) {
    var serviceUUID = trimUUID(serviceConfig.UUID);
    this.bluetoothServices[serviceUUID] = new BluetoothService(this.log, serviceConfig,
                                                               this.prefix);
  }

  var informationServiceUUID = trimUUID('180A')
  if (!(informationServiceUUID in this.bluetoothServices)) {
    informationServiceConfig = {
      "name": "Information",
      "type": "AccessoryInformation",
      "UUID": "180A",
      "characteristics": [
        {"type": "Manufacturer", "UUID": "2A29"},
        {"type": "Model", "UUID": "2A24"},
        {"type": "SerialNumber", "UUID": "2A25"}
      ]
    };
    this.bluetoothServices[informationServiceUUID]
        = new BluetoothService(this.log, informationServiceConfig, this.prefix);
  }

  this.homebridgeAccessory = null;
  this.nobleAccessory = null;
}


BluetoothAccessory.prototype.connect = function (nobleAccessory, homebridgeAccessory) {
  this.log.info(this.prefix, "Connected | " + this.name + " (" + this.address + ")");
  this.homebridgeAccessory = homebridgeAccessory;
  this.homebridgeAccessory.on('identify', this.identification.bind(this));
  this.homebridgeAccessory.updateReachability(true);

  this.nobleAccessory = nobleAccessory;
  this.nobleAccessory.once('disconnect', this.disconnect.bind(this));
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
    var bluetoothService = this.bluetoothServices[serviceUUID];
    if (!bluetoothService) {
      if (nobleService.uuid != '1800' && nobleService.uuid != '1801') {
        this.log.debug(this.prefix, "Ignored | Service (" + nobleService.uuid + ")");
      }
      continue;
    }

    var homebridgeService = this.homebridgeAccessory.getService(bluetoothService.class);
    if (!homebridgeService) {
      homebridgeService = this.homebridgeAccessory.addService(bluetoothService.class,
                                                              bluetoothService.name);
    }
    bluetoothService.connect(nobleService, homebridgeService);
  }
};


BluetoothAccessory.prototype.identification = function (paired, callback) {
  this.log.info(this.prefix, "Identify");
  callback();
};


BluetoothAccessory.prototype.disconnect = function (error) {
  if (error) {
    this.log.error("Disconnecting failed | " + this.name + " (" + this.address + ") | " + error);
  }

  for (var serviceUUID in this.bluetoothServices) {
    this.bluetoothServices[serviceUUID].disconnect();
  }
  if (this.nobleAccessory && this.homebridgeAccessory) {
    this.homebridgeAccessory.removeAllListeners('identify');
    this.homebridgeAccessory.updateReachability(false);
    this.homebridgeAccessory = null;
    this.nobleAccessory.removeAllListeners();
    this.nobleAccessory = null;
    this.log.info(this.prefix, "Disconnected");
  }
};


function trimUUID(uuid) {
  return uuid.toLowerCase().replace(/:/g, "").replace(/-/g, "");
}
