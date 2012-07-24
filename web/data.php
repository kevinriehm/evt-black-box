<?php
ob_start("ob_gzhandler");

// Useful for debugging
ini_set('display_errors',1);
error_reporting(E_ALL);

function kill_request($statuscode, $msg) {
	header(' ',true,$statuscode);
	ob_flush(); // Just to be sure
	die(is_null($msg) ? "request killed with status code " . $statuscode : $msg);
}

function check_car(&$car) {
	$car = strtolower($car);
	return $car == "alpha";
}

// Connect to the database
if(strpos($_SERVER['SERVER_NAME'],"3owl.com") !== false) // Remote server?
	$mysqli = new mysqli("mysql.3owl.com","u878764359_evt","uuxfXjK7Tc","u878764359_main");
else // Local server, hopefully
	$mysqli = new mysqli("localhost","root","evt","angel");

if($mysqli->connect_errno)
	kill_request(500,"cannot connect to MySQL database: (" . $mysqli->connect_errno . ") " . $mysqli->connect_error);

// Actually deal with the request
if($_SERVER["REQUEST_METHOD"] == "GET") { // Extracting data
	if(!check_car($_GET["car"]))
		kill_request(400,"bad value for car");
	
	if($_GET["type"] == "csv") {
		//header("Content-Type: text/csv; header=present");
		//header("Content-Disposition: attachment; filename=\"" + $_GET["car"] + "_" + $_GET["start"] + "_" + $_GET["end"] + ".csv\"");
	} elseif($_GET["type"] == "json") {
	} else kill_request(400,"bad value for type");
} elseif($_SERVER["REQUEST_METHOD"] == "POST") { // Adding data
	echo "hello";
}

ob_flush();
?>
