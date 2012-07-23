(function(){ // Keep everything as neat as possible

$(function(){
	panelbody = $('div#panels');
	
	panelbody.append(new Panel('alpha'));
});

var MAX_FAILURES = 5;

function Panel(car) {
	var panel = this; // Otherwise things get messy with nesting and such
	
	var potplot;
	var dataInterval;
	var potdata = [[]];
	
	panel.car = car;
	
	panel.attr({
		id: car + 'monitor',
		class: 'panel'
	});
	
	panel.append('<h1>' + car + '</h1>');
	panel.appendWidget('<div name="potentiometerplot"></div>');
	
	var potslider = $('<div></div>');
	potslider.append($('<input type="range" min="0" max="1.01" step="0.01" value="0.32">')
		.change(function(event){
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
			
			potslider.children('label').html(value + ' ' + unit + (value == 1 ? '' : 's'));
		})
		.css({width: '50%'}));
	potslider.append($('<label></label>')
		.css({float: 'right'}));
	panel.appendWidget(potslider);
	potslider.children('input').change();
	
	// Get an intial time, then update it once a second
	function initializeSync() {
		$.get('/data',{
				type: 'ajax',
				car: car
			})
		.done(function(datum){
				var failures = 0;
				var timeBase = datum.time;
				var time = 0;
				
				if(timeBase <= 0) {
					panel.log('server has no data; killing panel');
					return;
				}
				
				var syncinterval = setInterval(function(){
						$.get('/data',{
								type: 'ajax',
								car: car,
								time: timeBase + time
							})
						.done(function(datum){
								potdata.push([datum.time*1000,datum.potentiometer]);
	
								potplot = $.plot(panel.children('div[name=potentiometerplot]'),[potdata],{
									xaxis: {
										min: potdata[potdata.length - 1][0] - dataInterval,
										mode: 'time',
										timeformat: '%H:%M:%S %P',
										twelveHourClock: true
									},
									yaxis: {min: 0, max: 1023}
								});
								
								failures = 0;
							},'json')
						.fail(function(jqXHR){
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
		.fail(function(jqXHR){
			panel.log('GET failed: ' + jqXHR.responseText);
		});
	}
	
	initializeSync();
}

Panel.prototype = $('<div></div>');

Panel.prototype.appendWidget = function(widget) {
	widget = $(widget).addClass('onecolumn');
	this.append(widget);
}
	
Panel.prototype.log = function(msg) {
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
		.append('<p>' + timestamp + ': ' + this.car + ': ' + msg + '</p>')
		.animate({scrollTop: $('div#log')[0].scrollHeight});
}

})();
