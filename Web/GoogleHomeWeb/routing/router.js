const util = require('util');
const express = require('express');
const session = require('express-session');
const router = express.Router();

const authProvider = require('../management/auth-manager');
const ActionsManager = require('../management/actions-manager');
const ParticleHelper = require('../management/particle-manager');
const sqllite = require('../management/sqlite-manager');

const ParticleManager = new ParticleHelper();
const SqliteManager = new sqllite('GoogleIntegration.db');
const ActionManager = new ActionsManager();

/*
    /oauth implementation, will be called from Google Home
*/
router.get('/oauth', function (req, res) {

    let client_id = req.query.client_id;
    let redirect_uri = req.query.redirect_uri;
    let state = req.query.state;
    let response_type = req.query.response_type;
    let authCode = req.query.code;

    if ('code' != response_type) {

        return res.status(500).send('response_type ' + response_type + ' must equal "code"');
    }

    if (!authProvider.checkOAuth(client_id, redirect_uri)) {

        return res.status(500).send('Not a request from Google Actions');
    }

    // if you have an authcode use that
    if (authCode) {
        return res.redirect(util.format('%s?code=%s&state=%s',
            redirect_uri, authCode, state
        ));
    }

    let user = req.session.user;

    // Redirect anonymous users to login page.
    if (!user) {
        return res.redirect(util.format('/?client_id=%s&redirect_uri=%s&redirect=%s&state=%s',
            client_id, encodeURIComponent(redirect_uri), req.path, state));
    }

    authCode = SmartHomeModel.getAuthCodeFromUid(user.uid);

    if (authCode) {

        return res.redirect(util.format('%s?code=%s&state=%s', redirect_uri, authCode));
    }

    return res.status(400).send('something went wrong');
});

router.all('/token', async function (req, res) {

    let client_id = req.query.client_id ? req.query.client_id : req.body.client_id;
    let client_secret = req.query.client_secret ? req.query.client_secret : req.body.client_secret;
    let grant_type = req.query.grant_type ? req.query.grant_type : req.body.grant_type;
    let auth_token = req.query.code ? req.quey.code : req.body.code;
    let redirect_uri = req.query.redirect_uri ? req.quey.redirect_uri : req.body.redirect_uri;

    if (!client_id || !client_secret || !auth_token || !redirect_uri) {

        return res.status(400).send('missing required parameter');
    }

    //  validate that the request is from google
    if (!authProvider.checkRequest(client_id, client_secret, redirect_uri)) {

        return res.status(400).send('Response is not from Google Actions');
    }

    let tokenDetail = await SqliteManager.getAuthToken(auth_token);

    if (!tokenDetail || !tokenDetail.customerId || tokenDetail.customerId <= 0) {

        return res.status(400).send('incorrect client data');
    }

    if ('authorization_code' == grant_type) {

        return authProvider.checkAuthToken(tokenDetail, res);
    }
    else if ('refresh_token' == grant_type) {

        return handleRefreshToken(req, res);
    }
    else {

        console.error('grant_type ' + grant_type + ' is not supported');
        return res.status(400).send('grant_type ' + grant_type + ' is not supported');
    }
});

router.post('/login', async function (req, res) {

    var redirectUri = req.body.redirect_uri;
    var state = req.body.state;
    var userName = req.body.inp_name;
    var password = req.body.inp_pwd;
    var token;

    //  if details not found re-login
    if (!userName || userName.length == 0 || !password || password.length == 0) {

        return res.redirect('/login');
    }

    var data;

    //  handle special user for submission
    if (userName.toLowerCase() == "test44@gmail.com") {

        data = await ParticleManager.getAuthToken('craiginternet@me.com', 'silkcut25');
    }
    else {

        //  check particle login
        data = await ParticleManager.getAuthToken(userName, password);
    }

    if (!data || !data.authToken || !data.customerId) {

        let path = decodeURIComponent(req.body.redirect) || '/frontend';

        return res.redirect(util.format('%s?client_id=%s&redirect_uri=%s&state=%s&response_type=code', path, req.body.client_id, req.body.redirect_uri, req.body.state));
    }

    return res.redirect(util.format('%s?code=%s&state=%s', decodeURIComponent(req.body.redirect_uri), data.authToken, req.body.state));
});

/**
 * Enables prelight (OPTIONS) requests made cross-domain.
 */
router.options('/particlise', function (request, response) {
    response.status(200).set({
        'Access-Control-Allow-Origin': '*',
        'Access-Control-Allow-Headers': 'Content-Type, Authorization'
    }).send('null');
});

router.post('/particlise', async function (request, response) {

    let reqdata = request.body;

    let authToken = authProvider.getAccessToken(request);
    let tokenData = await SqliteManager.getAuthToken(authToken);

    if (!tokenData || !tokenData.customerId || tokenData.customerId <= 0) {

        response.status(401).set({
            'Access-Control-Allow-Origin': '*',
            'Access-Control-Allow-Headers': 'Content-Type, Authorization'
        }).json({ error: "Customer not found for token" });
    }

    if (!reqdata.inputs) {

        response.status(401).set({
            'Access-Control-Allow-Origin': '*',
            'Access-Control-Allow-Headers': 'Content-Type, Authorization'
        }).json({ error: "missing inputs" });
    }

    for (let i = 0; i < reqdata.inputs.length; i++) {

        let input = reqdata.inputs[i];
        let intent = input.intent;

        if (!intent) {

            response.status(401).set({
                'Access-Control-Allow-Origin': '*',
                'Access-Control-Allow-Headers': 'Content-Type, Authorization'
            }).json({ error: "missing inputs" });

            continue;
        }

        switch (intent) {

            case "action.devices.SYNC": {

                ActionManager.HandleSync({
                    uid: tokenData.customerId,
                    auth: tokenData.authToken,
                    requestId: reqdata.requestId
                }, response);

                break;
            }
            case "action.devices.QUERY": {

                ActionManager.HandleQuery({
                    uid: tokenData.customerId,
                    auth: tokenData.authToken,
                    requestId: reqdata.requestId,
                    devices: reqdata.inputs[0].payload.devices
                }, response);

                break;
            }
            case "action.devices.EXECUTE": {

                exec({
                    uid: tokenData.customerId,
                    auth: tokenData.authToken,
                    requestId: reqdata.requestId,
                    commands: reqdata.inputs[0].payload.commands
                }, response);

                break;
            }
            default:
                response.status(401).set({
                    'Access-Control-Allow-Origin': '*',
                    'Access-Control-Allow-Headers': 'Content-Type, Authorization'
                }).json({ error: "missing intent" });
                break;
        }
    }
});

module.exports = router;