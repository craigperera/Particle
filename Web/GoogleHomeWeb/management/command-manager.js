const util = require('util');

function CommandManager() {

    this.successIds = [];
    this.errorIds = [];

    this.states = {};
    this.errorCode;
}

/*
    Add a success state to the list
*/
CommandManager.prototype.addSuccessState = function (data) {

    this.states["online"] = true;

    var vals = data.split(';');

    for (z = 0; z < vals.length; z++) {

        var item = vals[z];

        var kvp = item.split('=');

        if (!kvp || kvp.length != 2) {

            continue;
        }

        var val = parseFloat(kvp[1]);

        if (isNaN(val)) {

            this.states[kvp[0]] = kvp[1];
        }
        else {

            this.states[kvp[0]] = val;
        }
    }
}

/*
    Add a failed state to the list
*/
CommandManager.prototype.setFailureState = function (result) {

    if (!result || result >= 0) {

        this.errorCode = "unknownError";
        return;
    }

    switch (result) {

        case -2: {

            this.errorCode = "authExpired";
            return;
        }
        case -3: {

            this.errorCode = "authFailure";
            return;
        }
        case -4: {

            this.errorCode = "deviceOffline";
            return;
        }
        case -5: {

            this.errorCode = "timeout";
            return;
        }
        case -6: {

            this.errorCode = "deviceTurnedOff";
            return;
        }
        case -7: {

            this.errorCode = "deviceNotFound";
            return;
        }
        case -8: {

            this.errorCode = "valueOutOfRange";
            return;
        }
        case -9: {

            this.errorCode = "notSupported";
            return;
        }
        case -10: {

            this.errorCode = "protocolError";
            return;
        }
        case -100: {

            this.errorCode = "resourceUnavailable";
            return;
        }
        case -200: {

            this.errorCode = "inHeatOrCool";
            return;
        }
        case -201: {

            this.errorCode = "inHeatCool";
            return;
        }
        case -202: {

            this.errorCode = "lockedToRange";
            return;
        }
        case -203: {

            this.errorCode = "rangeTooClose";
            return;
        }
        default:
        case -1: {

            this.errorCode = "unknownError";
            return;
        }
    }
}

module.exports = CommandManager;

