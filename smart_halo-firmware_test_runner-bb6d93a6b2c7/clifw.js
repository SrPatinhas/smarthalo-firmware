
var randomId = function () {
    var s = '';
    for (var i = 0; i < 4; i++) {
        s += (Math.random().toString(16).slice(2) + "00000000").slice(0,8);
    }
    return s;
}

//******************************************************************************
// frame
//******************************************************************************

var frame_create = exports.frame_create = function (options) {

	var defoptions = {
		color:'color1',
		label:'(sans nom)',
		classname:'',
	};
	var options = $.extend(defoptions, options);

	var id = randomId();

	var container = "#content div#frame_"+id;

	var nav = {
		id:id,
		label: options.label,
		type: 'subnav',
		color: options.color,
		click: function (nav) {
			$('div#content > div#frame_' + nav.id).addClass('visible').siblings().removeClass('visible');
			$('div#content > div#frame_' + nav.id).trigger('show');
		},
		close: function (nav) {
			$('div#content > div#frame_' + nav.id).trigger('close');
		}
	};
	nav_add(nav);

	$('<div id="frame_'+id+'" class="'+options.classname+'"></div>').appendTo('#content');

	$(container).on('close', function () {
		var visible = $(container).hasClass('visible');
		if(visible) {
			nav_back();					
		}
		nav_remove(nav);
		$(container).remove();
	});

	return id;
}


//******************************************************************************
// Nav
//******************************************************************************

var nav_add = exports.nav_add = function nav_add(nav) {
	if(!(nav instanceof Array)) {
		nav = [nav];
	}

	nav.forEach(function (item) {
		//console.log(item);
		item.scrolltop = 0;
		item.type = item.type || 'mainnav';
		var cont = 'div#nav > ul#' + item.type;
		var contli = $('<li />')
			.data('nav', item)
			.appendTo($(cont));
		if(item.id) {
			contli.attr('id', 'nav_' + item.id)
		}
		$('<a href="#"></a>')
			.text(item.label)
			.appendTo(contli);
		if(item.color) {
			contli.addClass(item.color);					
		}
		if(item.close) {
			contli.addClass('closable');					
			$('<a href="#"><i class="fa fa-times-circle"></i></a>').appendTo(contli);
		}
	});

}

var nav_remove = exports.nav_remove = function nav_remove(nav) {
	if(!(nav instanceof Array)) {
		nav = [nav];
	}
	nav.forEach(function (item) {
		if(item.id) {
			var cont = 'div#nav li#nav_' + item.id;
			var nav = $(cont).data('nav');
			if(nav && nav.remove) {
				nav.remove(nav);
			}
			$(cont).remove();
		}
	});
	
}

var nav_back = exports.nav_back = function nav_back() {
	$('div#nav').trigger('back');
}


var nav_init = exports.nav_init = function nav_init() {

	$('div#nav ul').empty();
	var histories = [];
	function history(e) {
		if(histories[histories.length-1] == e) 
			return;
		if(histories.length > 10) {
			histories.shift();
		}
		histories.push(e)
	}
	function unhistory(e) {
		var i = histories.length;
		while(i--) {
			if(e == histories[i]) {
				histories.splice(i, 1);
			}
		}

		i = histories.length;	
		var prevlen = 0;
		while(i != prevlen) {
			
			prevlen = i;
			i--;
			while(i--) {
				if(histories[i+1] == histories[i]) {
					histories.splice(i, 1);
				}
			}
			i = histories.length;	

		}
		//console.log(histories);
	}			

	$('div#nav').on('back', function () {
		var id = histories.pop();
		if(id) {
			$('div#nav li#'+id+' a:first-child').click();
		}				
	});

	$('div#nav > ul').on('click', ' li > a:first-child', function () {
		var id = $(this).parent().attr('id');
		if(id) {
			history(id);
		}

		var nav = $('div#nav > ul > li.active').data('nav');
		if(nav) {
			nav.scrolltop = $('body').scrollTop();
			//console.log('save ' + nav.scrolltop);
		}

		$('div#nav > ul > li').not($(this).parent()).removeClass('active');
		$(this).parent().addClass('active');
		nav = $(this).parent().data('nav');
		if(nav) {
			if(nav.click) {
				nav.click(nav);
			}
			//console.log('restore ' + nav.scrolltop);
			$('body').scrollTop(nav.scrolltop);
		}
		return false;
	});

	$('div#nav > ul').on('click', ' li > a:nth-child(2)', function () {
		var id = $(this).parent().attr('id');
		if(id) {
			unhistory(id);
		}
		var nav = $(this).parent().data('nav');
		if(nav && nav.close) {
			nav.close(nav);
		}
		return false;
	});

	//========================================================================================
	//========================================================================================
	//subnav

	$(window).on('scroll', function() {
		var scrtop = $(window).scrollTop();
		var closest = {id:"", top:Infinity};
		$('#content > div.visible').find('h1,h2').not('.hidden').each(function () {
			if(!$(this).attr('id')) return;
			var tst = Math.abs($(this).offset().top - scrtop);
			if(tst < closest.top) {
				closest.top = tst;
				closest.id = $(this).attr('id');
			}
		});
		//console.log('tst: ' + closest.id);
		if(closest.id) {
			$('#content > div.visible ul.nav-sidebar li').each(function () {
				if($(this).find('a').attr('href').substr(1) == closest.id) {
					$(this).addClass('active').siblings().removeClass('active');
				}
			});
		}
	});

	$('a.navbar-brand').on('click', function () {
		if($(this).parents('.navbar').hasClass('navbartoggle')) {
			$('div.navbarmain').animate({top:0}, 400);
			$('div.navbartoggle').animate({top:-55}, 400);
		} else {
			$('div.navbarmain').animate({top:-55}, 400);
			$('div.navbartoggle').animate({top:0}, 400);
		}
		return false;
	});

	//========================================================================================
	//========================================================================================
	//connection handling

	$('body').on('disconnect.core', function () {
		$('div#pleaseWaitDialog div.modal-body h2').text("Reconnect...");
		$('div#pleaseWaitDialog').modal('show');
	});

	$('body').on('connect.core', function () {
		$('div#pleaseWaitDialog').modal('hide');
	});

	var longerwait;
	$('body').on('waitstart.core', function () {
		$('body').addClass('wait');
		longerwait = setTimeout(function () {
			$('div#pleaseWaitDialog div.modal-body h2').text("Please Wait...");
			$('div#pleaseWaitDialog').modal('show');
		},2000);
	});

	$('body').on('waitdone.core', function () {
		$('body').removeClass('wait');
		if(longerwait) {
			clearTimeout(longerwait);
		}
		$('div#pleaseWaitDialog').modal('hide');
	});

	//========================================================================================
	//========================================================================================

}


