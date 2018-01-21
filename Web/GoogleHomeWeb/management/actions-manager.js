const fetch = require('node-fetch');
const util = require('util');

const ParticleHelper = require('../management/particle-manager');
const ParticleManager = new ParticleHelper();

function ActionManager() {

};

/*
    Google Home Sync Command
*/
ActionManager.prototype.HandleSync = async function (data, response) {

    var customerId = data.uid;

    if (!customerId || customerId <= 0) {

        response.status(500).set({
            'Access-Control-Allow-Origin': '*',
            'Access-Control-Allow-Headers': 'Content-Type, Authorization'
        }).json({ error: "failed" });

        return;
    }

    //  get devices for the customer id
    var devices = await ParticleManager.loadDevices(customerId, true);

    if (devices) {

        let deviceProps = {
            requestId: data.requestId,
            payload: {
                agentUserId: data.uid,
                devices: devices
            }
        };

        response.status(200).json(deviceProps);
        return deviceProps;
    }
};

/*
    Google Home Query Command
*/
ActionManager.prototype.HandleQuery = async function (data, response) {

    //  first find the device id(s) from the request
    let deviceIds = getDeviceIds(data.devices);
    let authToken = data.auth;
    let devicePayload = {};

    //  get the traits for each device
    for (var i = 0; i < deviceIds.length; i++) {

        var deviceId = deviceIds[i];

        let traitResults = {};

        traitResults["online"] = true;

        var res = await ParticleManager.getVariableValue(deviceId, "gData", authToken);

        if (!res) {

            continue;
        }

        var vals = res.split(';');

        for (z = 0; z < vals.length; z++) {

            var item = vals[z];

            var kvp = item.split('=');

            if (!kvp || kvp.length != 2) {

                continue;
            }

            var val = parseFloat(kvp[1]);

            if (isNaN(val)) {

                traitResults[kvp[0]] = kvp[1];
            }
            else {

                traitResults[kvp[0]] = val;
            }
        }

        devicePayload[deviceId] = traitResults;
    }

    let deviceStates = {
        requestId: data.requestId,
        payload: {
            devices: devicePayload
        }
    };

    response.status(200).json(deviceStates);
    return deviceStates;
}

/*
  Get a list of device id's that are involved in the command
*/
function getDeviceIds(devices) {

    let deviceIds = [];

    for (let i = 0; i < devices.length; i++) {

        if (devices[i] && devices[i].id)

            deviceIds.push(devices[i].id);
    }

    return deviceIds;
}

module.exports = ActionManager;
