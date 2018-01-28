const AuthManager = {};
const express = require('express');
const datastoreManager = require('./datastore-manager');
const DatastoreManager = new datastoreManager();

const CID = "GyQKPt4Wak62N26xwhlk4A";
const CLS = "KyfxreIE6kGAMdeVopXYNw";

AuthManager.registerAuth = function (app) {

    app.use('/login', express.static('./pages/index.html'));
}

/*
    Stores the customer details in the datastore and returns the new customer Id
    OR if they exist return the customer id
*/
AuthManager.saveCustomerDetail = async function (particleLogin, token) {

    var customerId = await DatastoreManager.getCustomerId(particleLogin, token);
    return customerId;
}

AuthManager.getCustomerFromToken = async function(auth_token) {

    var result = await DatastoreManager.getCustomerFromToken(auth_token);
    return result;
}

AuthManager.getCustomerFromId = async function(customerId) {

    var result = await DatastoreManager.getTokenFromCustomerId(customerId);
    return result;
}

/*
  Ensure request is from Google
*/
AuthManager.IsGoogleRequestOAuth = function (clientId, redirectUri) {

    if (clientId != CID) {

        return false;
    }

    if (!redirectUri.endsWith("particlecontroller")) {

        return false;
    }

    return true;
};

/*
  Ensure request is from Google
*/
AuthManager.IsGoogleRequest = function (clientId, clientSecret, redirectUri) {

    if (clientId != CID) {

        return false;
    }

    if (clientSecret != CLS) {

        return false;
    }

    if (!redirectUri.endsWith("particlecontroller")) {

        return false;
    }

    return true;
}

/*
    Handle the authorisation token
*/
AuthManager.HandleAuthToken = async function (tokenData, res) {

    if (!tokenData || !tokenData.authToken || !tokenData.refreshToken) {

        return false;
    }

    let authCode = {
        token_type: "bearer",
        access_token: tokenData.authToken,
        refresh_token: tokenData.refreshToken
    };

    var expiresAt = new Date(Date.now() + (tokenData.expiresIn * 1000));

    if (new Date(expiresAt) < Date.now()) {

        console.error('expired code');
        return res.status(400).send('expired code');
    }

    return res.status(200).json(authCode);
};

/*
    Extract the access token from the url
*/
AuthManager.ExtractAccessToken = async function (request) {

    var auth_token = request.headers.authorization ? request.headers.authorization.split(' ')[1] : null;

    if (!auth_token || auth_token == null) {

        return null;
    }

    var res = await DatastoreManager.getCustomerFromToken(auth_token);
    return res;
};

exports.registerAuth = AuthManager.registerAuth;
exports.checkOAuth = AuthManager.IsGoogleRequestOAuth;
exports.checkRequest = AuthManager.IsGoogleRequest;
exports.checkAuthToken = AuthManager.HandleAuthToken;
exports.getAccessToken = AuthManager.ExtractAccessToken;
exports.saveCustomerDetail = AuthManager.saveCustomerDetail;
exports.getCustomerFromToken = AuthManager.getCustomerFromToken;
exports.getCustomerFromId = AuthManager.getCustomerFromId;