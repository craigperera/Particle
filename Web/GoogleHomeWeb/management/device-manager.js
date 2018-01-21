const util = require('util');

function DeviceManager(device) {

    this.id = device.id;
    this.name = device.name;
    this.modelType = getModelType(device.product_id);
    this.hwVersion = device.system_firmware_version;
    this.swVersion = device.current_build_target;
    this.connected = device.connected;
    this.status = device.status;

    //  defaults
    this.addToGoogleAccount = false;
    this.deviceType;
    this.deviceTraits = [];
    this.deviceAttributes = {};
    this.traitsWithAttributes = [];

    this.image = util.format("%s.png", this.modelType);
}

/*
    Set the device type
*/
DeviceManager.prototype.setDeviceType = function (devType) {

    this.deviceType = getDeviceType(devType);
};

/*
    Add The trait to the device
*/
DeviceManager.prototype.addDeviceTrait = function (trait, traitData) {

    var mappedTrait = getTrait(trait);
    this.deviceTraits.push(mappedTrait);

    if (!traitData) {

        this.traitsWithAttributes.push({ traitName: mappedTrait, traitAttributes: null })
        return;
    }

    var interim = {};
    var split = traitData.split(';');

    for (var i = 0; i < split.length; i++) {

        var kvp = split[i].split('=');

        if (kvp.length != 2) {

            continue;
        }

        interim[kvp[0]] = kvp[1];
        this.deviceAttributes[kvp[0]] = kvp[1];
    }

    this.traitsWithAttributes.push({ traitName: mappedTrait, traitAttributes: interim })
};

/*
    Get the Google Response for the device
*/
DeviceManager.prototype.getGoogleResponse = function () {

    if (!this.addToGoogleAccount) {

        return;
    }

    //  now we can setup the device
    return {

        id: this.id,
        type: this.deviceType,
        traits: this.deviceTraits,
        name: {
            defaultNames: [this.name],
            name: this.name,
            nicknames: [this.name]
        },
        willReportState: false,
        attributes: this.deviceAttributes,
        deviceInfo: {
            manufacturer: "Particle",
            model: this.modelType,
            hwVersion: this.hwVersion,
            swVersion: this.swVersion
        },
        customData: {
            connected: this.connected,
            status: this.status,
            modelType: this.modelType
        }
    };
}

/*
  From the product_id get the name of the Particle 
*/
function getModelType(modelType) {

    switch (modelType) {

        case 0:
        case 2: {

            return "Spark";
        }
        case 3: {

            return "GCC";
        }
        case 4:
        case 6: {

            return "Photon";
        }
        case 8: {

            return "P1";
        }
        case 10: {

            return "Electron";
        }
        case 31: {

            return "Raspberry Pi";
        }
        default: {

            return "Unknown";
        }
    }
};

/*
    From the supplied return result determine the device type
*/
function getDeviceType(deviceType) {

    switch (deviceType) {

        case 1: {

            return "action.devices.types.CAMERA";
        }
        case 2: {

            return "action.devices.types.DISHWASHER";
        }
        case 3: {

            return "action.devices.types.DRYER";
        }
        case 4: {

            return "action.devices.types.LIGHT";
        }
        case 5: {

            return "action.devices.types.OUTLET";
        }
        case 7: {

            return "action.devices.types.THERMOSTAT";
        }
        case 8: {

            return "action.devices.types.VACUUM";
        }
        case 9: {

            return "action.devices.types.WASHER";
        }
        default:
        case 6: {

            return "action.devices.types.SWITCH";
        }
    }
};

/*
    Get the device trait name
*/
function getTrait(deviceTrait) {

    if (deviceTrait == 1) {

        return "action.devices.traits.Brightness";
    }

    if (deviceTrait == 2) {

        return "action.devices.traits.CameraStream";
    }

    if (deviceTrait == 4) {

        return "action.devices.traits.ColorSpectrum";
    }

    if (deviceTrait == 8) {

        return "action.devices.traits.ColorTemperature";
    }

    if (deviceTrait == 16) {

        return "action.devices.traits.Dock";
    }

    if (deviceTrait == 32) {

        return "action.devices.traits.Modes";
    }

    if (deviceTrait == 64) {

        return "action.devices.traits.OnOff";
    }

    if (deviceTrait == 128) {

        return "action.devices.traits.RunCycle";
    }

    if (deviceTrait == 256) {

        return "action.devices.traits.Scene";
    }

    if (deviceTrait == 512) {

        return "action.devices.traits.StartStop";
    }

    if (deviceTrait == 1024) {

        return "action.devices.traits.TemperatureSetting";
    }

    if (deviceTrait == 2048) {

        return "action.devices.traits.Toggles";
    }
};

module.exports = DeviceManager;
