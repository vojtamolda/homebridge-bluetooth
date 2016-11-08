var Noble, BluetoothAccessory;

module.exports = function (noble, bluetoothAccessory) {
  Noble = noble;
  BluetoothAccessory = bluetoothAccessory;

  return BluetoothPlatform;
};


function BluetoothPlatform(log, config, homebridgeAPI) {
  this.log = log;

  this.init(config);
  this.homebridgeAPI = homebridgeAPI;
  this.homebridgeAPI.on('didFinishLaunching', this.didFinishLaunching.bind(this));
}


BluetoothPlatform.prototype.init = function (config) {
  if (!config.accessories || !(config.accessories instanceof Array)) {
    throw new Error("Missing mandatory config 'accessories'");
  }
  this.accessoryConfigs = {};
  this.bluetoothAccessories = {};
  for (var accessoryConfig of config.accessories) {
    var accessoryAddress = trimAddress(accessoryConfig.address);
    this.accessoryConfigs[accessoryAddress] = accessoryConfig;
  }
};


BluetoothPlatform.prototype.didFinishLaunching = function () {
  Noble.on('stateChange', this.stateChange.bind(this));
};


BluetoothPlatform.prototype.stateChange = function (state) {
  if (state != 'poweredOn') {
    this.log.info("Stopped | " + state);
    Noble.stopScanning();
  }

  this.log.info("Started | " + state);
  Noble.startScanning([], false);
  Noble.on('discover', this.discover.bind(this));
};


BluetoothPlatform.prototype.configureAccessory = function (bluetoothAccessory) {
  this.log.info("Configure cached accessory | ");
  this.log.info(bluetoothAccessory);
};


BluetoothPlatform.prototype.discover = function (nobleAccessory) {
  var accessoryAddress = trimAddress(nobleAccessory.address);
  var accessoryConfig = this.accessoryConfigs[accessoryAddress];
  if (!accessoryConfig) {
    this.log.info("Ignored | " + nobleAccessory.advertisement.localName + " - " + nobleAccessory.address);
    return;
  }

  this.log.info("Discovered | " + nobleAccessory.advertisement.localName + " - " + nobleAccessory.address);
  nobleAccessory.connect(function (error) {
    this.connect(error, nobleAccessory)
  }.bind(this));
};


BluetoothPlatform.prototype.connect = function (error, nobleAccessory) {
  if (error) {
    this.log.error("Connecting failed | " + nobleAccessory.advertisement.localName + " - " + nobleAccessory.address + " | " + error);
    return;
  }

  var accessoryAddress = trimAddress(nobleAccessory.address);
  var accessoryConfig = this.accessoryConfigs[accessoryAddress];
  this.bluetoothAccessories[accessoryAddress] = new BluetoothAccessory(this.log, accessoryConfig, nobleAccessory, this);
  nobleAccessory.on('disconnect', function (error) {
    this.disconnect(error, this.bluetoothAccessories[accessoryAddress])
  }.bind(this));
};


BluetoothPlatform.prototype.disconnect = function (error, bluetoothAccessory) {
  if (error) {
    var nobleAccessory = bluetoothAccessory.nobleAccessory;
    this.log.error("Disconnecting failed | " + nobleAccessory.advertisement.localName + " - " + nobleAccessory.address + " | " + error);
  }

  bluetoothAccessory.nobleAccessory.removeAllListeners('disconnect');
  bluetoothAccessory.disconnect();
  Noble.startScanning([], false);
};


function trimAddress(address) {
  return address.toLowerCase().replace(/:/g, "");
}
