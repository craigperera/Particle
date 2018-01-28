const datastore = require('@google-cloud/datastore');
const DataStore = datastore();

function DatastoreManager() {

}

/*
    Save the token in the data store
*/
DatastoreManager.prototype.getCustomerId = async function (particleLogin, token) {

    //  first see if customer already exists
    var customerId = await findLoggedinUser(particleLogin, token).catch(function (err) {

        console.log(err);
        return -999;
    });

    if (customerId > 0) {

        return customerId;
    }

    customerId = await findNextUser().catch(function (err) {

        console.log(err);
        return -999;
    });

    //  if customer Id not found then create the customer id
    const customer = {

        customerId: customerId,
        particleLogin: particleLogin,
        authToken: token.token,
        refreshToken: token.refreshToken,
        expiresIn: token.expiresIn
    }

    //  save the user
    var res = await saveUser(customer);
    return customerId;
};

/*
    Retrieve the customer details from their authorisation token
*/
DatastoreManager.prototype.getCustomerFromToken = async function (auth_token) {

    //  first see if customer already exists
    var tokenObj = await findCustomerFromToken(auth_token).catch(function (err) {

        console.log(err);
        return -999;
    });

    if (!tokenObj || tokenObj == null) {

        return -999;
    }

    return tokenObj;
};

/*
    Retrieve the customer details from their customer Id
*/
DatastoreManager.prototype.getTokenFromCustomerId = async function (customerId) {

    //  first see if customer already exists
    var tokenObj = await findTokenFromCustomerId(customerId).catch(function (err) {

        console.log(err);
        return -999;
    });

    if (!tokenObj || tokenObj == null) {

        return -999;
    }

    return tokenObj;
};

/*
    Save Customer Devices

    We persist the device details with it's google response so we can quickly return it during sync
*/
DatastoreManager.prototype.saveCustomerDevice = async function (customerId, deviceId, googleResponse) {

    //  first check if the details already exist for this device
    var device = await findCustomerDevice(customerId, deviceId, googleResponse);

    //  device doesn't exist so create it
    if (!device || device == null) {

        //  if customer Id not found then create the customer id
        const device = {

            customerId: customerId,
            deviceId: deviceId,
            response: googleResponse
        }

        //  save the user
        var res = await saveDevice(device);
        return;
    }

    return;
};

/*
    Returns a list of the customers devices
*/
DatastoreManager.prototype.loadCustomerDevices = async function (customerId) {

    var devices = await findCustomerDevices(customerId);
    return devices;
};

/*
    Attempt to find an existing registered user by their particle email address

    If the user exists we need to update their token details

    Inputs:     particleLogin 
    Returns:    customerId if user exists, -1 if not
*/
function findLoggedinUser(particleLogin, token) {

    var customerId = -1;

    const query = DataStore.createQuery('customer')
        .filter('particleLogin', '=', particleLogin)
        .order('customerId');

    //  first get customer id
    return new Promise(function (resolve, reject) {

        var res = DataStore.runQuery(query).then(async function (results) {

            const entities = results[0];

            if (entities.length > 0) {

                const entity = entities[0];
                const key = entity[datastore.KEY];

                customerId = entity.customerId;

                //  update the entity
                entity.authToken = token.token;
                entity.refreshToken = token.refreshToken;
                entity.expiresIn = token.expiresIn;

                var res = await updateData(key, entity).catch(function (err) {

                    reject(new Error(err.message));
                });
            };

            resolve(customerId);

        }, function (err) {

            reject(new Error(err.message));
        });
    });
}

/*
    Get the next customer id based on the values already saved
*/
function findNextUser() {

    const query = DataStore.createQuery('customer').order('customerId', { descending: true });

    //  first get customer id
    return new Promise(function (resolve, reject) {

        var res = DataStore.runQuery(query).then((results) => {

            var customerId = 1;

            const entities = results[0];

            if (entities.length > 0) {

                const entity = entities[0];
                var cid = entity.customerId;

                customerId = cid + 1;
            }

            resolve(customerId);

        }, function (err) {

            reject(new Error(err.message));
        });
    });
}

