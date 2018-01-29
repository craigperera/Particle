const DeviceManager = require('./device-manager');
const authProvider = require('./auth-manager');
const datastoreManager = require('./datastore-manager');
const DatastoreManager = new datastoreManager();

const Particle = require('particle-api-js');
const util = require('util');

const particleApp = new Particle();

function ParticleManager() {

    this.UserDevices = [];
}

/*
    Attempt to Login to Particle Account

    On Success returns auth token on error null
*/
ParticleManager.prototype.getAuthToken = async function (particleUser, particlePwd) {

    let token = await getAuthorisationToken(particleUser, particlePwd).catch(function (err) {

        console.error(err);
        return;
    });

    if (!token) {

        return;
    }

    var id = await authProvider.saveCustomerDetail(particleUser, token);

    if (id < 0) {

        return;
    }

    return {
        authToken: token.token,
        customerId: id
    };
};

/*
    Load Devices for Google Home Sync command
*/
ParticleManager.prototype.loadDevices = async function (customerId) {

    if (!customerId || customerId <= 0) {

        return false;
    }

    //  get customer id from token
    var tokenDetail = await authProvider.getCustomerFromId(customerId);

    if (!tokenDetail || !tokenDetail.authToken) {

        return false;
    }

    let deviceList = await loadDeviceList(tokenDetail.authToken).catch(function (err) {

        return false;
    });

    if (!deviceList || deviceList.length == 0) {

        return false;
    }

    //  now we need to find devices for google home, this is handled by the firmware
    for (var i = 0; i < deviceList.length; i++) {

        var device = deviceList[i];

        //  if device is not connected to the cloud we can't use it
        if (!device.connected) {

            continue;
        }

        var counter = 0;
        var deviceInfo = new DeviceManager(device);

        while (true) {

            //  get the initial trait information
            var traitRes = await callParticleFunction(device.id, tokenDetail.authToken, "gInfo", "traits;" + counter).catch(function (err) {

                console.error(err);
                return;
            });

            if (!traitRes) {

                break;
            }

            //  first high order byte is the device type
            var deviceType = traitRes >> 24;

            //  second high order byte is the trait count
            var traitCount = (traitRes & 0x00FF0000) >> 16;

            //  low order byte is the trait
            var trait = traitRes & 0x0000FFFF;
            var traitAttributes;

            //  does the trait have attributes ?
            var hasAttributes = (trait == 2 || trait == 4 || trait == 8 || trait == 32 || trait == 256 || trait == 512 || trait == 1024 || trait == 2048);

            //  get the variable for the attributes
            if (hasAttributes) {

                traitAttributes = await getParticleVariableValue(device.id, tokenDetail.authToken, "gTraits").catch(function (err) {

                    console.error(err);
                });
            }

            //  update the device info
            if (counter == 0) {

                deviceInfo.setDeviceType(deviceType);
                deviceInfo.addToGoogleAccount = true;
            }

            deviceInfo.addDeviceTrait(trait, traitAttributes);
            counter = counter + 1;

            if (counter >= traitCount) {

                //todo: delete
                /*
                //  add to the list of users devices
                if (googleResponseRequired) {

                    devices.push(deviceInfo.getGoogleResponse());
                }
                else {

                    devices.push(deviceInfo);
                }*/

                //  save the device
                var res = await DatastoreManager.saveCustomerDevice(customerId, device.id, deviceInfo.getGoogleResponse());
                break;
            }
        }
    }

    /*
    //  save the users devices
    this.UserDevices.push({
        customerId: tokenDetail.customerId,
        googleDevices: devices
    });

    return devices;*/
    return true;
}

/*
    Get the specified variable value from the device
*/
ParticleManager.prototype.getVariableValue = async function (deviceId, variableName, authToken) {

    return await getParticleVariableValue(deviceId, authToken, variableName).catch(function (err) {

        console.error(err);
    });
};

/*
    Send a command to the particle
*/
ParticleManager.prototype.execute = async function (deviceId, authToken, inpData) {

    var sendArgs = util.format("%d;%d;%s;", inpData.commandCount, inpData.commandIndex, inpData.command);

    var res = await callParticleFunction(deviceId, authToken, "gExecute", sendArgs).catch(function (err) {

        console.error(err);
    });

    return res;
};

/*
    Get Authorisation Token
*/
function getAuthorisationToken(particleUser, particlePassword) {

    //  first get customer id
    return new Promise(function (resolve, reject) {

        particleApp.login({
            username: particleUser,
            password: particlePassword,
            tokenDuration: 0
        }).then(function (data) {

            resolve({
                token: data.body.access_token,
                refreshToken: data.body.access_token,
                expiresIn: data.body.expires_in
            });
        }, function (err) {

            reject(new Error(err.message));
        });
    });
};

/*
    Load Users Device List

    On Success returns the list of devices, null on error
*/
function loadDeviceList(authToken) {

    return new Promise(function (resolve, reject) {

        //  first load the users device list
        var deviceList = particleApp.listDevices({
            auth: authToken
        }).then(function (devices) {

            resolve(devices.body);

        }, function (err) {

            reject(new Error(err.message));
        });
    });
};

/*
    Call's a particle function

    On Success returns the result (int) from the call, null if an error
*/
function callParticleFunction(deviceId, authToken, name, argument) {

    return new Promise(function (resolve, reject) {

        var fnPr = particleApp.callFunction({
            deviceId: deviceId,
            name: name,
            argument: argument,
            auth: authToken
        }).then(function (data) {

            resolve(data.body.return_value);

        }, function (err) {

            reject(new Error(err.message));
        });
    });
};

/*
    Get the value of a Particle Variable

    On Success returns the value returned in the variable, null on error
*/
function getParticleVariableValue(deviceId, authToken, name) {

    return new Promise(function (resolve, reject) {

        particleApp.getVariable({
            deviceId: deviceId,
            name: name,
            auth: authToken
        }).then(function (data) {

            resolve(data.body.result);

        }, function (err) {

            reject(new Error(err.message));
        });
    });
};

module.exports = ParticleManager;