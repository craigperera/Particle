
var express = require('express');
var app = express();
var bodyParser = require('body-parser');
var session = require('express-session');
var SQLiteStore = require('connect-sqlite3')(session);

const ngrok = require('ngrok');
const authProvider = require('./management/auth-manager');
const actionsManager = require('./management/actions-manager');

// parse incoming requests
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: false }));

//use sessions for tracking logins
app.use(session({
    secret: '1PHEcXZ+l0y93hMrv/5yKw==',
    resave: true,
    saveUninitialized: false,
    store: new SQLiteStore
}));

// serve static files from template
app.use(express.static(__dirname + '/pages'));

// include routes
var routes = require('./routing/router');
app.use('/', routes);

const appPort = process.env.PORT || 3000;
const server = app.listen(appPort, function () {

    const host = server.address().address;
    const port = server.address().port;

    ngrok.connect({
        addr: appPort,
        subdomain: "mutleysoftware",
        region: 'eu'
    }, function (err, url) {
        if (err) {
            console.log('ngrok err', err);
            process.exit();
        }

        console.log("|###################################################|");
        console.log("|                                                   |");
        console.log("|        COPY & PASTE NGROK URL BELOW:              |");
        console.log("|                                                   |");
        console.log("|     " + url + "            |");
        console.log("|                                                   |");
        console.log("|###################################################|");
    });
});

authProvider.registerAuth(app);