//******************************************************************************
// Table
//******************************************************************************

var table = exports.table = function (tableid, options) {

	options = options || {};

	var clickable = (options.onClick instanceof Function);

	if(clickable) {
		$(tableid).on('click', 'tbody tr.entry', function(event) {
		    ////var entry = $(this).data('entry');
		    ////options.onClick(entry, $(this));
			options.onClick($(this).attr("docid"), $(this));
		});
	}

	var _frame = function(id, template) {
		var editrow = '<tr docid="'+id+'" class="edit"><td colspan="10"></td></tr>';
		var row = $(tableid + ' tr.entry[docid="'+id+'"]');
		if(row.length) {
			row.after(editrow);
		} else {
			$(tableid + ' tbody').prepend(editrow);
		}
		if(template) {
			var editrow = $(tableid + ' tr.edit[docid="'+id+'"] td');
			editrow.append($("core-templates > core-template#"+template).clone());
		}
		return tableid + ' tr.edit[docid="'+id+'"] td';
	}

	var _update = function (model) {
		var clickableClass = (clickable) ? 'clickable' : '';
		var headfield = [];
		var headclass = [];
		var actionbtn = options.actionbtn || '&nbsp;';
		if(!(actionbtn instanceof Array)) {
			actionbtn = [actionbtn];
		}
		
		$(tableid + ' > thead > tr > th').each(function () {
			headfield.push($(this).attr('field'));
			headclass.push($(this).attr('class'));
		});
		if(model.body instanceof Array) {
			$(tableid + ' > tbody').empty();
			var rows = [];
			model.body.forEach(function (doc) {
				//$(tableid + ' > tbody').append(createrow(doc));
				////var row = $(tableid + ' > tbody').find('tr.entry[docid="'+doc['_id'] + '"]');
				////row.data('entry', doc);
				rows.push(createrow(doc));
			});
			$(tableid + ' > tbody').append(rows.join(''));
		} 
		else if(model.data instanceof Array) {
			model.data.forEach(function (doc) {
				var id = doc._id;
				for(var i in doc) {
					if(i == '_id') 
						continue;
					$(tableid + ' > tbody tr[docid="'+id+'"] td[field="'+i+'"]').html(doc[i]);
				}
			});
		} 
		else if(model.delete) {
			$(tableid + ' > tbody').find('tr.entry[docid="'+model.delete.id + '"]').remove();
		} 
		else if(model.entry){
			var doc = model.entry;
			var row = $(tableid + ' > tbody').find('tr.entry[docid="'+doc['_id'] + '"]');
			if(row.length) {
				row.replaceWith(createrow(doc));
			} else {
				$(tableid + ' > tbody').prepend(createrow(doc));
			}
			////row = $(tableid + ' > tbody').find('tr.entry[docid="'+doc['_id'] + '"]');
			////row.data('entry', doc);
		}
		if(model.footer) {
			for(var i in model.footer) {
				$(tableid + ' > tfoot .' + i).html(model.footer[i]);
			}
		}

		function createrow(doc) {
			var html_class = doc._html_class || '';
			var str = '<tr docid="' + doc['_id'] + '" class="entry ' + clickableClass + ' ' + html_class + '">';
			var actionbtn_cnt = 0;
			for(var i in headfield) {
				if(headfield[i] == '_actionbtn') {
					str += '<td class="'+headclass[i]+'">' + actionbtn[actionbtn_cnt++] + '</td>';
				} else {
					var props = headfield[i].split('.');
					var td = nestedget(doc, props);
					if(td === undefined) {
						td = '<em>(undefined)</em>';
					}
					str += '<td field="'+headfield[i]+'" class="'+headclass[i]+'">' + td + '</td>';
				}
			}
			str += '</tr>';
			return str;
		}	
		function nestedget(obj, props) {
			if(!obj) return obj;
			var name = props.shift();
			if(props.length) {
				return nestedget(obj[name], props);
			} else {
				return obj[name];
			}
		}	
	}	

	return {
		'update':_update,
		'frame':_frame
	}


}
