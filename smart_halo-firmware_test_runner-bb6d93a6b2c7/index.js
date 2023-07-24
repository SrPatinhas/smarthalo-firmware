
var Promise = require("bluebird");
var http = require('http');

var shoe = require('shoe');
var ecstatic = require('ecstatic');

var fs = require('fs');
fs.readFileAsync = fs.readFileAsync || Promise.promisify(fs.readFile);
fs.writeFileAsync = fs.writeFileAsync || Promise.promisify(fs.writeFile);
fs.appendFileAsync = fs.appendFileAsync || Promise.promisify(fs.appendFile);

var bleClient = require("smarthalo-node-ble-client/bleClient")
var devices = bleClient.devices

//******************************************************************************
// Web API
//******************************************************************************
var connect = require('connect')

var app = connect();

var bodyParser = require('body-parser');
app.use(bodyParser.json());

app.use(ecstatic({ root: __dirname + '/public' }));

app.use('/do', function (req, res) {
	var reqs = req.url.split("/");
	var action = reqs[1];
	reqs.splice(0, 2);
	var args = reqs;
	Promise.resolve()
	.then(function (obj) {
		res.writeHead(200, {"Content-Type": "application/json"});
		res.end(JSON.stringify({success:1,result:obj}));
	})
	.catch(function (err) {
		console.log(err);
		res.writeHead(500, {"Content-Type": "application/json"});
		res.end(JSON.stringify({
			failed:1,
			msg:(err||{}).message || err || "Internal server error"
		}));
	})
});
var port = +process.env.WEB_PORT || 3080;

//******************************************************************************
// Remote Promises
//******************************************************************************

var args = process.argv.slice(2);
var sessionType = args[0];
if(sessionType == 'admin') {
	bleClient.initializeChangeListeners()
}
var remotepromise = require('remotepromise');

var sock = shoe(function (stream) {
	var publish = function () {}
	var onDisconnect = function () {}

	var promises = {
		getSessionType : () => sessionType,
		connect : async (mac) => {
			var device = await bleClient.connectWithMac(mac)
			if(device) {
			    device.instance.registerNotify(function (data) {
			    	publish(mac, data);
			    })
			    device._peripheral.on('disconnect', function() {
			    	onDisconnect(mac);
			    });
			}
		},
		disconnect : (mac) => bleClient.disconnectFromMac(mac),
		sendCommand : bleClient.sendGenericCommandFromMac,
	}

	remotepromise.instantiate(stream, promises)
	.then(function (remote) {
		onDisconnect = remote.onDisconnect;
		publish = remote.publish;
		bleClient.updateClientList = function (devices) {
			var list = [];
			for(var i in devices) {
				list.push({
					'_id':i,
					'mac':i,
					'id':devices[i].id
				});
			}
			remote.devices({'body':list});
		}
	})
	.delay(500)
	.then(function () {
		bleClient.updateClientList(devices);
	})

	stream.on('close', function () {
		bleClient.updateClientList = function () {}
	})

});

//******************************************************************************
// Process Entry point
//******************************************************************************

process.on('SIGINT', function() {
	process.exit();
});

Promise.resolve()
.then(() => {
	var server = http.createServer(app);
	server.listen(port, function () {
		sock.install(server, '/rp');
		var host = server.address().address;
		var port = server.address().port;
		console.log('listening at http://%s:%s', host, port);
	});
})
.catch(function (err) {
	console.log(err.stack || err);
})
