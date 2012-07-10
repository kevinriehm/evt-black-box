var MAX_FAILURES = 5;

$(function(){
	panelbody = $('div#panels');
	
	panelbody.append(newMonitorPanel('alpha'));
});

function newMonitorPanel(car) {
	var failures, timeBase, time;
	
	var panel = $(
		  '<div id="' + car + 'monitor" class="panel">'
		+ '    <h1>' + car + '</h1>'
		+ '    <div name="potentiometer"></div>'
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
	
	panel.log = function(msg){
		$('div#log')
			.append('<p>' + timestamp() + ': ' + car + ': ' + msg + '</p>')
			.animate({scrollTop: $('div#log')[0].scrollHeight});
	}
	
	var data = [[]];
	
	// Get an intial time, then update it once a second
	function initializeSync() {
		$.get('/data',{
				type: 'ajax',
				car: car
			})
		.done(function(datum){
				failures = 0;
				timeBase = datum.time;
				time = 0;
				
				if(timeBase <= 0) {
					panel.log('server has no data; killing panel');
					return;
				}
				
				var syncinterval = setInterval(function(){
						time++;
						
						$.get('/data',{
								type: 'ajax',
								car: car,
								time: timeBase + time
							})
						.done(function(datum){
								data[0].push([datum.time,datum.potentiometer]);
								$.plot($('div[name=potentiometer]'),data,{
										xaxis: {
												show: false,
												min: data[0][data[0].length - 1][0] - 60
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
					},1000);
			},'json')
		.fail(function(jqXHR){
			panel.log('GET failed: ' + jqXHR.responseText);
		});
	}
	
	initializeSync();
	
	return panel;
}
