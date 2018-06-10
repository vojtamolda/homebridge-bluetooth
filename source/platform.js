var Noble, UUIDGen, Accessory, BluetoothAccessory;

module.exports = function (noble, uuidGen, accessory, bluetoothAccessory) {
  Noble = noble;
  UUIDGen = uuidGen
  Accessory = accessory;
  BluetoothAccessory = bluetoothAccessory;

  return BluetoothPlatform;
};


function BluetoothPlatform(log, config, homebridgeAPI) {
  this.log = log;

  if (!config) {
    this.log.warn("Missing mandatory platform config named 'Bluetooth'");
    return;
  }

  if (!config.accessories || !(config.accessories instanceof Array)) {
    this.log.warn("Missing mandatory config 'accessories'");
    return;
  }
  this.bluetoothAccessories = {};
  for (var accessoryConfig of config.accessories) {
    var accessoryAddress = trimAddress(accessoryConfig.address);
    var bluetoothAccessory = new BluetoothAccessory(this.log, accessoryConfig);
    this.bluetoothAccessories[accessoryAddress] = bluetoothAccessory;
  }
  this.cachedHomebridgeAccessories = {};

  this.homebridgeAPI = homebridgeAPI;
  this.homebridgeAPI.on('didFinishLaunching', this.didFinishLaunching.bind(this));
}


BluetoothPlatform.prototype.configureAccessory = function (homebridgeAccessory) {
  var accessoryAddress = homebridgeAccessory.context['address'];
  var bluetoothAccessory = this.bluetoothAccessories[accessoryAddress];
  if (!bluetoothAccessory) {
    this.log.debug("Removed | " + homebridgeAccessory.displayName + " (" + accessoryAddress + ")");
    this.homebridgeAPI.unregisterPlatformAccessories("homebridge-bluetooth", "Bluetooth",
                                                     [homebridgeAccessory]);
    return;
  }

  this.log.debug("Persist | " + homebridgeAccessory.displayName + " (" + accessoryAddress + ")");
  this.cachedHomebridgeAccessories[accessoryAddress] = homebridgeAccessory;
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


BluetoothPlatform.prototype.discover = function (nobleAccessory) {
  var accessoryAddress = trimAddress(nobleAccessory.address);
  var bluetoothAccessory = this.bluetoothAccessories[accessoryAddress];
  if (!bluetoothAccessory) {
    this.log.debug("Ignored | " + nobleAccessory.advertisement.localName +
                  " (" + nobleAccessory.address + ") | RSSI " + nobleAccessory.rssi + "dB");
    return;
  }

  this.log.debug("Discovered | " + nobleAccessory.advertisement.localName +
                " (" + nobleAccessory.address + ") | RSSI " + nobleAccessory.rssi + "dB");
  nobleAccessory.connect(function (error) {
    this.connect(error, nobleAccessory)
  }.bind(this));
};


BluetoothPlatform.prototype.connect = function (error, nobleAccessory) {
  if (error) {
    this.log.error("Connecting failed | " + nobleAccessory.advertisement.localName +
                   " (" + nobleAccessory.address + ") | " + error);
    return;
  }

  var accessoryAddress = trimAddress(nobleAccessory.address);
  var bluetoothAccessory = this.bluetoothAccessories[accessoryAddress];
  var homebridgeAccessory = this.cachedHomebridgeAccessories[accessoryAddress];
  if (!homebridgeAccessory) {
    homebridgeAccessory = new Accessory(bluetoothAccessory.name,
                                        UUIDGen.generate(bluetoothAccessory.name));
    homebridgeAccessory.context['address'] = accessoryAddress;
    this.homebridgeAPI.registerPlatformAccessories("homebridge-bluetooth", "Bluetooth",
                                                   [homebridgeAccessory]);
  } else {
    delete this.cachedHomebridgeAccessories[accessoryAddress];
  }

  bluetoothAccessory.connect(nobleAccessory, homebridgeAccessory);
  nobleAccessory.once('disconnect', function (error) {
    this.disconnect(nobleAccessory, homebridgeAccessory, error);
  }.bind(this));

  if (Object.keys(this.cachedHomebridgeAccessories).length > 0) {
    Noble.startScanning([], false);
  }
};


BluetoothPlatform.prototype.disconnect = function (nobleAccessory, homebridgeAccessory, error) {
  var accessoryAddress = trimAddress(nobleAccessory.address);
  this.cachedHomebridgeAccessories[accessoryAddress] = homebridgeAccessory;

  Noble.startScanning([], false);
};


function trimAddress(address) {
  return address.toLowerCase().replace(/:/g, "");
}
