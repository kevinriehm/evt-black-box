(function() { // Keep everything as neat as possible

$(function() {
	$('div#header').append(new Clock());
	
	panelbody = $('div#panels');
	newPanel(panelbody,'alpha');
});

function pad0(value, width) {
	var str = String(value);
	while(str.length < width)
		str = '0' + str;
	return str;
}

function hours24to12(hours24) {
	var hours12 = hours24%12;
	return hours12 == 0 ? 12 : hours12;
}

Clock.prototype = $('<div></div>');
function Clock() {
	var clock = this;
	clock.css('float','right');
	setInterval(function() {
		var date = new Date();
		clock.html(hours24to12(date.getHours(),2)
			+ ':' + pad0(date.getMinutes(),2)
			+ ':' + pad0(date.getSeconds(),2)
			+ ' ' + (date.getHours() < 12 ? 'AM' : 'PM'));
	},1000);
}

function fuzzyValidateDate(element) {
	return new Date(Date.parse(element.val()));
}

function newPanel(parent, car) {
	var MAX_FAILURES = 5;
	
	var graphs = [];
	var panel = $('<div></div>')
		.appendTo(parent);
	
	panel.appendWidget = function(widget, columns) {
		var columnclass;
		
		if(columns == undefined || columns == 'one')
			columnclass = 'onecolumn';
		else columnclass = columns + 'columns';
		
		widget = $(widget).addClass(columnclass);
		panel.append(widget);
	}

	panel.log = function(msg) {
		var date = new Date();
		var timestamp = pad0(date.getHours(),2)
			+ ':' + pad0(date.getMinutes(),2)
			+ ':' + pad0(date.getSeconds(),2)
			+ '.' + pad0(date.getMilliseconds(),3);
		
		$('div#log')
			.append('<p>' + timestamp + ': ' + panel.car + ': ' + msg + '</p>')
			.animate({scrollTop: $('div#log')[0].scrollHeight});
	}
	
	panel.appendGraph = function(datumfield) {
		// Graph (Flot)
		var graphplot;
		var graphrange;
		var graphdata = [];
		var graph = $('<div></div>');
		
		graph.handleDatum = function(datum) {
			graphdata.push([parseInt(datum.time)*1000,parseInt(datum[datumfield])]);
			
			$.plot(graphplot,[graphdata],{
				xaxis: {
					min: graphdata[graphdata.length - 1][0] - graphrange*1000,
					mode: 'time',
					timeformat: '%H:%M:%S %P',
					twelveHourClock: true
				},
				yaxis: {min: 0, max: 1023}
			});
		}
		
		graphs.push(graph);
		panel.appendWidget(graph);
		
		graphplot = $('<div></div>')
			.css({height: '240px'})
			.appendTo(graph);
		
		// Slider
		$('<div></div>')
			.append($('<input type="range" min="0" max="1.01" step="0.01" value="0.32">')
				.change(function(event) {
					var interp = $(event.target).val();
					var logseconds = (1 - interp)*Math.log(2) + interp*Math.log(24*60*60);
					var seconds = Math.round(Math.exp(logseconds));
					graphrange = seconds;
					try {
						graphplot.setupGrid();
						graphplot.draw();
					} catch(e) {}
					
					var value, unit;
					if(seconds < 60) value = Math.round(seconds), unit = 'second';
					else if(seconds < 60*60) value = Math.round(seconds/60), unit = 'minute';
					else if(seconds < 24*60*60) value = Math.round(seconds/(60*60)), unit = 'hour';
					else value = Math.round(seconds/(24*60*60)), unit = 'day';
					
					$(this).siblings('label').html(value + ' ' + unit + (value == 1 ? '' : 's'));
				})
				.css({width: '50%'}))
			.append($('<label></label>')
				.css({float: 'right'}))
			.appendTo(graph)
			.children('input').change();
	}
	
	panel.car = car;
	
	panel.attr({
		id: car + 'monitor',
		class: 'panel'
	});
	
	panel.append('<h1>' + car + '</h1>');
	panel.appendGraph('potentiometer');
	
	// Map (Leaflet)
	panel.appendWidget('<div id="' + car + 'map"></div>');
	var map = new L.Map(car + 'map',{
		attributionControl: false,
		dragging: false,
		touchZoom: false,
		scrollWheelZoom: false,
		doubleClickZoom: false,
		boxZoom: false
	});
	var attribution = new L.Control.Attribution();
	attribution.setPrefix('Powered by <a href="http://leaflet.cloudmade.com/" target="_blank">Leaflet</a>');
	map.addControl(attribution);
	map.addLayer(new L.TileLayer('http://otile{s}.mqcdn.com/tiles/1.0.0/osm/{z}/{x}/{y}.jpg',{
		attribution: 'Tiles Courtesy of <a href="http://www.mapquest.com/" target="_blank">MapQuest</a>'
			+ ' <img src="http://developer.mapquest.com/content/osm/mq_logo.png" style="vertical-align: middle;">'
			+ ' &mdash; Map data &copy; by <a href="http://www.openstreetmap.org/" target="_blank">OpenStreetMap</a>'
			+ ' contributors, <a href="http://creativecommons.org/licenses/by-sa/2.0/" target="_blank">CC-BY-SA</a>',
		maxZoom: 18,
		subdomains: '1234'
	}));
	var carpath = new L.Polyline([]);
	map.addLayer(carpath);
	
	// CSV export
	panel.appendWidget($('<div></div>')
		.append('<h2>CSV Export</h2>')
		.append($('<form></form>')
			.append('<label for="' + car + 'exportfrom">From:</label>')
			.append('<input id="' + car + 'exportfrom" type="datetime">')
			.append('<label for="' + car + 'exportto">To:</label>')
			.append('<input id="' + car + 'exportto" type="datetime">')
			.append('<input type="submit">')
			.submit(function(event) {
				event.preventDefault();
				var from = fuzzyValidateDate($('#' + car + 'exportfrom'));
				var to = fuzzyValidateDate($('#' + car + 'exportto'));
				if(!(from instanceof Date && to instanceof Date)) return;
				
				$('<iframe></iframe>')
					.load(function(event) {alert('Loaded');
						$(this).remove();
					})
					.css({display: 'none'})
					.attr('src','data?type=csv&car=' + car + '&start=' + from.getTime()/1000 + '&end=' + to.getTime()/1000)
					.appendTo($(this).parent());
			})));
	
	// Get an initial time, then update it once a second
	function initializeSync() {
		$.get('data',{
			type: 'ajax',
			car: car
		})
		.done(function(datum) {
			var failures = 0;
			var timebase = parseInt(datum.time);
			var time = 0;
			
			var syncinterval;
			var resynctimeout;
			
			if(timebase <= 0) {
				panel.log('server has no data; killing panel');
				return;
			}
			
			
			syncinterval = setInterval(function() {
				$.get('data',{
					type: 'ajax',
					car: car,
					time: timebase + time
				})
				.done(function(datum) {						
					graphs.forEach(function(graph) {graph.handleDatum(datum)});
					var latlng = new L.LatLng(parseInt(datum.latitude),parseInt(datum.longitude));
					carpath.addLatLng(latlng);
					map.setView(latlng,15);
					failures = 0;
				},'json')
				.fail(function(jqXHR) {
					panel.log('GET failed: ' + jqXHR.responseText);
					
					if(++failures >= MAX_FAILURES)
					{
						panel.log(MAX_FAILURES + ' sequential failures; suspending panel for 10 seconds');
						clearInterval(syncinterval);
						resynctimeout = setTimeout(initializeSync,10000); // Reinitialize; the clocks may have just gotten out of sync
					}
				});
				
				time++;
			},1000);
			
			// Hook up the pause button
			(function() {
				var paused = false;
				$('button[name=pause]').click(function() {
					if(paused) {
						setTimeout(initializeSync,10000);
						$(this).html('Running');
					} else {
						clearInterval(syncinterval);
						clearTimeout(resynctimeout);
						$(this).html('Stopped');
					}
					paused = !paused;
				});
			})();
		},'json')
		.fail(function(jqXHR) {
			panel.log('GET failed: ' + jqXHR.responseText);
		});
	}
	
	initializeSync();
}

})();
