var exceptionTemplate = 
	  '<li class="exception" id="">'
	+ '	<form>'
	+ '		<input name="action" type="hidden" value="delete_exception">'
	+ '		<input name="key" type="hidden" value="">'
	+ '		<label class="email_label"></label>'
	+ '		<label class="authorized_label"></label>'
	+ '		<input type="submit" value="&#215;">'
	+ '	</form>'
	+ '</li>'
	;

function createExceptionDOM(key, email, authorized)
{
	var exception = $(exceptionTemplate);
	
	// Insert data
	exception.attr('id',key);
	exception.find('input[name=key]').val(key);
	exception.find('.email_label').text(email);
	exception.find('.authorized_label').text(authorized ? "authorized" : "not authorized");
	
	exception.children('form').submit(handleExceptionSubmit);
	
	return exception;
}

function handleExceptionSubmit(event){
	// Do it my way!
	event.preventDefault();
	
	// Talk to the server and get a response
	$.post('/admin',$(this).serialize(),function(data){
		// Update the page
		
		if(data.added.key != undefined && data.added.key != "")
		{
			exception = createExceptionDOM(data.added.key,data.added.email,data.added.authorized);
			exception.hide();
			$('#exception_list').prepend(exception);
			exception.slideDown();
		}
		
		if(data.deleted != undefined && data.deleted != "")
		{
			$('#' + data.deleted).animate({
				opacity: 0,
				height: 0
			},function(){
				$(this).remove();
			});
		}
	},'json')
	.error(function(){
		alert('Error POSTing to server.');
	});
	
	$(this).get(0).reset();
}

$(function(){
	// Transform the pre-existing exception list
	$('#exception_list > li').replaceWith(function(){
		// Extract data
		var key = $(this).children('.exception_key').text();
		var email = $(this).children('.exception_email').text();
		var authorized = $(this).children('.exception_authorized').text();
		
		return createExceptionDOM(key,email,authorized == "true");
	});
	
	$('form#add_exception').submit(handleExceptionSubmit);
});
