$(function(){
	$('form#add_exception, li.exception > form').submit(function(event){
		// Do it my way!
		event.preventDefault();
		
		// Talk to the server and get a response
		$.post('/admin',$(this).serialize(),function(data){
			alert(data);alert(data.added);alert(data.deleted);
		},'json')
		.error(function(){
			alert('Error POSTing to server.');
		});
	});
});
