var Noble, BluetoothAccessory;

module.exports = function (noble, bluetoothAccessory) {
  Noble = noble;
  BluetoothAccessory = bluetoothAccessory;

  return BluetoothPlatform;
};


function BluetoothPlatform(log, config, homebridgeAPI) {
  this.log = log;
  this.config = config;
  this.homebridgeAPI = homebridgeAPI;
  
  this.configs = {};
  config.accessories.map(function(accessoryConfig) {
    this.configs[accessoryConfig.address.toLowerCase()] = accessoryConfig;
  }.bind(this));
  
  this.homebridgeAPI.on('didFinishLaunching', this.onDidFinishLaunching.bind(this));
}


BluetoothPlatform.prototype.onDidFinishLaunching = function () {
  Noble.on('stateChange', this.onStateChange.bind(this));
};


BluetoothPlatform.prototype.onStateChange = function (state) {
  if (state != 'poweredOn') {
    this.log.info("Stopped | " + state);
    Noble.stopScanning();
  }

  this.log.info("Started | " + state);
  Noble.startScanning([], false);
  Noble.on('discover', this.onDiscoverAccessory.bind(this));
};


BluetoothPlatform.prototype.configureAccessory = function (bluetoothAccessory) {
  this.log.info("Configure cached accessory | ");
  this.log.info(bluetoothAccessory);
};


BluetoothPlatform.prototype.onDiscoverAccessory = function (nobleAccessory) {
  var address = nobleAccessory.address.toLowerCase();
  if (! (address in this.configs)) {
    this.log.info("Ignored | " + nobleAccessory.advertisement.localName + " - " + nobleAccessory.address);
    return;
  }
  
  this.log.info("Discovered | " + nobleAccessory.advertisement.localName + " - " + nobleAccessory.address);
  nobleAccessory.connect(function (error) {
    this.onConnectAccessory(error, nobleAccessory)
  }.bind(this));
};


BluetoothPlatform.prototype.onConnectAccessory = function (error, nobleAccessory) {
  if (error) {
    this.log.error("Connecting failed | "  + nobleAccessory.advertisement.localName + " - " + nobleAccessory.address);
    return;
  }
  
  var bluetoothAccessory = new BluetoothAccessory(this.log, this.configs[nobleAccessory.address], this.homebridgeAPI);
  this.homebridgeAPI.registerPlatformAccessories("homebridge-bluetooth", "Bluetooth", [bluetoothAccessory.homebridgeAccessory]);
  bluetoothAccessory.onConnect(nobleAccessory);
  nobleAccessory.on('disconnect', function (error) {
    this.onDisconnectAccessory(error, bluetoothAccessory)
  }.bind(this));
};


BluetoothPlatform.prototype.onDisconnectAccessory = function (error, bluetoothAccessory) {
  if (error) {
    this.log.error("Disconnecting failed | "  + bluetoothAccessory.name + " - " + bluetoothAccessory.address);
  }

  this.homebridgeAPI.unregisterPlatformAccessories("homebridge-bluetooth", "Bluetooth", [bluetoothAccessory.homebridgeAccessory]);
  Noble.startScanning([], false);
};

