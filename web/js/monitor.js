(function() { // Keep everything as neat as possible

$(function() {
	$('div#header').append(new Clock());
	initMonitor(['alpha']);
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
	clock.addClass('right');
	setInterval(function() {
		var date = new Date();
		clock.html(hours24to12(date.getHours(),2)
			+ ':' + pad0(date.getMinutes(),2)
			+ ':' + pad0(date.getSeconds(),2)
			+ ' ' + (date.getHours() < 12 ? 'AM' : 'PM'));
	},1000);
}

function fuzzyValidateDate(element) {
	var time = Date.parse(element.val());
	return isNaN(time) ? false : new Date(time);
}

function initMonitor(cars) {
	var MAX_FAILURES = 5;
	
	var graphs = [];
	var monitor = $('div#monitor');
	
	monitor.appendWidget = function(widget, columns) {
		var columnclass;
		
		if(columns == undefined || columns == 'one')
			columnclass = 'onecolumn';
		else columnclass = columns + 'columns';
		
		widget = $(widget).addClass(columnclass);
		monitor.append(widget);
	}

	monitor.log = function(msg) {
		var date = new Date();
		var timestamp = pad0(date.getHours(),2)
			+ ':' + pad0(date.getMinutes(),2)
			+ ':' + pad0(date.getSeconds(),2)
			+ '.' + pad0(date.getMilliseconds(),3);

		console.log(timestamp + ': ' + msg);
	}

	monitor.appendGraph = function(datumfield) {
		// Graph (Flot)
		var graphplot;
		var graphrange;
		var graphdata = [];
		var graph = $('<div></div>');

		graph.handleDatum = function(datum) {
			var curtime = 0;

			// Add each car's datum
			cars.forEach(function(car) {
				if(datum[car] != undefined) {
					var time = parseInt(datum[car].time)*1000;
					if(time > curtime) curtime = time;

					try {
						if(graphdata[car].data.slice(-1)[0][0] != time) throw {};
					} catch(e) {
						graphdata[car].data.push([time,parseInt(datum[car][datumfield])]);
					}
				}
			});

			// Replot with the new data
			$.plot(graphplot,graphdata,{
				xaxis: {
					min: curtime - graphrange*1000,
					mode: 'time',
					timeformat: '%H:%M:%S %P',
					twelveHourClock: true
				},
				yaxis: {min: 0}
			});
		};

		cars.forEach(function(car) {
			graphdata.push(graphdata[car] = {
				data: [],
				label: car
			});
		});

		graph.append('<h2>' + datumfield + '</h2>');

		graphs.push(graph);
		monitor.appendWidget(graph);

		graphplot = $('<div></div>')
			.css({height: '250px'})
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
	
	var maps = [];
	var carpaths = [];
	
	// Track the cars
	cars.forEach(function(car) {
		var carpath = new L.Polyline([]);
		carpaths[car] = carpath;
		carpaths.push(carpath);
	});
	
	// Map (Leaflet)
	monitor.appendMap = function(car) {
		monitor.appendWidget('<div id="' + car + 'map"></div>');
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
		
		// Use MapQuest if available; otherwise fallback on local tiles
		var tileoptions = {
			attribution: 'Tiles Courtesy of <a href="http://www.mapquest.com/" target="_blank">MapQuest</a>'
				+ ' <img src="http://developer.mapquest.com/content/osm/mq_logo.png" style="vertical-align: middle;">'
				+ ' &mdash; Map data &copy; by <a href="http://www.openstreetmap.org/" target="_blank">OpenStreetMap</a>'
				+ ' contributors, <a href="http://creativecommons.org/licenses/by-sa/2.0/" target="_blank">CC-BY-SA</a>',
			maxZoom: 18
		};
		
		$.get('http://otile1.mqcdn.com/tiles/1.0.0/osm/0/0/0.jpg?' + new Date().valueOf())
		.done(function() {
			tileoptions.subdomains = '1234';
			map.addLayer(new L.TileLayer('http://otile{s}.mqcdn.com/tiles/1.0.0/osm/{z}/{x}/{y}.jpg',tileoptions));
		})
		.fail(function() {
			map.addLayer(new L.TileLayer('tiles/{z}/{x}/{y}.jpg',tileoptions));
		});
		
		// Add a map for each car
		carpaths.forEach(function(carpath) {
			map.addLayer(carpath);
		});
		
		maps[car] = map;
	};
	
	monitor.appendGraph('speed');
	monitor.appendGraph('amperage');
	monitor.appendGraph('voltage');
	
	cars.forEach(function(car) {
		monitor.appendMap(car);
	});
	
	// CSV export
	monitor.appendWidget($('<div></div>')
		.append('<h2>CSV Export</h2>')
		.append($('<form></form>')
			.each(function(index, element) { // A bit hackish, but oh well...
				cars.forEach(function(car, index) {
					$(element)
						.append('<label class="car stacked" for="' + car + 'export">' + car + ':</label>')
						.append('<input id="' + car + 'export" name="export" type="radio" value=' + car + (index == 0 ? ' checked="checked"' : '') + '>')
						.append('<br>');
				});
			})
			
			.append('<label class="stacked" for="exportfrom">From:</label>')
			.append('<input id="exportfrom" type="datetime">')
			.append('<br>')
			
			.append('<label class="stacked" for="exportto">To:</label>')
			.append('<input id="exportto" type="datetime">')
			.append('<br>')
			
			.append('<input class="center" type="submit">')
			.submit(function(event) {
				event.preventDefault();
				
				var abort = false;
				
				var from = fuzzyValidateDate($('#exportfrom'));
				var to = fuzzyValidateDate($('#exportto'));
				
				if(!from) $('#exportfrom').addClass('bad'), abort = true;
				else $('#exportfrom').removeClass('bad');
				
				if(!to) $('#exportto').addClass('bad'), abort = true;
				else $('#exportto').removeClass('bad');
				
				if(abort) return;
				
				$('<iframe></iframe>')
					.load(function(event) {$(this).remove();})
					.css({display: 'none'})
					.attr('src','data?format=csv&car=' + $('input[type=radio][name=export][checked=checked]').val() + '&start=' + from.getTime()/1000 + '&end=' + to.getTime()/1000)
					.appendTo($(this).parent());
			})));
	
	var syncinterval;
	var resynctimeout;
	
	// Get an initial time, then update it once a second
	function initializeSync() {
		$.get('data',{
			format: 'ajax',
			cars: cars
		})
		.done(function(datum) {
			var time = 0;
			var failures = 0;
			var timebase = parseInt(datum.time);
			
			syncinterval = setInterval(function() {
				$.get('data',{
					format: 'ajax',
					cars: cars,
					time: timebase + time
				})
				.done(function(datum) {
					graphs.forEach(function(graph) {graph.handleDatum(datum)});
					
					// Update the maps
					cars.forEach(function(car) {
						try {
							var latlng = new L.LatLng(parseInt(datum[car].latitude),parseInt(datum[car].longitude));
							var lastlatlng = carpaths[car].getLatLngs().slice(-1)[0];
							if(latlng.lat != lastlatlng.lat || latlng.lng != lastlatlng.lng)
								carpaths[car].addLatLng(latlng);
							maps[car].setView(latlng,15);
						} catch(e) {}
					});
					
					failures = 0;
				},'json')
				.fail(function(jqXHR) {
					monitor.log('GET failed: ' + jqXHR.responseText.replace(/\n$/,''));
					
					if(++failures >= MAX_FAILURES)
					{
						monitor.log(MAX_FAILURES + ' sequential failures; resyncing monitor');
						clearInterval(syncinterval);
						resynctimeout = setTimeout(initializeSync,1000); // The clocks may have just gotten out of sync
					}
				});
				
				time++;
			},1000);
		},'json')
		.fail(function(jqXHR) {
			monitor.log('GET failed: ' + jqXHR.responseText);
			monitor.log('server has no data; killing monitor');
		});
	}
			
	// Hook up the pause button
	var paused = false;
	$('button[name=pause]').click(function() {
		if(paused) {
			setTimeout(initializeSync,1000);
			$(this).html('Running');
		} else {
			clearInterval(syncinterval);
			clearTimeout(resynctimeout);
			$(this).html('Stopped');
		}
		paused = !paused;
	});
	
	initializeSync();
}

})();