/*
    Attempt to find an existing registered customer by their authorisation token

    Inputs:     authorisation token 
    Returns:    Object 
                    authToken
                    customerId
                    refreshToken
                    expiresIn
*/
function findCustomerFromToken(authToken) {

    const query = DataStore.createQuery('customer')
        .filter('authToken', '=', authToken)
        .order('customerId');

    //  first get customer id
    return new Promise(function (resolve, reject) {

        var res = DataStore.runQuery(query).then((results) => {

            const entities = results[0];

            if (entities.length > 0) {

                const entity = entities[0];

                resolve({
                    authToken: entity.authToken,
                    customerId: entity.customerId,
                    refreshToken: entity.refreshToken,
                    expiresIn: entity.expiresIn
                });
            }
            else {

                reject(new Error("No details found for token " + authToken));
            }
        }, function (err) {

            reject(new Error(err.message));
        });
    });
}

/*
    Attempt to find an existing registered customer by their Customer Id

    Inputs:     customer id 
    Returns:    Object 
                    authToken
                    customerId
                    refreshToken
                    expiresIn
*/
function findTokenFromCustomerId(customerId) {

    const query = DataStore.createQuery('customer').filter('customerId', '=', customerId);

    return new Promise(function (resolve, reject) {

        var res = DataStore.runQuery(query).then((results) => {

            const entities = results[0];

            if (entities.length > 0) {

                const entity = entities[0];

                resolve({
                    authToken: entity.authToken,
                    customerId: entity.customerId,
                    refreshToken: entity.refreshToken,
                    expiresIn: entity.expiresIn
                });
            }
            else {

                reject(new Error("No details found for customer id " + customerId));
            }
        }, function (err) {

            reject(new Error(err.message));
        });
    });
}

/*
    Save User into the DataStore

    Inputs:  customer Entity
    Outputs: insert results
*/
function saveUser(customer) {

    //  first get customer id
    return new Promise(function (resolve, reject) {

        var res = DataStore.save({

            key: DataStore.key('customer'),
            data: customer
        }).then(function (data) {

            resolve(data);

        }, function (err) {

            reject(new Error(err.message));
        });
    });
};

/*
    Updates the data
*/
function updateData(key, data) {

    //  first get customer id
    return new Promise(function (resolve, reject) {

        DataStore.update({

            key: key,
            data: data
        }).then(() => {

            resolve(1);

        }, function (err) {

            reject(new Error(err.message));
        });
    });
}

/*
    Retrieves all customer devices

    Inputs:     customerId
    Returns:    Object Array
                    customerId
                    deviceId
                    googleResponse
*/
function findCustomerDevices(customerId) {

    const query = DataStore.createQuery('devices')
        .filter('customerId', '=', customerId)
        .order('deviceId');

    return new Promise(function (resolve, reject) {

        var res = DataStore.runQuery(query).then(async function (results) {

            const devices = [];
            const entities = results[0];

            for (var i = 0; i < entities.length; i++) {

                const device = entities[i];

                var deviceDetail = {
                    customerId : device.customerId,
                    deviceId: device.deviceId,
                    response: device.response
                }

                devices.push(deviceDetail);
            }

            resolve(devices);
        }, function (err) {

            reject(new Error(err.message));
        });
    });
}

/*
    Attempt to find an existing customer device

    Inputs:     customerId, deviceId
    Returns:    Object
                    customerId
                    deviceId
                    googleResponse
*/
function findCustomerDevice(customerId, deviceId, googleResponse) {

    const query = DataStore.createQuery('devices')
        .filter('customerId', '=', customerId)
        .filter('deviceId', '=', deviceId)
        .order('deviceId');

    return new Promise(function (resolve, reject) {

        var res = DataStore.runQuery(query).then(async function (results) {

            const entities = results[0];

            if (entities.length > 0) {

                const entity = entities[0];
                const key = entity[datastore.KEY];

                //  update the entity
                if (googleResponse) {

                    entity.response = googleResponse;

                    var res = await updateData(key, entity).catch(function (err) {
    
                        reject(new Error(err.message));
                    });
                }
                
                resolve(entity);
            }
            else {

                //  no devices
                resolve(null);
            }
        }, function (err) {

            reject(new Error(err.message));
        });
    });
}
/*
    Save device into the DataStore

    Inputs:  object to store
    Outputs: none
*/
function saveDevice(deviceObj) {

    //  first get customer id
    return new Promise(function (resolve, reject) {

        var res = DataStore.save({

            key: DataStore.key('devices'),
            data: deviceObj
        }).then(function (data) {

            resolve(data);

        }, function (err) {

            reject(new Error(err.message));
        });
    });
};

module.exports = DatastoreManager;