var Characteristic;
var Chalk = require('chalk');

module.exports = function (characteristic) {
  Characteristic = characteristic;

  return BluetoothCharacteristic;
};


function BluetoothCharacteristic(log, config, bluetoothService) {
  this.log = log;
  this.config = config;
  this.prefix = bluetoothService.prefix + " " + Chalk.green("[" + config.type + "]");

  this.nobleCharacteristic = null;
  if (config.type in Characteristic) {
    this.type = config.type;
    var CharacteristicType = Characteristic[this.type]; // For example - Characteristic.Brightness
    this.homebridgeCharacteristic = bluetoothService.homebridgeService.getCharacteristic(CharacteristicType);
  } else {
    throw new Error("Type Characteristic." + this.type + " is not available. See 'HAP-NodeJS/lib/gen/HomeKitType.js' for options.")
  }

  if (config.UUID) {
    this.UUID = trimUUID(config.UUID);
  } else {
    throw new Error("Missing bluetooth UUID for Characteristic." + this.type + ".");
  }
}


BluetoothCharacteristic.prototype.onDiscover = function (nobleCharacteristic) {
  this.log.info(this.prefix, "Discovered | Characteristic." + this.type + " - " + this.UUID);
  this.nobleCharacteristic = nobleCharacteristic;

  for (var permission of this.homebridgeCharacteristic.props['perms']) {
    switch (permission) {

      case Characteristic.Perms.READ:
        // Corresponds to BLEPeripheral/BLERead property
        if (this.nobleCharacteristic.properties.indexOf('read') >= 0) {
          this.homebridgeCharacteristic.on('get', this.get.bind(this));
        } else {
          this.log.warn(this.prefix, "Read from bluetooth haracteristic not permitted | " + this.UUID);
        }
        break;

      case Characteristic.Perms.WRITE:
        // Corresponds to BLEPeripheral/BLEWrite property
        if (this.nobleCharacteristic.properties.indexOf('write') >= 0) {
          this.homebridgeCharacteristic.on('set', this.set.bind(this));
        } else {
          this.log.warn(this.prefix, "Write to bluetooth characteristic not permitted | " + this.UUID);
        }
        break;

      case Characteristic.Perms.NOTIFY:
        // Corresponds to BLEPeripheral/BLENotify property
        if (this.nobleCharacteristic.properties.indexOf('notify') >= 0) {
          this.nobleCharacteristic.on('read', this.notify.bind(this));
          this.nobleCharacteristic.subscribe(function (error) {
            if (error) {
              this.log.warn(this.prefix, "Subscribe to bluetooth characteristic failed | " + this.UUID);
            }
          }.bind(this));
        } else {
          this.log(this.prefix, "Subscribe to bluetooth characteristic not permitted | " + this.UUID);
        }
        break;
    }
  }

};


BluetoothCharacteristic.prototype.get = function (callback) {
  this.nobleCharacteristic.read(function (error, buffer) {
    if (error) {
      this.log.warn(this.prefix, "Read from bluetooth characteristic failed | " + error);
      callback(error, null);
      return
    }
    var value = this.fromBuffer(buffer);
    this.log.info(this.prefix, "Get | " + value);
    callback(null, value);
  }.bind(this));
};

BluetoothCharacteristic.prototype.set = function (value, callback) {
  this.log.info(this.prefix, "Set | " + value);
  var buffer = this.toBuffer(value);
  this.nobleCharacteristic.write(buffer, false);
  callback();
};


BluetoothCharacteristic.prototype.notify = function (buffer, notification) {
  if (notification) {
    var value = this.fromBuffer(buffer);
    this.log.info(this.prefix, "Notify | " + value);
    this.homebridgeCharacteristic.updateValue(value, null, null);
  }
};


BluetoothCharacteristic.prototype.toBuffer = function (value) {
  var buffer;
  switch (this.homebridgeCharacteristic.props['format']) {
    case Characteristic.Formats.BOOL:
      // Corresponds to BLECharCharacteristic
      buffer = Buffer.alloc(1);
      buffer.writeInt8(value ? 1 : 0, 0);
      break;

    case Characteristic.Formats.INT:
      // Corresponds to BLEPeripheral/BLEIntCharacteristic
      buffer = Buffer.alloc(4);
      buffer.writeInt32LE(value, 0);
      break;

    case Characteristic.Formats.FLOAT:
      // Corresponds to BLEPeripheral/BLEFloatCharacteristic
      buffer = Buffer.alloc(4);
      buffer.writeFloatLE(value, 0);
      break;

    default:
      // TODO Add support for more Characteristic.Formats
      this.log.warn(this.prefix, "Unsupported data conversion | " + this.UUID);
      buffer = Buffer.alloc(1);
      buffer.writeInt8(0, 0);
      break;
  }
  return buffer;
};


BluetoothCharacteristic.prototype.fromBuffer = function (buffer) {
  var value;
  switch (this.homebridgeCharacteristic.props['format']) {
    case Characteristic.Formats.BOOL:
      // Corresponds to BLECharCharacteristic
      value = buffer.readInt8(0);
      break;

    case Characteristic.Formats.INT:
      // Corresponds to BLEIntCharacteristic
      value = buffer.readInt32LE(0);
      break;

    case Characteristic.Formats.FLOAT:
      // Corresponds to BLEFloatCharacteristic
      value = buffer.readFloatLE(0);
      break;

    default:
      // TODO Add support for more Characteristic.Formats
      value = 0;
      this.log.warn(this.prefix, "Unsupported data conversion | " + this.UUID);
  }
  return value;
};


BluetoothCharacteristic.prototype.onDisconnect = function () {
  this.log.info(this.prefix, "Disconnected | " + this.UUID);
  if (this.nobleCharacteristic.properties.indexOf('read') >= 0) {
    this.homebridgeCharacteristic.removeAllListeners('get');
  }
  if (this.nobleCharacteristic.properties.indexOf('write') >= 0) {
    this.homebridgeCharacteristic.removeAllListeners('set');
  }
  if (this.nobleCharacteristic.properties.indexOf('notify') >= 0) {
    this.nobleCharacteristic.unsubscribe(null);
    this.nobleCharacteristic.removeAllListeners('read');
  }
  this.nobleCharacteristic = null;
};


function trimUUID(uuid) {
  return uuid.toLowerCase().replace(/:/g, "").replace(/-/g, "");
}
