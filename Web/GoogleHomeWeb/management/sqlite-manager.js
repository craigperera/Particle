const sqlite = require('sqlite3').verbose();
const PathResolve = require('path');
const util = require('util');

var dbPath;

function SqliteManager(path) {

    dbPath = PathResolve.resolve(__dirname, path);
};

/*
    Check if the particle user exists in the database

    returns CustomerId if present, -1, or -99 if an error
*/
SqliteManager.prototype.checkUserExists = async function (particleUser) {

    let db = new sqlite.Database(dbPath, sqlite.OPEN_READWRITE);
    let result = -1;

    let customerId = await getCustomerId(particleUser, db).catch(function (err) {

        console.log(err);
        result = -99;
    });

    if (customerId && customerId > 0) {

        result = customerId;
    }

    db.close();
    return result;
};

/*
    Register the user with their main access token
*/
SqliteManager.prototype.registerUser = async function (particleUser, accessToken, token) {

    let db = new sqlite.Database(dbPath, sqlite.OPEN_READWRITE);

    let customerId = await getCustomerId(particleUser, db).catch(function (err) {

        console.log(err);
        return -99;
    });

    if (!customerId || customerId <= 0) {

        var sql = util.format(
            "INSERT INTO CustomerSession(ParticleLogin, AccessToken, Token, RefreshToken, ExpiresIn) VALUES('%s','%s', '%s', '%s', %d)",
            particleUser,
            accessToken,
            token.token,
            token.refreshToken,
            token.expiresIn);

        var res = await executeSqlStatement(sql, db).catch(function (err) {

            console.error(err);
            return -99;
        });

        //  retrieve the customer id
        customerId = await getCustomerId(particleUser, db).catch(function (err) {

            console.log(err);
            return -99;
        });
    }

    db.close();
    return customerId;
};

/*
    Register the user with their main access token
*/
SqliteManager.prototype.registerToken = async function (token, customerId) {

    let db = new sqlite.Database(dbPath, sqlite.OPEN_READWRITE);

    //  see if user has an existing token
    var tokenDetail = await getCustomerTokenFromCustomerId(customerId, db).catch(function (err) {

        console.log(err);
        return false;
    });

    var sql;

    //  if token detail not found then it's an error
    if (!tokenDetail) {

        console.log("Customer not found in database");
        return false;
    }

    // replace the existing token
    sql = util.format("UPDATE CustomerSession SET Token='%s', RefreshToken='%s', ExpiresIn=%d WHERE CustomerId=%d", token.token, token.refreshToken, token.expiresIn, customerId);

    //  insert or update the token
    var res = await executeSqlStatement(sql, db).catch(function (err) {

        console.error(err);
        return false;
    });

    db.close();
    return true;
};

/*
    Get token datails from supplied token
*/
SqliteManager.prototype.getAuthToken = async function (token) {

    let db = new sqlite.Database(dbPath, sqlite.OPEN_READWRITE);

    //  see if user has an existing token
    var tokenDetail = await getCustomerToken(token, db).catch(function (err) {

        console.log(err);
        return false;
    });


    db.close();
    return tokenDetail;
};

/*
    Get token datails from supplied customer id
*/
SqliteManager.prototype.getAuthTokenFromCustomerId = async function (customerId) {

    let db = new sqlite.Database(dbPath, sqlite.OPEN_READWRITE);

    //  see if user has an existing token
    var tokenDetail = await getCustomerTokenFromCustomerId(customerId, db).catch(function (err) {

        console.log(err);
        return false;
    });


    db.close();
    return tokenDetail;
};

/*
    Get Customer Id
*/
function getCustomerId(particleUser, db) {

    let timeout = setTimeout(() => {

        console.log("getCustomerId Timed out ...");

    }, 5000);

    //  first get customer id
    return new Promise(function (resolve, reject) {

        db.get("SELECT CustomerId FROM CustomerSession WHERE ParticleLogin=?", [particleUser], (err, row) => {

            clearTimeout(timeout);

            if (err) {

                reject(new Error(err.message));
            }

            if (row) {

                resolve(row.CustomerId);
            }

            resolve(null);
        });
    });
};

/*
    Get Customer Token (if any) from the customer id
*/
function getCustomerTokenFromCustomerId(customerId, db) {

    return new Promise(function (resolve, reject) {

        db.get("SELECT Token, RefreshToken, ExpiresIn FROM CustomerSession WHERE CustomerId=?", [customerId], (err, row) => {

            if (err) {

                reject(new Error(err.message));
            }

            if (row) {

                resolve({
                    authToken: row.Token,
                    refreshToken: row.RefreshToken,
                    expiresAt: row.ExpiresIn
                });
            }

            resolve(null);
        });
    });
};

/*
    Get Customer Token (if any)
*/
function getCustomerToken(token, db) {

    return new Promise(function (resolve, reject) {

        db.get("SELECT CustomerId, RefreshToken, ExpiresIn FROM CustomerSession WHERE Token=?", [token], (err, row) => {

            if (err) {

                reject(new Error(err.message));
            }

            if (row) {

                resolve({
                    authToken: token,
                    customerId: row.CustomerId,
                    refreshToken: row.RefreshToken,
                    expiresIn: row.ExpiresIn
                });

                return;
            }

            resolve(null);
        });
    });
};

/*
    Execute an insert or update statement
*/
function executeSqlStatement(sqlStatement, db) {

    let timeout = setTimeout(() => {

        reject(new Error("executeSqlStatement Timed out ..."));

    }, 5000);

    //  first get customer id
    return new Promise(function (resolve, reject) {

        db.run(sqlStatement, function (err) {

            clearTimeout(timeout);

            if (err) {

                reject(new Error(err.message));
            }

            resolve(1);
        });
    });
}

module.exports = SqliteManager;

