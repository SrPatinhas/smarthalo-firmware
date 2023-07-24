var Promise = require('bluebird');
var shoe = require('shoe');
var remotepromise = require('remotepromise');
var clifw = require('./clifw');

var admin = require('./admin');

var clipromises = {};

var cliptr;

var proxy = function (name) {
	return function () {
		if(cliptr && cliptr[name]) {
			return cliptr[name].apply(this, arguments);
		}
	}
}

function clifill(fns) {
	for(var i in fns) {
		if(!clipromises[i]) {
			clipromises[i] = proxy(i);
		}
	}
}

clifill(admin.clifns);

$(function () {
    var stream = shoe('/rp');

	stream.on('end', function () {
		document.location.reload(true);
	})

	remotepromise.instantiate(stream, clipromises)
	.then((remote) => {
	    return Promise.resolve()
	    .then(() => remote.getSessionType())
	    .then((sessionType) => {
			$('div#logged').show();

			if(sessionType === 'admin') {
				cliptr = admin.clifns;
				return admin.entry(remote);
			}
	    })
	    .error((error) => {
	        if(error instanceof remotepromise.CloseStreamError) {
	            console.log("Stream Closed.");
	        }
	        else throw error;
	    })

	})
	.catch((error) =>  console.log('rpcatch', error.stack || error))

	clifw.nav_init();
});



