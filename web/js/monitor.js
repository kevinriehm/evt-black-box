(function(){ // Keep everything as neat as possible

var MAX_FAILURES = 5;

$(function(){
	panelbody = $('div#panels');
	
	panelbody.append(newMonitorPanel('alpha'));
});

function newMonitorPanel(car) {
	var panel = $(
		  '<div id="' + car + 'monitor" class="panel">'
		+ '    <h1>' + car + '</h1>'
		+ '    <div name="potentiometer" class="onecolumn"></div>'
		+ '    <div name="potentiometerslider" class="onecolumn" style="height: 20px;"><div></div></div>'
		+ '</div>'
		);
	
	function timestamp() {
		function pad0(value, width) {
			var str = String(value);
			while(str.length < width)
				str = '0' + str;
			return str;
		}
		
		var date = new Date();
		return pad0(date.getHours(),2)
			+ ':' + pad0(date.getMinutes(),2)
			+ ':' + pad0(date.getSeconds(),2)
			+ '.' + pad0(date.getMilliseconds(),3);
	}
	
	panel.log = function(msg) {
		$('div#log')
			.append('<p>' + timestamp() + ': ' + car + ': ' + msg + '</p>')
			.animate({scrollTop: $('div#log')[0].scrollHeight});
	}
	
	
	
	var potdata = [];
	
	// Get an intial time, then update it once a second
	function initializeSync() {
		$.get('/data',{
				type: 'ajax',
				car: car
			})
		.done(function(datum){
				var dataInterval;
				var failures = 0;
				var timeBase = datum.time;
				var time = 0;
				
				if(timeBase <= 0) {
					panel.log('server has no data; killing panel');
					return;
				}
				
				$('div[name=potentiometerslider] > div').slider({
						min: 2,
						max: 24*60*60,
						value: 60,
						slide: function(event, ui) {
								dataInterval = ui.value;
							}
					});
				
				var syncinterval = setInterval(function(){
						$.get('/data',{
								type: 'ajax',
								car: car,
								time: timeBase + time
							})
						.done(function(datum){
								potdata.push([datum.time,datum.potentiometer]);
								$.plot($('div[name=potentiometer]'),[potdata],{
										xaxis: {
												show: false,
												min: potdata[potdata.length - 1][0] - dataInterval
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
	
	return panel;
}

})();
