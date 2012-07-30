(function() { // Keep everything as neat as possible

$(function() {
	panelbody = $('div#panels');
	
	newPanel(panelbody,'alpha');
});

var MAX_FAILURES = 5;

function newPanel(parent, car) {
	var potplot;
	var dataInterval;
	var potdata = [[]];
	
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
		function pad0(value, width) {
			var str = String(value);
			while(str.length < width)
				str = '0' + str;
			return str;
		}
		
		var date = new Date();
		var timestamp = pad0(date.getHours(),2)
			+ ':' + pad0(date.getMinutes(),2)
			+ ':' + pad0(date.getSeconds(),2)
			+ '.' + pad0(date.getMilliseconds(),3);
		
		$('div#log')
			.append('<p>' + timestamp + ': ' + panel.car + ': ' + msg + '</p>')
			.animate({scrollTop: $('div#log')[0].scrollHeight});
	}
	
	panel.car = car;
	
	panel.attr({
		id: car + 'monitor',
		class: 'panel'
	});
	
	panel.append('<h1>' + car + '</h1>');
	
	// Potentiometer graph (Flot)
	var potentiometer = $('<div name="potentiometer"></div>');
	panel.appendWidget(potentiometer);
	potplot = $.plot($('<div></div>').css({height: '240px'}).appendTo(potentiometer),[potdata],{
		xaxis: {
			min: potdata[potdata.length - 1][0] - dataInterval,
			mode: 'time',
			timeformat: '%H:%M:%S %P',
			twelveHourClock: true
		},
		yaxis: {min: 0, max: 1023}
	});
	
	// Slider
	$('<div></div>')
		.append($('<input type="range" min="0" max="1.01" step="0.01" value="0.32">')
			.change(function(event) {
				var interp = $(event.target).val();
				var logseconds = (1 - interp)*Math.log(2) + interp*Math.log(24*60*60);
				var seconds = Math.round(Math.exp(logseconds));
				dataInterval = seconds;
				try {
					potplot.setupGrid();
					potplot.draw();
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
		.appendTo(potentiometer)
		.children('input').change();
	
	// Map (Leaflet)
	panel.appendWidget($('<div id="' + car + 'map"></div>')
		.css({height: '320px'})
	,'two');
	var map = new L.Map(car + 'map',{
		dragging: false,
		touchZoom: false,
		scrollWheelZoom: false,
		doubleClickZoom: false,
		boxZoom: false,
		zoomControl: false
	});
	map.addLayer(new L.TileLayer('http://otile{s}.mqcdn.com/tiles/1.0.0/osm/{z}/{x}/{y}.jpg',{
		attribution: 'Tiles Courtesy of <a href="http://www.mapquest.com/" target="_blank">MapQuest</a>'
			+ ' <img src="http://developer.mapquest.com/content/osm/mq_logo.png">'
			+ ' &mdash; Map data &copy; by <a href="http://www.openstreetmap.org/" target="_blank">OpenStreetMap</a>'
			+ ' contributors, <a href="http://creativecommons.org/licenses/by-sa/2.0/" target="_blank">CC-BY-SA</a>',
		maxZoom: 18,
		subdomains: '1234'
	}));
	map.setView(new L.LatLng(44.86774,-93.137112),15);
	
	// Get an initial time, then update it once a second
	function initializeSync() {
		$.get('data',{
			type: 'ajax',
			car: car
		})
		.done(function(datum) {
			var failures = 0;
			var timeBase = datum.time;
			var time = 0;
			
			if(timeBase <= 0) {
				panel.log('server has no data; killing panel');
				return;
			}
			
			var syncinterval = setInterval(function() {
					$.get('data',{
						type: 'ajax',
						car: car,
						time: timeBase + time
					})
					.done(function(datum) {
						potdata.push([parseInt(datum.time)*1000,parseInt(datum.potentiometer)]);
						
						potplot.setupGrid();
						potplot.draw();
						
						failures = 0;
					},'json')
					.fail(function(jqXHR) {
						panel.log('GET failed: ' + jqXHR.responseText);
						
						if(++failures >= MAX_FAILURES)
						{
							panel.log(MAX_FAILURES + ' sequential failures; suspending panel for 10 seconds');
							clearInterval(syncinterval);
							setTimeout(initializeSync,10000); // Reinitialize; the clocks may have just gotten out of sync
						}
					});
					
					time++;
				},1000);
		},'json')
		.fail(function(jqXHR) {
			panel.log('GET failed: ' + jqXHR.responseText);
		});
	}
	
	initializeSync();
}

})();
