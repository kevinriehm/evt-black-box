<?php ob_start("ob_gzhandler"); ?>
<!doctype html>
<html>
	<head>
		<title>STA EVT Angel</title>
		
		<meta name="application-name" content="Saint Thomas Academy Experimental Vehicle Team Angel Monitor">
		<link rel="icon" type="image/vnd.microsoft.iconless" href="favicon.ico">

		<link rel="stylesheet" type="text/css" href="css/monitor.css">
		
		<link rel="stylesheet" type="text/css" href="css/leaflet.css">

		<!--jQuery as the basic toolbox-->
		<script src="js/jquery.min.js"></script>

		<!--Flot for graphing-->
		<script src="js/jquery.flot.min.js"></script>
		<script src="js/jquery.ba-resize.min.js"></script>
		<script src="js/jquery.flot.resize.min.js"></script>
		
		<!--Leaflet for mapping-->
		<script src="js/leaflet.js"></script>
		
		<!--Custom code-->
		<script src="js/monitor.js"></script>
	</head>

	<body>
		<div id="header">
			<div name="title" class="left">Saint Thomas Academy Experimental Vehicle Team Angel Monitor</div>
		</div>

		<div id="panels">
			<!--Monitor panels go here (via Javascipt)-->
			<noscript>This site requires Javascript to be supported and enabled in your browser.</noscript>
		</div>
		
		<div id="logpanel" class="panel">
			<h1>Log</h1>
			<div id="log">
				<!--Messages go here (via Javascript)-->
			</div>
		</div>
	</body>
</html>
<?php ob_flush(); ?>
