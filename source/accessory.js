var Accessory, BluetoothService, UUIDGen;
var Chalk = require('chalk');

module.exports = function (accessory, bluetoothService, uuidGen) {
  Accessory = accessory;
  BluetoothService = bluetoothService;
  UUIDGen = uuidGen;

  return BluetoothAccessory;
};


function BluetoothAccessory(log, config, homebridgeAPI) {
  this.log = log;
  this.config = config;
  this.name = config.name;
  this.address = config.address.toLowerCase();
  this.prefix = Chalk.blue("[" + config.name + "]");

  this.nobleAccessory = null;
  this.homebridgeAccessory = new Accessory(config.name, UUIDGen.generate(config.name));
  this.homebridgeAccessory.updateReachability(false);

  this.bluetoothServices = [];
  for (var serviceConfig of config.services) {
    var bluetoothService = new BluetoothService(log, serviceConfig, this);
    this.bluetoothServices.push(bluetoothService);
  }
}


BluetoothAccessory.prototype.onConnect = function (nobleAccessory) {
  this.log.info(this.prefix, "Connected | " + nobleAccessory.advertisement.localName + " - "+ this.address);

  this.nobleAccessory = nobleAccessory;
  this.homebridgeAccessory.updateReachability(true);

  var nobleServiceUUIDs = [];
  for (var bluetoothService of this.bluetoothServices) {
    nobleServiceUUIDs.push(bluetoothService.UUID);
  }
  this.nobleAccessory.discoverServices(nobleServiceUUIDs, this.onDiscoverServices.bind(this));
};


BluetoothAccessory.prototype.onDiscoverServices = function (error, nobleServices) {
  if (error) {
    this.log.error(this.prefix, "Discover services failed | " + error);
    return;
  }
  if (nobleServices.length == 0) {
    this.log.warn(this.prefix, "No services discovered");
    return;
  }

  for (var nobleService of nobleServices) {
    for (var bluetoothService of this.bluetoothServices) {
      if (bluetoothService.UUID == nobleService.uuid) {
        bluetoothService.onDiscover(nobleService);
      }
    }
  }
};


BluetoothAccessory.prototype.onIdentify = function (paired, callback) {
  this.log.info(this.prefix, "Identify");
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


BluetoothAccessory.prototype.onDisconnect = function (homebridgeAPI) {
  this.log.info(this.prefix, "Disconnected | " + this.address);
  this.homebridgeAccessory.removeAllListeners('identify');
  this.homebridgeAccessory.updateReachability(false);
  
  this.nobleAccessory = null;
};
