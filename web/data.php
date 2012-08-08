<?php
ob_start("ob_gzhandler"); // GZip!

// Useful for debugging
ini_set("display_errors",1);
error_reporting(E_ALL);

// More elaborate than simple die()
function kill_request($statuscode, $msg) {
	header(' ',true,$statuscode);
	ob_flush(); // Just to be sure
	die((is_null($msg) ? "request killed with status code " . $statuscode : $msg) . "\n");
}

function check_car(&$car) {
	$car = strtolower($car);
	return $car == "alpha" || $car == "beta";
}

function check_cars(&$cars) {
	if(!is_array($cars))
		return false;
	
	foreach($cars as $key => $car)
		if(!check_car(&$cars[$key]))
			return false;
	
	return true;
}

// This forgoes speed to thwart timing attacks
function constant_time_streq($a, $b) {
	if(strlen($a) != strlen($b))
		return false;
	
	$result = 0;
	for($i = 0; $i < strlen($a); $i++) {
		$result |= $a[$i] ^ $b[$i];
	}
	return $result == 0;
}

// Safely validates a request variable, or dies trying
function check_request_var($varname, $testfunc = NULL, $killonfail = true) {
	if(array_key_exists($varname,$_REQUEST) && (is_null($testfunc) ? true : $testfunc($_REQUEST[$varname])))
		return $_REQUEST[$varname];
	else if($killonfail)
		kill_request(400,"bad value for " . $varname);
	else return false;
}

// Connect to the database
if(strpos($_SERVER['SERVER_NAME'],"3owl.com") !== false) // Remote server
	$mysqli = new mysqli("mysql.3owl.com","u878764359_evt","uuxfXjK7Tc","u878764359_main");
else // Local server, presumably
	$mysqli = new mysqli("localhost","root","evt","angel");

if($mysqli->connect_errno)
	kill_request(500,"cannot connect to MySQL database: (" . $mysqli->connect_errno . ") " . $mysqli->connect_error);

// Actually deal with the request
if($_SERVER["REQUEST_METHOD"] == "GET") { // Extracting data
	$format = check_request_var("format");
	switch($format) {
		case "ajax":
		$cars = check_request_var("cars","check_cars");
		$time = check_request_var("time","is_numeric",false);
		
		$havedata = false;
		$response = array();
		foreach($cars as $car) {
			$result;
			if($time === false) // Return the most recent entry?
				$result = $mysqli->query("SELECT * FROM " . $car . " ORDER BY time DESC LIMIT 1");
			else // Otherwise, be specific
				$result = $mysqli->query("SELECT * FROM " . $car . " WHERE time = " . $time . " LIMIT 1");
			
			// Save the entry, if it exists
			if($result !== false && $result->num_rows != 0) {
				$havedata = true;
				
				$response[$car] = $result->fetch_object();
				if($time === false) $time = $response[$car]->time;
				
				$result->free();
			}
		}
		
		// Only die if none of the cars have the right entry
		if(!$havedata)
			kill_request(404,"cannot find entry" . ($time === false ? "" : " " . $time));
		
		$response["time"] = $time; // For convenience
		
		header("Content-Type: application/json");
		echo json_encode($response);
		break;
		
		case "csv":
		$car = check_request_var("car","check_car");
		$start = check_request_var("start","is_numeric");
		$end = check_request_var("end","is_numeric");
		
		$result = $mysqli->query("SELECT * FROM " . $car . " WHERE time >= " . $start . " AND time <= " . $end . " ORDER BY time");
		
		if($result === FALSE)
			kill_request(404,"cannot find entries");
		
		// Give specifics about this file
		header("Content-Type: text/csv; header=present");
		header("Content-Disposition: attachment; filename=\"" + $car + "_" + $start + "_" + $end + ".csv\"");
		
		// CSV header
		foreach($result->fetch_fields() as $i => $field)
			echo ($i > 0 ? "," : "") . ucwords($field->name);
		echo "\n";
		
		// CSV body
		while($row = $result->fetch_row()) {
			foreach($row as $i => $field)
				echo ($i > 0 ? "," : "") . $field;
			echo "\n";
		}
		
		$result->free();
		break;
		
		default: kill_request(400,"bad value for format"); break;
	}
} elseif($_SERVER["REQUEST_METHOD"] == "POST") { // Adding data
	// Authenticate the message
	$headers = getallheaders();
	$message = file_get_contents("php://input");
	$hmackey = "\xF5\x2B\x58\x46\x1A\x02\xC9\xFE"
	         . "\xF8\xA6\x6F\xD3\xE0\xC8\x9C\xB7"
	         . "\xDA\x42\x2C\x38\xC0\xCA\xD1\x9A"
	         . "\x94\x47\x6F\x74\x98\x63\x99\xB3";
	$hmac = hash_hmac("sha256",$message,$hmackey);
	if(!constant_time_streq($hmac,$headers["Authorization"]))
		kill_request(401,"bad authorization");
	
	// Validate the data
	$car = check_request_var("car","check_car");
	$time = check_request_var("time","is_numeric");
	$potentiometer = check_request_var("potentiometer","is_numeric");
	$latitude = check_request_var("latitude","is_numeric");
	$longitude = check_request_var("longitude","is_numeric");

	// Make sure a table exists for this car
	$mysqli->query("CREATE TABLE IF NOT EXISTS " . $car
		. "(time BIGINT, potentiometer SMALLINT, latitude DECIMAL(9,6), longitude DECIMAL(9,6), PRIMARY KEY (time))");
	
	// Insert the data, as long as it isn't already there
	$mysqli->query("INSERT INTO " . $car
		. " VALUES (" . $time . ", " . $potentiometer . ", " . $latitude . ", " . $longitude . ")");
}

ob_flush();
?>
