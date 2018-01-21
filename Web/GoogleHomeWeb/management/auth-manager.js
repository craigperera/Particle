const AuthManager = {};
const express = require('express');

const CID = "GyQKPt4Wak62N26xwhlk4A";
const CLS = "KyfxreIE6kGAMdeVopXYNw";

AuthManager.registerAuth = function (app) {

    app.use('/login', express.static('./pages/index.html'));
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
AuthManager.ExtractAccessToken = function (request) {

    return request.headers.authorization ? request.headers.authorization.split(' ')[1] : null;
  };
  

exports.registerAuth = AuthManager.registerAuth;
exports.checkOAuth = AuthManager.IsGoogleRequestOAuth;
exports.checkRequest = AuthManager.IsGoogleRequest;
exports.checkAuthToken = AuthManager.HandleAuthToken;
exports.getAccessToken = AuthManager.ExtractAccessToken;