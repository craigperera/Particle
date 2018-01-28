const fetch = require('node-fetch');
const util = require('util');

const ParticleHelper = require('../management/particle-manager');
const CommandManager = require('../management/command-manager');
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
    var devices = await ParticleManager.forcefail(customerId, true);

    //todo: delete
//    var devices = await ParticleManager.loadDevices(customerId, true);

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
  Execute request from google home
*/
ActionManager.prototype.HandleExec = async function exec(data, response) {

    //  first find the device id(s) from the request
    let customerId = data.uid;
    let authToken = data.auth;

    let commands = [];

    for (var i = 0; i < data.commands.length; i++) {

        var cmdResponse = new CommandManager();
        var command = data.commands[i];

        if (!command || !command.devices) {

            continue;
        }

        //  for each command get the device(s) involved
        for (j = 0; j < command.devices.length; j++) {

            var device = command.devices[j];

            if (!device || !device.id) {

                continue;
            }

            var deviceId = device.id;
            var deviceCommand;

            //  now get the execution information
            for (k = 0; k < command.execution.length; k++) {

                var executionItem = command.execution[k];
                var commandSplit = executionItem.command.split('.');

                var executeCommand = getExecutionCommand(commandSplit[commandSplit.length - 1]);
                var keys = Object.keys(executionItem.params);
                var parmString = "";

                for (a = 0; a < keys.length; a++) {

                    var paramVal = getExecutionParameter(keys[a]);
                    var parm = executionItem.params[keys[a]];

                    parmString = util.format("%s%d=%s^", parmString, paramVal, parm);
                }

                var dcs = util.format("%d^%s", executeCommand, parmString);

                if (!deviceCommand) {

                    deviceCommand = dcs;
                }
                else {

                    deviceCommand = util.format("%s,%s", deviceCommand, dcs);
                }
            }

            //  ensure the array ends with ,
            deviceCommand = deviceCommand + ",";

            //  we can only send a maximum of 63 bytes to the function so we need to split into 63 byte chunks
            var size = 63;

            if (deviceCommand.length > size) {

                var re = new RegExp('.{1,' + size + '}', 'g');
                var split = deviceCommand.match(re);

                //  send each to the particle
                for (var l = 0; l < split.length; l++) {

                    var res = await ParticleManager.execute(deviceId, authToken, {
                        commandCount: split.length,
                        commandIndex: l + 1,
                        command: split[l]
                    });

                    if (!res) {

                        cmdResponse.errorIds.push(deviceId);
                        cmdResponse.setFailureState(-1);
                    }

                    if (res < 0) {

                        cmdResponse.errorIds.push(deviceId);
                        cmdResponse.setFailureState(res);
                    }
                }
            }
            else {

                //  we can send the whole string
                var res = await ParticleManager.execute(deviceId, authToken, {

                    commandCount: 1,
                    commandIndex: 1,
                    command: deviceCommand
                });

                if (!res) {

                    cmdResponse.errorIds.push(deviceId);
                    cmdResponse.setFailureState(-1);
                }
                else if (res < 0) {

                    cmdResponse.errorIds.push(deviceId);
                    cmdResponse.setFailureState(res);
                }
                else {

                    //  retrieve the value of the respective variable
                    var res = await ParticleManager.getVariableValue(deviceId, "gData", authToken);

                    if (!res) {

                        cmdResponse.errorIds.push(deviceId);
                        cmdResponse.setFailureState(-1);
                    }

                    //  add state data
                    cmdResponse.successIds.push(deviceId);
                    cmdResponse.addSuccessState(res);
                }
            }

            //  report success and errors
            if (cmdResponse.successIds.length > 0 && cmdResponse.errorIds.length > 0) {

                commands.push({
                    ids: cmdResponse.successIds,
                    status: "SUCCESS",
                    states: cmdResponse.states
                }, {
                        ids: cmdResponse.errorIds,
                        status: "ERROR",
                        errorCode: cmdResponse.errorCode
                    });

                continue;
            }

            // reporting success only
            if (cmdResponse.successIds.length > 0 && cmdResponse.errorIds.length == 0) {

                commands.push({
                    ids: cmdResponse.successIds,
                    status: "SUCCESS",
                    states: cmdResponse.states
                });

                continue;
            }

            if (cmdResponse.errorIds.length > 0 && cmdResponse.successIds.length == 0) {

                commands.push({
                    ids: cmdResponse.errorIds,
                    status: "ERROR",
                    errorCode: cmdResponse.errorCode
                });

                continue;
            }
        }
    }

    let resBody = {
        requestId: data.requestId,
        payload: {
            commands: commands
        }
    };

    console.log(JSON.stringify(resBody));
    response.status(200).json(resBody);
    return resBody;
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

/*
  Translates the execution command into an integer value
*/
function getExecutionCommand(command) {

    if (!command) {

        return;
    }

    switch (command.toLowerCase().trim()) {

        case "brightnessabsolute": {

            return 1;
        }
        case "getcamerastream": {

            return 2;
        }
        case "colorabsolute": {

            return 3;
        }
        case "dock": {

            return 4;
        }
        case "setmodes": {

            return 5;
        }
        case "onoff": {

            return 6;
        }
        case "activatescene": {

            return 7;
        }
        case "startstop": {

            return 8;
        }
        case "pauseunpause": {

            return 9;
        }
        case "thermostattemperaturesetpoint": {

            return 10;
        }
        case "thermostattemperaturesetrange": {

            return 11;
        }
        case "thermostatsetmode": {

            return 12;
        }
        case "settoggle": {

            return 13;
        }
        default: {
            return;
        }
    }
};

/*
  Translates the parameter name into an integer value
*/
function getExecutionParameter(paramName) {

    if (!paramName) {

        return;
    }

    switch (paramName.toLowerCase().trim()) {

        case "brightness": {

            return 1;
        }
        case "streamtochromecast": {

            return 2;
        }
        case "supportedstreamprotocols": {

            return 3;
        }
        case "color": {

            return 4;
        }
        case "updatemodesettings": {

            return 5;
        }
        case "on": {

            return 6;
        }
        case "deactivate": {

            return 7;
        }
        case "start": {

            return 8;
        }
        case "pause": {

            return 9;
        }
        case "thermostattemperaturesetpoint": {

            return 10;
        }
        case "thermostattemperaturesetpointhigh": {

            return 11;
        }
        case "thermostattemperaturesetpointlow": {

            return 12;
        }
        case "thermostatmode": {

            return 13;
        }
        case "updatetogglesettings": {

            return 14;
        }
        default: {

            return;
        }
    }
};

module.exports = ActionManager;
