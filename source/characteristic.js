var Characteristic;
var Chalk = require('chalk');

module.exports = function (characteristic) {
  Characteristic = characteristic;

  return BluetoothCharacteristic;
};


function BluetoothCharacteristic(log, config, prefix) {
  this.log = log;

   if (!config.type) {
    throw new Error(this.prefix + " Missing mandatory config 'type'");
  }
  this.type = config.type;
  this.prefix = prefix + " " + Chalk.green("[" + this.type + "]");
  if (!Characteristic[this.type]) {
    throw new Error(this.prefix + " Characteristic type '" + this.type + "' is not defined. " +
                    "See 'HAP-NodeJS/lib/gen/HomeKitType.js' for options.")
  }
  this.class = Characteristic[this.type]; // For example - Characteristic.Brightness

  if (!config.UUID) {
    throw new Error(this.prefix + " Missing mandatory config 'UUID'");
  }
  this.UUID = config.UUID;

  this.log.info(this.prefix, "Initialized | Characteristic." + this.type + " - " + this.UUID);

  this.homebridgeCharacteristic = null;
  this.nobleCharacteristic = null;
}


BluetoothCharacteristic.prototype.connect = function (nobleCharacteristic, homebridgeCharacteristic) {
  this.log.info(this.prefix, "Connected | Characteristic." + this.type + " - " + this.UUID);
  this.homebridgeCharacteristic = homebridgeCharacteristic;

  this.nobleCharacteristic = nobleCharacteristic;
  for (var permission of this.homebridgeCharacteristic.props['perms']) {
    switch (permission) {
      case Characteristic.Perms.READ:
        if (this.nobleCharacteristic.properties.indexOf('read') >= 0) {
          this.homebridgeCharacteristic.on('get', this.get.bind(this));
        } else {
          this.log.warn(this.prefix, "Read from bluetooth characteristic not permitted");
        }
        break;
      case Characteristic.Perms.WRITE:
        if (this.nobleCharacteristic.properties.indexOf('write') >= 0) {
          this.homebridgeCharacteristic.on('set', this.set.bind(this));
        } else {
          this.log.warn(this.prefix, "Write to bluetooth characteristic not permitted");
        }
        break;
      case Characteristic.Perms.NOTIFY:
        if (this.nobleCharacteristic.properties.indexOf('notify') >= 0) {
          this.nobleCharacteristic.on('read', this.notify.bind(this));
          this.nobleCharacteristic.subscribe(function (error) {
            if (error) {
              this.log.warn(this.prefix, "Subscribe to bluetooth characteristic failed");
            }
          }.bind(this));
        } else {
          this.log(this.prefix, "Subscribe to bluetooth characteristic not permitted");
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
    this.homebridgeCharacteristic.updateValue(value, null, this);
  }
};


BluetoothCharacteristic.prototype.toBuffer = function (value) {
  var buffer;
  switch (this.homebridgeCharacteristic.props['format']) {
    case Characteristic.Formats.BOOL: // BLECharCharacteristic
      buffer = Buffer.alloc(1);
      buffer.writeInt8(value ? 1 : 0, 0);
      break;
    case Characteristic.Formats.INT: // BLEIntCharacteristic
      buffer = Buffer.alloc(4);
      buffer.writeInt32LE(value, 0);
      break;
    case Characteristic.Formats.FLOAT: // BLEFloatCharacteristic
      buffer = Buffer.alloc(4);
      buffer.writeFloatLE(value, 0);
      break;
    case Characteristic.Formats.STRING: // BLECharacteristic
      buffer = Buffer.from(value, 'utf8');
      break;
    case Characteristic.Formats.UINT8: // BLEUnsignedCharCharacteristic
      buffer = Buffer.alloc(1);
      buffer.writeUInt8(value, 0);
      break;
    case Characteristic.Formats.UINT16: // BLEUnsignedShortCharacteristic
      buffer = Buffer.alloc(2);
      buffer.writeUInt16(value, 0);
      break;
    case Characteristic.Formats.UINT32: // BLEUnsignedIntCharacteristic
      buffer = Buffer.alloc(4);
      buffer.writeUInt32(value, 0);
      break;
    case Characteristic.Formats.UINT64: // BLEUnsignedLongCharacteristic
      buffer = Buffer.alloc(8);
      buffer.writeUIntLE(value, 0, 8);
      break;
    default:
      this.log.error(this.prefix, "Unsupported data conversion | " +
                     this.homebridgeCharacteristic.props['format']);
      buffer = Buffer.alloc(1);
      buffer.writeInt8(0, 0);
      break;
  }
  return buffer;
};


BluetoothCharacteristic.prototype.fromBuffer = function (buffer) {
  var value;
  switch (this.homebridgeCharacteristic.props['format']) {
    case Characteristic.Formats.BOOL: // BLECharCharacteristic
      value = buffer.readInt8(0);
      break;
    case Characteristic.Formats.INT: // BLEIntCharacteristic
      value = buffer.readInt32LE(0);
      break;
    case Characteristic.Formats.FLOAT: // BLEFloatCharacteristic
      value = buffer.readFloatLE(0);
      break;
    case Characteristic.Formats.STRING: // BLECharacteristic
      value = buffer.toString('utf8', 0);
      break;
    case Characteristic.Formats.UINT8: // BLEUnsignedCharCharacteristic
      value = buffer.readUInt8(0);
      break;
    case Characteristic.Formats.UINT16: // BLEUnsignedShortCharacteristic
      value = buffer.readUInt16LE(0);
      break;
    case Characteristic.Formats.UINT32: // BLEUnsignedIntCharacteristic
      value = buffer.readUInt32LE(0);
      break;
    case Characteristic.Formats.UINT64: // BLEUnsignedLongCharacteristic
      value = buffer.readUIntLE(0, 8);
      break;
    default:
      value = 0;
      this.log.error(this.prefix, "Unsupported data conversion | " +
                     this.homebridgeCharacteristic.props['format']);
  }
  return value;
};


BluetoothCharacteristic.prototype.disconnect = function () {
  if (this.nobleCharacteristic && this.homebridgeCharacteristic) {
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
    this.homebridgeCharacteristic = null;
    this.nobleCharacteristic = null;
    this.log.info(this.prefix, "Disconnected");
  }
};
