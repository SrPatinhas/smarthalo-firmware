var Promise = require('bluebird');
var sha256 = require('js-sha256').sha256;

var clifw = require('./clifw');

function HSVparse(text) {
	var re = /hsv\(\s*(\d*(?:\.\d+)?)\s*,\s*(\d*(?:\.\d+)?)\s*,\s*(\d*(?:\.\d+)?)\s*?\)/;
 	var match = re.exec(text);
	return {
        h: match[1],
        s: match[2],
        v: match[3]
    }; 	
}

exports.clifns = {
	test : function (msg) {
		console.log(msg);
	},
	onDisconnect : function (mac) {
		$('body').trigger('admin.disconnect', [mac]);
	},
    devices : function (devices) {
		$('body').trigger('admin.devicelist', [devices]);
    },
    publish : function (mac, msg) {
        console.log(mac + ': ' + JSON.stringify(msg));
    }
}


exports.entry = function (remote) {

	function deviceView(mac) {
		var id = clifw.frame_create({'label':mac});
		var container = "#content div#frame_"+id;
		$(container).append($("core-templates > core-template#deviceView").clone());
		$('div#nav li#nav_' + id + ' a:first-child').trigger('click');	

		$('.colorpicker-component', container).colorpicker({
            format: 'hsv'
        });

		var connected = false;
		$('button#connect', container).on('click', function () {
			connected = true;
			$('button#connect', container).hide();
			$('button#disconnect', container).show();
			$('span#connect_msg', container).html("");
			remote.connect(mac)
			.then(function () {
				$('span#connect_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#connect_msg', container).html("Error!");
				console.log(err);
			})
		});
		$('button#disconnect', container).on('click', function () {
			connected = false;
			$('button#connect', container).show();
			$('button#disconnect', container).hide();
			$('span#connect_msg', container).html("");
			remote.disconnect(mac)
			.then(function () {
				$('span#connect_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#connect_msg', container).html("Error!");
				console.log(err);
			})
		});
		$('button#disconnect', container).hide();

		$('body').on('admin.disconnect', function (e, evmac) {
			if(evmac == mac) {
				connected = false;
				$('button#connect', container).show();
				$('button#disconnect', container).hide();
				$('span#connect_msg', container).html("");
			}
		});

		$(container).on('close', function () {
			if(connected) {
				remote.dev_disconnect(mac);
			}
		});

		$('button#auth_getVersions', container).on('click', function () {
			$('span#auth_getVersions_msg', container).html("");
			remote.sendCommand(mac, 'auth_getVersions')
			.then(function (versions) {
				var lst = [];
				if(versions.stmFirmware && versions.stmBootloader) {
					lst.push("<b>STM Firmware</b> : " + versions.stmFirmware)
					lst.push("<b>STM Bootloader</b> : " + versions.stmBootloader)
				}
				lst.push("<b>Nordic Firmware</b> : " + versions.nordicFirmware)
				lst.push("<b>Nordic Bootloader</b> : " + versions.nordicBootloader)
				if(versions.hardwareVersion) { 
					lst.push("<b>Hardware Version</b> : " + versions.hardwareVersion)
				}
				if(versions.metersVersion) { 
					lst.push("<b>Meters Version</b> : "+versions.metersVersion)
				}

				$('span#auth_getVersions_msg', container).html(lst.join(", "));
			})
			.catch(function (err) {
				$('span#auth_getVersions_msg', container).html("Error!");
				console.log(err);
			})
		});

		$('button#authenticate', container).on('click', function () {
			var val = $('input#devpass', container).val();
			var hash = (val) ? sha256(val) : "";
			console.log("authenticate", hash)
			$('span#authenticate_msg', container).html("");
			remote.sendCommand(mac, 'authenticate', hash)
			.then(function () {
				$('span#authenticate_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#authenticate_msg', container).html("Error!");
				console.log(err);
			})
		});

		$('button#auth_demo', container).on('click', function () {
			$('span#auth_demo_msg', container).html("");
			console.log("auth_demo")
			remote.sendCommand(mac, 'auth_demo')
			.then(function () {
				$('span#auth_demo_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#auth_demo_msg', container).html("Error!");
				console.log(err);
			})
		});

		$('button#setPass', container).on('click', function () {
			var val = $('input#newpass', container).val();
			var hash = (val) ? sha256(val) : "";
			$('span#setPass_msg', container).html("");
			remote.sendCommand(mac, 'auth_setPassword', hash)
			.then(function () {
				$('span#setPass_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#setPass_msg', container).html("Error!");
				console.log(err);
			})
		});

		$('button#resetPassword', container).on('click', function () {
			var secretKey = $('input#resetPasswordSecretKey', container).val();
			$('span#resetPassword_msg', container).html("");
			if(!secretKey || secretKey.length === 0){
				$('span#resetPassword_msg', container).html("Please provide a secret key to reset the password");
				return
			}
			remote.sendCommand(mac, 'auth_resetPassword', secretKey.split(','))
			.then(function () {
				$('span#resetPassword_msg', container).html("Ok! Power cycle your device to complete reset");
			})
			.catch(function (err) {
				$('span#resetPassword_msg', container).html("Error!");
				console.log(err);
			})
		});


		$('button#ui_brightness', container).on('click', function () {
			var brightness = parseInt($('input#ui_brightness', container).val(), 10);
			if(isNaN(brightness) || brightness > 100 || brightness < 0) {
				$('span#ui_brightness_msg', container).html("Brightness must be in range 0 - 100");
				return;
			}
			$('span#ui_brightness_msg', container).html("");
			remote.sendCommand(mac, 'ui_setBrightness', brightness)
			.then(function () {
				$('span#ui_brightness_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#ui_brightness_msg', container).html("Error!");
				console.log(err);
			})
		});

		$('button#cmd_generic', container).on('click', function () {
			var rawPayload = $('input#cmd_generic_payload', container).val()
			var payloadArray = (rawPayload.trim()).split(',')
			var incorrecValue = payloadArray.find(value => value < 0 || value > 255)
			console.log(incorrecValue)
			if(incorrecValue) {
				$('span#cmd_generic_msg', container).html("Error : Payload values must be in range 0 - 255")
				return
			}
			if(payloadArray > 80) {
				$('span#cmd_generic_msg', container).html("Error : You can only send up to 80 bytes")
				return
			}

			$('span#cmd_generic_msg', container).html("")
			remote.sendCommand(mac, 'test_generic', payloadArray)
			.then(function (value) {
				$('span#cmd_generic_msg', container).html('OK! Results : ' + value)
			})
			.catch(function (err) {
				$('span#cmd_generic_msg', container).html("Error!")
				console.log(err)
			})
		});


		$('button#ui_logo', container).on('click', function () {
			$('span#ui_logo_msg', container).html("");
			remote.sendCommand(mac, 'ui_logo')
			.then(function () {
				$('span#ui_logo_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#ui_logo_msg', container).html("Error!");
				console.log(err);
			})
		});

		$('button#ui_disconnect', container).on('click', function () {
			$('span#ui_disconnect_msg', container).html("");
			remote.sendCommand(mac, 'ui_disconnect')
			.then(function () {
				$('span#ui_disconnect_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#ui_disconnect_msg', container).html("Error!");
				console.log(err);
			})
		});

		$('button#ui_nav', container).on('click', function () {
			var color = HSVparse($('#nav_color', container).data('colorpicker').input.val());
			var dir = parseInt($('select#nav_direction', container).val(), 10);
			var progress = parseInt($('input#nav_progress', container).val(), 10);
			if(isNaN(progress) || progress > 100 || progress < 0) {
				$('span#ui_nav_msg', container).html("Progress must be in range 0 - 100");
				return;
			}
			$('span#ui_nav_msg', container).html("");
			remote.sendCommand(mac, 'ui_nav', color.h,color.s,color.v, dir, progress)
			.then(function () {
				$('span#ui_nav_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#ui_nav_msg', container).html("Error!");
				console.log(err);
			})
		});

		$('button#ui_nav_angle', container).on('click', function () {
			//var legacytest = $('#ui_nav_angle_legacytest', container).prop('checked');
			var color = HSVparse($('#nav_angle_color', container).data('colorpicker').input.val());
			var heading = parseInt($('input#nav_angle_heading', container).val(), 10);
			if(isNaN(heading) || heading < -180 || heading > 180) {
				$('span#ui_nav_angle_msg', container).html("bad heading");
				return;
			}
			var progress = parseInt($('input#nav_angle_progress', container).val(), 10);
			if(isNaN(progress) || progress > 100 || progress < 0) {
				$('span#ui_nav_angle_msg', container).html("Progress must be in range 0 - 100");
				return;
			}
            var colorNextDir = HSVparse($('#nav_angle_color_next_direction', container).data('colorpicker').input.val());
            var nextdirection = parseInt($('input#nav_angle_nextdirection', container).val(), 10);
			if(isNaN(nextdirection) || nextdirection < -180 || nextdirection > 180) {
				$('span#ui_nav_angle_msg', container).html("bad heading for next direction");
				return;
			}
            var progress_next_direction = parseInt($('input#nav_angle_progress_next_direction', container).val(), 10);
            var compass_mode = parseInt($('select#nav_angle_compass_mode', container).val(), 10);
            var cmd = (compass_mode) ? "exp_ui_nav_angle_2" : "ui_nav_angle";
			$('span#ui_nav_angle_msg', container).html("");
			remote.sendCommand(mac, cmd, color.h,color.s,color.v, heading, progress, colorNextDir.h, colorNextDir.s, colorNextDir.v, nextdirection, progress_next_direction, compass_mode)
			.then(function () {
				$('span#ui_nav_angle_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#ui_nav_angle_msg', container).html("Error!");
				console.log(err);
			})
		});

		$('button#exp_ui_nav_destination_angle', container).on('click', function () {
			var heading = parseInt($('input#nav_destination_angle_heading', container).val(), 10);
			if(isNaN(heading) || heading < -180 || heading > 180) {
				$('span#exp_ui_nav_destination_angle_msg', container).html("bad heading");
				return;
			}
			var progress = parseInt($('input#nav_destination_angle_progress', container).val(), 10);
			if(isNaN(progress) || progress < 0 || progress > 100) {
				$('span#exp_ui_nav_destination_angle_msg', container).html("bad progress");
				return;
			}
			var color = HSVparse($('#nav_destination_angle_color', container).data('colorpicker').input.val());
			$('span#exp_ui_destination_angle_msg', container).html("");
			remote.sendCommand(mac, 'exp_ui_nav_destination_angle', color.h,color.s,color.v, heading, progress)
			.then(function () {
				$('span#exp_ui_nav_destination_angle_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#exp_ui_nav_destination_angle_msg', container).html("Error!");
				console.log(err);
			})
		});

		$('button#ui_nav_reroute', container).on('click', function () {
			$('span#ui_nav_reroute_msg', container).html("");
			remote.sendCommand(mac, 'ui_nav_reroute')
			.then(function () {
				$('span#ui_nav_reroute_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#ui_nav_reroute_msg', container).html("Error!");
				console.log(err);
			})
		});

		$('button#ui_nav_off', container).on('click', function () {
			$('span#ui_nav_off_msg', container).html("");
			remote.sendCommand(mac, 'ui_nav_off')
			.then(function () {
				$('span#ui_nav_off_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#ui_nav_off_msg', container).html("Error!");
				console.log(err);
			})
		});

        $('button#ui_nav_roundAbout', container).on('click', function () {
            var exitNow = parseInt($('select#nav_roundAbout_exitNow', container).val(), 10);
            var driveRight = parseInt($('select#nav_roundAbout_driveRight', container).val(), 10);
            var exits = $('input#nav_roundAbout_angle_exits', container).val();
            var color1 = HSVparse($('#roundAbout_color1', container).data('colorpicker').input.val());
			var color2 = HSVparse($('#roundAbout_color2', container).data('colorpicker').input.val());
			$('span#ui_nav_roundAbout_msg', container).html("");
			remote.sendCommand(mac, 'ui_nav_roundAbout', exitNow, driveRight, color1.h, color1.s, color1.v, color2.h, color2.s, color2.v, exits)
			.then(function () {
				$('span#ui_nav_roundAbout_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#ui_nav_roundAbout_msg', container).html("Error!");
				console.log(err);
			})
		});

		$('button#ui_turn_by_turn_intro', container).on('click', function () {
			remote.sendCommand(mac, 'ui_turn_by_turn_intro')
			.then(function () {
				$('span#exp_ui_turn_by_turn_intro_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#exp_ui_turn_by_turn_intro_msg', container).html("Error!");
				console.log(err);
			})
		});

		$('button#ui_frontlight_settings', container).on('click', function () {
			var mode = parseInt($('select#front_mode', container).val(), 10);
			var brightness = parseInt($('input#front_brightness', container).val(), 10);
			if(isNaN(brightness) || brightness > 100 || brightness < 0) {
				$('span#ui_frontlight_settings_msg', container).html("Brightness must be in range 0 - 100");
				return;
			}
			var blinkLock = parseInt($('select#blink_lock', container).val(), 10);
			$('span#ui_frontlight_settings_msg', container).html("");
			remote.sendCommand(mac, 'ui_frontlight_settings', mode, brightness, blinkLock)
			.then(function () {
				$('span#ui_frontlight_settings_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#ui_frontlight_settings_msg', container).html("Error!");
				console.log(err);
			})
		});

		$('button#ui_frontlight_on', container).on('click', function () {
			$('span#ui_frontlight_msg', container).html("");
			remote.sendCommand(mac, 'ui_frontlight', 1, false, false)
			.then(function () {
				$('span#ui_frontlight_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#ui_frontlight_msg', container).html("Error!");
				console.log(err);
			})
		});

		$('button#ui_frontlight_onbg', container).on('click', function () {
			$('span#ui_frontlight_msg', container).html("");
			remote.sendCommand(mac, 'ui_frontlight', 1, true, false)
			.then(function () {
				$('span#ui_frontlight_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#ui_frontlight_msg', container).html("Error!");
				console.log(err);
			})
		});

		$('button#ui_frontlight_noMovement', container).on('click', function () {
			$('span#ui_frontlight_msg', container).html("");
			remote.sendCommand(mac, 'ui_frontlight', 1, false, true)
			.then(function () {
				$('span#ui_frontlight_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#ui_frontlight_msg', container).html("Error!");
				console.log(err);
			})
		});

		$('button#ui_frontlight_off', container).on('click', function () {
			$('span#ui_frontlight_msg', container).html("");
			remote.sendCommand(mac, 'ui_frontlight', 0, false, false)
			.then(function () {
				$('span#ui_frontlight_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#ui_frontlight_msg', container).html("Error!");
				console.log(err);
			})
		});

		$('button#ui_light_external_toggle', container).on('click', function () {
			var isRequired = parseInt($('select#ui_light_external_mode_select', container).val(), 10);
			var cmd = "ui_light_external_toggle";
			$('span#ui_light_external_toggle_msg', container).html("");
			remote.sendCommand(mac,cmd,isRequired)
			.then(function () {
				$('span#ui_light_external_toggle_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#ui_light_external_toggle_msg', container).html("Error!");
				console.log(err);
			})
		});

		$('button#ui_progress', container).on('click', function () {
            var mode = parseInt($('select#progress_mode', container).val(), 10);
			var cycle = parseInt($('input#progress_cycle', container).val(), 10);
			var percent = parseInt($('input#progress_percent', container).val(), 10);
			if(isNaN(percent) || percent > 100 || percent < 0) {
				$('span#ui_progress_msg', container).html("percent must be in range 0 - 100");
				return;
			}
			if(isNaN(cycle)) {
				$('span#ui_progress_msg', container).html("cycle must be a number");
				return;
			}
			var color1 = HSVparse($('#progress_color1', container).data('colorpicker').input.val());
			var color2 = HSVparse($('#progress_color2', container).data('colorpicker').input.val());
			$('span#ui_progress_msg', container).html("");
			remote.sendCommand(mac, 'ui_progress', color1.h,color1.s,color1.v,color2.h,color2.s,color2.v, cycle,percent,mode)
			.then(function () {
				$('span#ui_progress_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#ui_progress_msg', container).html("Error!");
				console.log(err);
			})
		});

        $('button#ui_progress_intro', container).on('click', function () {
            var cycle = parseInt($('input#progress_cycle', container).val(), 10);
            if(isNaN(cycle)) {
				$('span#ui_progress_msg', container).html("cycle must be a number");
				return;
			}
			var color1 = HSVparse($('#progress_color1', container).data('colorpicker').input.val());
			var color2 = HSVparse($('#progress_color2', container).data('colorpicker').input.val());
			$('span#ui_progress_intro_msg', container).html("");
			remote.sendCommand(mac, 'ui_progress_intro',color1.h,color1.s,color1.v,color2.h,color2.s,color2.v, cycle)
			.then(function () {
				$('span#ui_progress_intro_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#ui_progress_intro_msg', container).html("Error!");
				console.log(err);
			})
		});

		$('button#ui_progress_off', container).on('click', function () {
			$('span#ui_progress_off_msg', container).html("");
			remote.sendCommand(mac, 'ui_progress_off')
			.then(function () {
				$('span#ui_progress_off_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#ui_progress_off_msg', container).html("Error!");
				console.log(err);
			})
		});

		$('button#ui_speedometer', container).on('click', function () {
			var percent = parseInt($('input#speedometer_percent', container).val(), 10);
			if(isNaN(percent) || percent > 100 || percent < 0) {
				$('span#ui_speedometer_msg', container).html("percent must be in range 0 - 100");
				return;
			}
			$('span#ui_speedometer_msg', container).html("");
			remote.sendCommand(mac, 'ui_speedometer', percent)
			.then(function () {
				$('span#ui_speedometer_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#ui_speedometer_msg', container).html("Error!");
				console.log(err);
			})
		});

        $('button#ui_speedometer_intro', container).on('click', function () {
			$('span#ui_speedometer_intro_msg', container).html("");
			remote.sendCommand(mac, 'ui_speedometer_intro')
			.then(function () {
				$('span#ui_speedometer_intro_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#ui_speedometer_intro_msg', container).html("Error!");
				console.log(err);
			})
		});

		$('button#ui_speedometer_off', container).on('click', function () {
			$('span#ui_speedometer_off_msg', container).html("");
			remote.sendCommand(mac, 'ui_speedometer_off')
			.then(function () {
				$('span#ui_speedometer_off_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#ui_speedometer_off_msg', container).html("Error!");
				console.log(err);
			})
		});

        $('button#exp_ui_cadence', container).on('click', function () {
			var percent = parseInt($('input#cadence_percent', container).val(), 10);
            var color1 = HSVparse($('#cadence_color1', container).data('colorpicker').input.val());
            var color2 = HSVparse($('#cadence_color2', container).data('colorpicker').input.val());
			if(isNaN(percent) || percent > 100 || percent < -100) {
				$('span#exp_ui_cadence_msg', container).html("percent must be in range -100 - 100");
				return;
			}
			$('span#exp_ui_cadence_msg', container).html("");
			remote.sendCommand(mac, 'exp_ui_cadence', color1.h, color1.s, color1.v, color2.h, color2.s, color2.v, percent)
			.then(function () {
				$('span#exp_ui_cadence_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#exp_ui_cadence_msg', container).html("Error!");
				console.log(err);
			})
		});

		$('button#exp_ui_cadence_off', container).on('click', function () {
			$('span#exp_ui_cadence_off_msg', container).html("");
			remote.sendCommand(mac, 'exp_ui_cadence_off')
			.then(function () {
				$('span#exp_ui_cadence_off_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#exp_ui_cadence_off_msg', container).html("Error!");
				console.log(err);
			})
		});

		$('button#ui_notif', container).on('click', function () {
			var fadein = parseInt($('input#notif_fadein', container).val(), 10);
			var on = parseInt($('input#notif_on', container).val(), 10);
			var fadeout = parseInt($('input#notif_fadeout', container).val(), 10);
			var off = parseInt($('input#notif_off', container).val(), 10);
			var repeat = parseInt($('input#notif_repeat', container).val(), 10);
			if(isNaN(fadein) || isNaN(fadeout) || isNaN(on) || isNaN(off) || isNaN(repeat)) {
				$('span#ui_notif_msg', container).html("all parameters must be a number");
				return;
			}
			var color1 = HSVparse($('#notif_color1', container).data('colorpicker').input.val());
			var color2 = HSVparse($('#notif_color2', container).data('colorpicker').input.val());
			$('span#ui_notif_msg', container).html("");
			remote.sendCommand(mac, 'ui_notif', color1.h,color1.s,color1.v,color2.h,color2.s,color2.v, fadein,on,fadeout,off,repeat)
			.then(function () {
				$('span#ui_notif_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#ui_notif_msg', container).html("Error!");
				console.log(err);
			})
		});

		$('button#ui_notif_off', container).on('click', function () {
			$('span#ui_notif_off_msg', container).html("");
			remote.sendCommand(mac, 'ui_notif_off')
			.then(function () {
				$('span#ui_notif_off_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#ui_notif_off_msg', container).html("Error!");
				console.log(err);
			})
		});

		$('button#ui_hb', container).on('click', function () {
			var fadein = parseInt($('input#hb_fadein', container).val(), 10);
			var on = parseInt($('input#hb_on', container).val(), 10);
			var fadeout = parseInt($('input#hb_fadeout', container).val(), 10);
			var off = parseInt($('input#hb_off', container).val(), 10);
			if(isNaN(fadein) || isNaN(fadeout) || isNaN(on) || isNaN(off)) {
				$('span#ui_hb_msg', container).html("all parameters must be a number");
				return;
			}
			var color1 = HSVparse($('#hb_color1', container).data('colorpicker').input.val());
			var color2 = HSVparse($('#hb_color2', container).data('colorpicker').input.val());
			$('span#ui_hb_msg', container).html("");
			remote.sendCommand(mac, 'ui_hb', color1.h,color1.s,color1.v,color2.h,color2.s,color2.v, fadein,on,fadeout,off)
			.then(function () {
				$('span#ui_hb_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#ui_hb_msg', container).html("Error!");
				console.log(err);
			})
		});

		$('button#ui_hb_off', container).on('click', function () {
			$('span#ui_hb_off_msg', container).html("");
			remote.sendCommand(mac, 'ui_hb_off')
			.then(function () {
				$('span#ui_hb_off_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#ui_hb_off_msg', container).html("Error!");
				console.log(err);
			})
		});

		$('button#ui_assist', container).on('click', function () {
			var fadein = parseInt($('input#assist_fadein', container).val(), 10);
			var on = parseInt($('input#assist_on', container).val(), 10);
			var fadeout = parseInt($('input#assist_fadeout', container).val(), 10);
			var off = parseInt($('input#assist_off', container).val(), 10);
			var speed = parseInt($('input#assist_speed', container).val(), 10);
			if(isNaN(fadein) || isNaN(fadeout) || isNaN(on) || isNaN(off) || isNaN(speed)) {
				$('span#ui_assist_msg', container).html("all parameters must be a number");
				return;
			}
			var color1 = HSVparse($('#assist_color1', container).data('colorpicker').input.val());
			var color2 = HSVparse($('#assist_color2', container).data('colorpicker').input.val());
			var flash = parseInt($('select#assist_flash', container).val(), 10);
			$('span#ui_assist_msg', container).html("");
			remote.sendCommand(mac, 'exp_ui_assist', color1.h,color1.s,color1.v,color2.h,color2.s,color2.v, fadein,on,fadeout,off,flash,speed)
			.then(function () {
				$('span#ui_assist_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#ui_assist_msg', container).html("Error!");
				console.log(err);
			})
		});

		$('button#ui_assist_off', container).on('click', function () {
			$('span#ui_assist_off_msg', container).html("");
			remote.sendCommand(mac, 'exp_ui_assist_off')
			.then(function () {
				$('span#ui_assist_off_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#ui_assist_off_msg', container).html("Error!");
				console.log(err);
			})
		});

		$('button#ui_notif_lowBat', container).on('click', function () {
			var volume = parseInt($('input#ui_notif_lowBat_volume', container).val(), 10);
		$('span#ui_notif_lowBat_msg', container).html("");
			remote.sendCommand(mac, 'ui_notif_lowBat',volume)
			.then(function () {
				$('span#ui_notif_lowBat_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#ui_notif_lowBat_msg', container).html("Error!");
				console.log(err);
			})
		});

		$('button#ui_state_of_charge', container).on('click', function () {
			$('span#ui_state_of_charge_msg', container).html("");
			remote.sendCommand(mac, 'ui_state_of_charge')
			.then(function () {
				$('span#ui_state_of_charge_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#ui_state_of_charge_msg', container).html("Error!");
				console.log(err);
			})
		});

		$('button#exp_ui_notif_state_of_charge', container).on('click', function () {
            remote.sendCommand(mac, 'exp_ui_notif_state_of_charge')
			.then(function () {
				$('span#exp_ui_notif_state_of_charge_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#exp_ui_notif_state_of_charge_msg', container).html("Error!");
				console.log(err);
			})
		});
		
		$('button#ui_clock', container).on('click', function () {
			var hour = parseInt($('input#ui_clock_hour', container).val(), 10);
			if(isNaN(hour) || hour < 0 || hour > 12) {
				$('span#ui_clock_msg', container).html("bad hour");
				return;
			}
			var colorHour = HSVparse($('#ui_clock_hour_color', container).data('colorpicker').input.val());
			var minute = parseInt($('input#ui_clock_minute', container).val(), 10);
			var colorMinute = HSVparse($('#ui_clock_minute_color', container).data('colorpicker').input.val());
            if(isNaN(minute) || minute < 0 || minute > 59) {
				$('span#ui_clock_msg', container).html("bad minute");
				return;
			}
			var direction = parseInt($('select#ui_clock_duration', container).val(), 10);
			var duration = parseInt($('input#ui_clock_duration', container).val(), 10);
			if(isNaN(duration) || duration < 0 || duration > 65536) {
				$('span#ui_clock_msg', container).html("bad duration");
				return;
			}

            var fade = parseInt($('select#ui_clock_fade', container).val(), 10);
            var intro = parseInt($('select#ui_clock_intro', container).val(), 10);
            var pulse = parseInt($('select#ui_clock_pulse', container).val(), 10);
            var cmd = "ui_clock";
			$('span#ui_clock_msg', container).html("");
			remote.sendCommand(mac,cmd,hour,colorHour.h,colorHour.s,colorHour.v,minute,colorMinute.h,colorMinute.s,colorMinute.v,duration,fade,intro,pulse)
			.then(function () {
				$('span#ui_clock_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#ui_clock_msg', container).html("Error!");
				console.log(err);
			})
		});

		$('button#ui_clock_off', container).on('click', function () {
			var cmd = "ui_clock_off";
			$('span#ui_clock_off_msg', container).html("");
			remote.sendCommand(mac, cmd)
			.then(function () {
				$('span#ui_clock_off_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#ui_clock_off_msg', container).html("Error!");
				console.log(err);
			})
		});

		$('button#exp_ui_change_mode', container).on('click', function () {
			var cmd = "exp_ui_change_mode";
            var direction = parseInt($('select#change_mode_direction', container).val(), 10);
			var speed = parseInt($('input#change_mode_speed', container).val(), 10);
			if(isNaN(speed)) {
				$('span#exp_ui_change_mode_msg', container).html("speed must be a number");
				return;
			}
			var color = HSVparse($('#change_mode_color', container).data('colorpicker').input.val());
			$('span#exp_ui_change_mode_msg', container).html("");
			remote.sendCommand(mac, cmd, color.h,color.s,color.v, direction,speed)
			.then(function () {
				$('span#exp_ui_change_mode_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#exp_ui_change_mode_msg', container).html("Error!");
				console.log(err);
			})
		});

		$('button#exp_ui_change_mode_off', container).on('click', function () {
			var cmd = "exp_ui_change_mode_off";
			$('span#exp_ui_change_mode_off_msg', container).html("");
			remote.sendCommand(mac, cmd)
			.then(function () {
				$('span#exp_ui_change_mode_off_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#exp_ui_change_mode_off_msg', container).html("Error!");
				console.log(err);
			})
		});

		$('button#ui_touch_sound', container).on('click', function () {
			var volume = parseInt($('input#ui_touch_sound_volume', container).val(), 10);
			if(isNaN(volume) || volume < 0 || volume > 100) {
				$('span#ui_touch_sound_msg', container).html("bad volume");
				return;
			}
			var isOn = parseInt($('select#ui_sound', container).val(), 10);
			var cmd = "ui_touch_sound";
			$('span#ui_touch_sound_msg', container).html("");
			remote.sendCommand(mac,cmd,volume,isOn)
			.then(function () {
				$('span#ui_touch_sound_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#ui_touch_sound_msg', container).html("Error!");
				console.log(err);
			})
		});

		$('button#ui_compass', container).on('click', function () {
			var heading = parseInt($('input#compass_heading', container).val(), 10);
			if(isNaN(heading) || heading < -180 || heading > 180) {
				$('span#ui_compass_msg', container).html("bad heading");
				return;
			}
			var color = HSVparse($('#compass_color', container).data('colorpicker').input.val());
			$('span#ui_compass_msg', container).html("");
			remote.sendCommand(mac, 'ui_compass', color.h,color.s,color.v, heading)
			.then(function () {
				$('span#ui_compass_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#ui_compass_msg', container).html("Error!");
				console.log(err);
			})
		});

		$('button#ui_compass_off', container).on('click', function () {
			$('span#ui_compass_off_msg', container).html("");
			remote.sendCommand(mac, 'ui_compass_off')
			.then(function () {
				$('span#ui_compass_off_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#ui_compass_off_msg', container).html("Error!");
				console.log(err);
			})
		});

		$('button#ui_pointer', container).on('click', function () {
			var heading = parseInt($('input#pointer_heading', container).val(), 10);
			if(isNaN(heading) || heading < -180 || heading > 180) {
				$('span#ui_pointer_msg', container).html("bad heading");
				return;
			}
			var color = HSVparse($('#pointer_color', container).data('colorpicker').input.val());
			var mode = parseInt($('select#pointer_intro_mode', container).val(), 10);
			$('span#ui_pointer_msg', container).html("");
			remote.sendCommand(mac, 'ui_pointer', color.h,color.s,color.v,heading)
			.then(function () {
				$('span#ui_pointer_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#ui_pointer_msg', container).html("Error!");
				console.log(err);
			})
		});

		$('button#ui_pointer_standby', container).on('click', function () {
			var color = HSVparse($('#pointer_color', container).data('colorpicker').input.val());
			var mode = parseInt($('select#pointer_intro_mode', container).val(), 10);
			$('span#ui_pointer_standby_msg', container).html("");
			remote.sendCommand(mac, 'ui_pointer_standby', color.h,color.s,color.v)
			.then(function () {
				$('span#ui_pointer_standby_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#ui_pointer_standby_msg', container).html("Error!");
				console.log(err);
			})
		});

		$('button#ui_pointer_off', container).on('click', function () {
			$('span#ui_pointer_off_msg', container).html("");
			remote.sendCommand(mac, 'ui_pointer_off')
			.then(function () {
				$('span#ui_pointer_off_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#ui_pointer_off_msg', container).html("Error!");
				console.log(err);
			})
		});

		$('button#ui_oled_show', container).on('click', function () {
			var column = parseInt($('input#column', container).val(), 10);
			if(isNaN(column) || column < 0 || column > 20) {
				$('span#ui_oled_show_msg', container).html("bad column");
				return;
			}
			var row = parseInt($('input#row', container).val(), 10);
			if(isNaN(row) || row < 0 || row > 1) {
				$('span#ui_oled_show_msg', container).html("bad row");
				return;
			}
			var direction = parseInt($('select#direction', container).val(), 10);
			$('span#ui_oled_show_msg', container).html("");
			var duration = parseInt($('input#duration', container).val(), 10);
			if(isNaN(duration) || duration < 0 || duration > 65536) {
				$('span#ui_oled_show_msg', container).html("bad duration");
				return;
			}
			var position = parseInt($('input#position', container).val(), 10);
			if(isNaN(position) || position < -1 || position > 6) {
				$('span#ui_oled_show_msg', container).html("bad position");
				return;
			}
			remote.sendCommand(mac, 'ui_oled_show', column,row,direction,duration,position)
			.then(function () {
				$('span#ui_oled_show_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#ui_oled_show_msg', container).html("Error!");
				console.log(err);
			})
		});

		$('button#ui_oled_contrast', container).on('click', function () {
			var contrast = parseInt($('input#contrast', container).val(), 10);
			if(isNaN(contrast) || contrast < 0 || contrast > 255) {
				$('span#ui_oled_contrast_msg', container).html("bad contrast");
				return;
			}
			remote.sendCommand(mac, 'ui_oled_contrast', contrast)
			.then(function () {
				$('span#ui_oled_contrast_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#ui_oled_contrast_msg', container).html("Error!");
				console.log(err);
			})
		});

		$('button#ui_oled_brightness', container).on('click', function () {
			var brightness = parseInt($('input#brightness', container).val(), 10);
			if(isNaN(brightness) || brightness < 0 || brightness > 255) {
				$('span#ui_oled_brightness_msg', container).html("bad brightness");
				return;
			}
			remote.sendCommand(mac, 'ui_oled_brightness', brightness)
			.then(function () {
				$('span#ui_oled_brightness_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#ui_oled_brightness_msg', container).html("Error!");
				console.log(err);
			})
		});

		$('button#ui_oled_off', container).on('click', function () {
			$('span#ui_oled_off_msg', container).html("");
			remote.sendCommand(mac, 'ui_oled_off')
			.then(function () {
				$('span#ui_oled_off_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#ui_oled_off_msg', container).html("Error!");
				console.log(err);
			})
		});

		$('button#snd_legacy_play', container).on('click', function () {
			var vol = $('input#snd_legacy_vol', container).val();
			var repeat = $('input#snd_legacy_repeat', container).val();
			var track = $('input#snd_legacy_track', container).val();
			$('span#snd_legacy_play_msg', container).html("");
			remote.sendCommand(mac, 'snd_legacy_play', vol, repeat, track)
			.then(function () {
				$('span#snd_legacy_play_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#snd_legacy_play_msg', container).html("Error!");
				console.log(err);
			})
		});

		$('button#snd_play', container).on('click', function () {
			var repeat = $('input#snd_repeat', container).val();
			var track = $('input#snd_track', container).val();
			$('span#snd_play_msg', container).html("");
			remote.sendCommand(mac, 'snd_play', repeat, track)
			.then(function () {
				$('span#snd_play_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#snd_play_msg', container).html("Error!");
				console.log(err);
			})
		});

		$('button#snd_stop', container).on('click', function () {
			$('span#snd_stop_msg', container).html("");
			remote.sendCommand(mac, 'snd_stop')
			.then(function () {
				$('span#snd_stop_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#snd_stop_msg', container).html("Error!");
				console.log(err);
			})
		});

		$('button#alarm_report', container).on('click', function () {
			$('span#alarm_report_msg', container).html("");
			remote.sendCommand(mac, 'alarm_report')
			.then(function (report) {
				$('span#alarm_report_msg', container).html("armed:"+report.armed+', trigger:'+report.trigger+', severity:'+report.severity+', auto:'+report.auto);
			})
			.catch(function (err) {
				$('span#alarm_report_msg', container).html("Error!");
				console.log(err);
			})
		});

		$('button#alarm_arm', container).on('click', function () {
			$('span#alarm_arm_msg', container).html("");
			remote.sendCommand(mac, 'alarm_arm', 1)
			.then(function () {
				$('span#alarm_arm_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#alarm_arm_msg', container).html("Error!");
				console.log(err);
			})
		});
		$('button#alarm_disarm', container).on('click', function () {
			$('span#alarm_arm_msg', container).html("");
			remote.sendCommand(mac, 'alarm_arm', 0)
			.then(function () {
				$('span#alarm_arm_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#alarm_arm_msg', container).html("Error!");
				console.log(err);
			})
		});


		$('button#alarm_setConfig', container).on('click', function () {

			var mode = parseInt($('select#alarm_mode', container).val(), 10);
			var alarmCode = $('input#alarm_code', container).val();
			var severity = $('input#alarm_severity', container).val();

			if(!alarmCode) {
			    var code = 0;
			    var codelen = 0;
			} else {
			    codelen = alarmCode.length;
			    for(var i = 0; i < codelen; i++) {
			        if(alarmCode[i] == 'l') {
			            code |= (1 << i);
			        }
			    }
			}

			$('span#alarm_setConfig_msg', container).html("");
			remote.sendCommand(mac, 'alarm_setConfig', code, codelen, mode, severity)
			.then(function () {
				$('span#alarm_setConfig_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#alarm_setConfig_msg', container).html("Error!");
				console.log(err);
			})
		});

		$('button#device_enterBootloader', container).on('click', function () {
			$('span#device_enterBootloader_msg', container).html("");
			remote.sendCommand(mac, 'device_enterBootloader')
			.then(function () {
				$('span#device_enterBootloader_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#device_enterBootloader_msg', container).html("Error!");
				console.log(err);
			})
		});

		$('button#device_getState', container).on('click', function () {
			$('span#device_getState_msg', container).html("");
			remote.sendCommand(mac, 'device_getState')
			.then(function (state) {
				$('span#device_getState_msg', container).html(
					"SOC:"+state.SOC+', Charging:'+state.charging+', Compass:'+ state.compass+
					', Front.setting:'+ state.frontlight.setting+', Front.state:'+ state.frontlight.state+', Temp:'+ state.temperature+', Plugged:'+ state.plugged+
                    ', Travel mode:'+ state.travelMode
				);
			})
			.catch(function (err) {
				$('span#device_getState_msg', container).html("Error!");
				console.log(err);
			})
		});

		$('button#device_setName', container).on('click', function () {
			var val = $('input#device_name', container).val();
			$('span#device_setName_msg', container).html("");
			remote.sendCommand(mac, 'device_setName', val)
			.then(function () {
				$('span#device_setName_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#device_setName_msg', container).html("Error!");
				console.log(err);
			})
		});
		$('button#device_compass_calibrate', container).on('click', function () {
			$('span#device_compass_calibrate_msg', container).html("");
			remote.sendCommand(mac, 'device_compass_calibrate')
			.then(function () {
				$('span#device_compass_calibrate_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#device_compass_calibrate_msg', container).html("Error!");
				console.log(err);
			})
		});
		$('button#device_shutdown', container).on('click', function () {
			$('span#device_shutdown_msg', container).html("");
			remote.sendCommand(mac, 'device_shutdown')
			.then(function () {
				$('span#device_shutdown_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#device_shutdown_msg', container).html("Error!");
				console.log(err);
			})
		});

		$('button#device_getSerial', container).on('click', function () {
			$('span#device_getSerial_msg', container).html("");
			remote.sendCommand(mac, 'device_getSerial', 0)
			.then(function (serial) {
				$('span#device_getSerial_msg', container).html("Ok! Serial: " + serial);
			})
			.catch(function (err) {
				$('span#device_getSerial_msg', container).html("Error!");
				console.log(err);
			})
		});


		$('button#device_capture_reset', container).on('click', function () {
			remote.dump_reset()
			.then(function () {
				$('span#device_capture_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#device_capture_msg', container).html("Error!");
				console.log(err);
			})
		});
		$('button#device_capture_start', container).on('click', function () {
			remote.sendCommand(mac, 'device_capture', 1)
			.then(function () {
				$('span#device_capture_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#device_capture_msg', container).html("Error!");
				console.log(err);
			})
		});
		$('button#device_capture_stop', container).on('click', function () {
			remote.sendCommand(mac, 'device_capture', 0)
			.then(function () {
				$('span#device_capture_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#device_capture_msg', container).html("Error!");
				console.log(err);
			})
		});


		$('button#travelmode_set', container).on('click', function () {
			$('span#travelmode_msg', container).html("");
			remote.sendCommand(mac, 'device_setTravelMode', 1)
			.then(function () {
				$('span#travelmode_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#travelmode_msg', container).html("Error!");
				console.log(err);
			})
		});

		$('button#travelmode_clear', container).on('click', function () {
			$('span#travelmode_msg', container).html("");
			remote.sendCommand(mac, 'device_setTravelMode', 0)
			.then(function () {
				$('span#travelmode_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#travelmode_msg', container).html("Error!");
				console.log(err);
			})
		});


		$('button#test_leds', container).on('click', function () {
			var num = parseInt($('input#test_leds_num', container).val(), 10);
			var r = parseInt($('input#test_leds_r', container).val(), 10);
			var g = parseInt($('input#test_leds_g', container).val(), 10);
			var b = parseInt($('input#test_leds_b', container).val(), 10);
			$('span#test_leds_msg', container).html("");
			remote.sendCommand(mac, 'test_leds_legacy',num,r,g,b)
			.then(function () {
				$('span#test_leds_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#test_leds_msg', container).html("Error!");
				console.log(err);
			})
		});

        /*$('button#exp_ble_disable', container).on('click', function () {
            var standby_time = $('input#exp_ble_disable_time', container).val();
			$('span#exp_ble_disable_msg', container).html("");
			remote.sendCommand(mac, 'exp_ble_disable', standby_time)
			.then(function () {
				$('span#exp_ble_disable_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#exp_ble_disable_msg', container).html("Error!");
				console.log(err);
			})
		});*/

        $('button#exp_ui_demo', container).on('click', function () {
            var id = $('input#exp_ui_demo_id', container).val();
            var arg1 = $('input#exp_ui_demo_arg1', container).val();
            var arg2 = $('input#exp_ui_demo_arg2', container).val();
			$('span#exp_ui_demo_msg', container).html("");
			remote.sendCommand(mac, 'ui_demo', id, arg1, arg2)
			.then(function () {
				$('span#exp_ui_demo_msg', container).html("Ok!");
			})
			.catch(function (err) {
				$('span#exp_ui_demo_msg', container).html("Error!");
				console.log(err);
			})
		});

	}

	function graphView() {
		var id = frame_create({'label':'Graph',color:'color2'});
		var container = "#content div#frame_"+id;
		$(container).append($("core-templates > core-template#graphView").clone());
		$('div#nav li#nav_' + id + ' a:first-child').trigger('click');

		remote.dump_get()
		.then(function (data) {
			var rows = data.split('\n');
			var xses = rows.map(function(row) { 
				return row.split(',')[0]; 
			});
			var yses = rows.map(function(row) { 
				return row.split(',')[1]; 
			});
			var zses = rows.map(function(row) { 
				return row.split(',')[2]; 
			});
			var trace1 = {
				x:xses,  y: yses, z: zses, 
				mode: 'markers',
				marker: {
					size: 6,
					line: {
						color: 'rgba(217, 217, 217, 0.14)',
						width: 0.5
					},
					opacity: 0.8
				},
				type: 'scatter3d'
			};
			var data = [trace1];
			var layout = {margin: {
				l: 0,
				r: 0,
				b: 0,
				t: 0
			}};
			Plotly.newPlot('myDiv', data, layout);

		})

	}

	clifw.nav_add({
		label: "Admin",
		id: 'admin',
		click: function () {
			$('div#content div#admin').addClass('visible').siblings().removeClass('visible');
		}
	});
	$('div#nav ul#mainnav li:first a:first').trigger('click');


	$('div#admin button#tool_plot').on('click', function () {
		graphView();
	});


	var listdevices = clifw.table('div#admin table#listdevices', {'onClick':deviceView});
	$('body').on('admin.devicelist', function (e, list) {
		if(list) {
			listdevices.update(list);
		}
	});	

}
