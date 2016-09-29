var Service, Characteristic;
var Bluetooth = require('noble');


module.exports = function(homebridge) {
  Service = homebridge.hap.Service;
  Characteristic = homebridge.hap.Characteristic;

  homebridge.registerAccessory('homebridge-bluetooth',
      'BLE Accessory', BLEAccessory);
};

function BLEAccessory(log, config) {
  this.log = log;
  this.name = config.name;
  this.address = config.address.toLowerCase();
  this.lights = config.lights;
  this.lights.service = trim_uuid(config.lights.service);
  this.lights.characteristic = trim_uuid(config.lights.characteristic);
  this.switches = config.switches;
  this.switches.service = trim_uuid(config.switches.service);
  this.switches.characteristic = trim_uuid(config.switches.characteristic);

  this.lightService = new Service.Lightbulb(this.name + '-' + this.lights.name)
  this.lightService.getCharacteristic(Characteristic.On).on('get', this.getLight.bind(this));
  this.lightService.getCharacteristic(Characteristic.On).on('set', this.setLight.bind(this));

  this.switchService = new Service.Switch(this.name + '-' + this.switches.name);
  this.switchService.getCharacteristic(Characteristic.On).on('get', this.getSwitch.bind(this));
  this.switchService.getCharacteristic(Characteristic.On).on('set', this.setSwitch.bind(this));

  Bluetooth.on('stateChange', this.onStateChange.bind(this));
};

BLEAccessory.prototype.getServices = function() {
  return [this.lightService, this.switchService];
};

BLEAccessory.prototype.getLight = function(callback) {
  this.log("Getting LED state ");
  callback(null, this.lightsChracteristic.read());
};

BLEAccessory.prototype.setLight = function(on, callback) {
  this.log("Setting LED to " + on);
  if (on) {
    this.lightsChracteristic.write(new Buffer([0x01]), true);
  } else {
    this.lightsChracteristic.write(new Buffer([0x00]), true);
  }
  callback();
};

BLEAccessory.prototype.getSwitch = function(callback) {
  this.log("Get Switch state");
  callback(null, true);
};

BLEAccessory.prototype.setSwitch = function(on, callback) {
  this.log("Setting Switch to " + on);
  callback();
};

BLEAccessory.prototype.onStateChange = function(state) {
  if (state == 'poweredOn') {
    this.log('Start scanning');
    Bluetooth.startScanning([], false);
    Bluetooth.on('discover', this.onDiscoverPeripheral.bind(this));
  } else {
    this.log('Stop scanning');
    Bluetooth.stopScanning();
  }
};

BLEAccessory.prototype.onDiscoverPeripheral = function(peripheral) {
  var address = peripheral.address;
  var localName = peripheral.advertisement.localName;
  var serviceUuids = peripheral.advertisement.serviceUuids;
  if (address != this.address) {
    this.log('Ignoring: ' + localName + ' (ADR|' + address + ', UUIDs|' + serviceUuids + ')');
    return
  } else {
    this.log('Connecting: ' + localName + ' (ADR|' + address + ', UUIDs|' + serviceUuids + ')');
  }

  this.peripheral = peripheral;
  peripheral.once('disconnect', this.onDisconnect.bind(this));
  peripheral.connect(this.onConnect.bind(this));
  Bluetooth.stopScanning();
};

BLEAccessory.prototype.onConnect = function(error) {
  if (error) {
    this.log('Connect failed: ' + error);
    return
  } else {
    this.log('Connected');
  }

  this.peripheral.discoverSomeServicesAndCharacteristics(
      [this.lights.service, this.switches.service],
      [this.lights.characteristic, this.switches.characteristic],
      this.onDiscoverServicesAndCharacteristics.bind(this));
};

BLEAccessory.prototype.onDiscoverServicesAndCharacteristics = function(error, services, characteristics) {
  this.log('Discover services & characteristics');

  for (characteristic of characteristics) {
    if (this.lights.characteristic == characteristic.uuid) {
      this.log('Discovered characteristic: ' + characteristic.uuid);
      this.lightsChracteristic = characteristic;
      //this.lightsChracteristic.on('data', this.dataLights.bind(this))
    }
  }
};

BLEAccessory.prototype.identify = function(callback) {
  this.log('Identify');
/*  if (this.peripheral) {
    this.alertCharacteristic.write(new Buffer([0x02]), true);
    setTimeout(function() {
      this.alertCharacteristic.write(new Buffer([0x00]), true);
    }.bind(this), 250);
    callback();
  } else {
    callback(new Error('not connected'));
  } */
};


BLEAccessory.prototype.onDisconnect = function(error) {
  this.log('Disconnected');
  this.peripheral = null;
};


function trim_uuid(uuid) {
  return uuid.toLowerCase().replace(/:/g, '').replace(/-/g, '');
}
