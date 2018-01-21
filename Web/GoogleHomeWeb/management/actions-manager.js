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

module.exports = ActionManager;
