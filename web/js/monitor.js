var MAX_FAILURES = 5;

$(function(){
	panelbody = $('div#panels');
	
	panelbody.append(newMonitorPanel('alpha'));
});

function newMonitorPanel(car)
{
	var failures, timeBase, time;
	
	var panel = $(
		  "<div id=\"" + car + "monitor\">"
		+ "	<p name=\"potentiometer\"></p>"
		+ "</div>"
		);
	
	panel.log = function(msg){
		$('div#log').append('<p>' + car + ': ' + msg + '</p>');
	}
	
	// Get an intial time, then update it once a second
	$.get('/data',{
		type: 'ajax',
		car: car
	},function(data){
		failures = 0;
		timeBase = data.time;
		time = 0;
		
		panelInterval = setInterval(function(){
			time++;
			
			$.get('/data',{
				type: 'ajax',
				car: car,
				time: timeBase + time
			},function(data){
				panel.find('p[name=potentiometer]').html('' + (timeBase + time) + data.potentiometer);
				
				failures = 0;
			},'json')
			.fail(function(jqXHR){
				panel.log('GET failed: ' + jqXHR.responseText);
				
				if(++failures >= MAX_FAILURES)
				{
					panel.log(MAX_FAILURES + ' sequential failures; killing panel');
					clearInterval(panelInterval);
				}
			});
		},1000);
	},'json')
	.fail(function(jqXHR){
		panel.log('GET failed: ' + jqXHR.responseText);
	});
	
	return panel;
}
