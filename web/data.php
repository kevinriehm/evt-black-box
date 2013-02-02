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

function check_cars($cars) {
	if(!is_array($cars))
		return false;

	foreach($cars as $key => $car)
		if(!check_car($cars[$key]))
			return false;

	return true;
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
$sqlite = new SQLite3("angel.sqlite3",SQLITE3_OPEN_READONLY);

// Actually deal with the request
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
			$result = $sqlite->querySingle("SELECT * FROM cars WHERE car = '" . $car . "' ORDER BY time DESC LIMIT 1",true);
		else // Otherwise, be specific
			$result = $sqlite->querySingle("SELECT * FROM  cars WHERE car = '" . $car . "' AND time = '" . $time . "' LIMIT 1",true);

		// Save the entry, if it exists
		if(!empty($result)) {
			$havedata = true;

			$response[$car] = $result;
			if($time === false) $time = $response[$car]["time"];
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

	$result = $sqlite->query("SELECT * FROM $car WHERE time >= $start AND time <= $end ORDER BY time");

	if(empty($result) || $result->numColumns == 0)
		kill_request(404,"cannot find entries");

	// Give specifics about this file
	header("Content-Type: text/csv; header=present");
	header("Content-Disposition: attachment; filename=\"$car_$start_$end.csv\"");

	// CSV header
	for($i = 0; $i < $result->numColumns(); i++)
		echo ($i > 0 ? "," : "") . ucwords($result->columnName($i));
	echo "\n";

	// CSV body
	while($row = $result->fetchArray(SQLITE3_NUM)) {
		foreach($row as $i => $field)
			echo ($i > 0 ? "," : "") . $field;
		echo "\n";
	}

	$result->finalize();
	break;

default: kill_request(400,"bad value for format"); break;
}

$sqlite->close();

ob_flush();
?>
