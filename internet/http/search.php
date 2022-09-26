<?php
include("connect.php");

try {
	$conn = new PDO("mysql:host=$servername;dbname=$dbname", $username, $password);
	$conn->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
	
	$time = intval($_POST["time"]);
	if ($time == 0) {
		$time = time();
	}
	
	$stmt = $conn->prepare("SELECT id,DATE_FORMAT(timestamp,'%a %r') as time FROM live WHERE timestamp <= FROM_UNIXTIME(:time) ORDER BY id DESC LIMIT 1");
	$stmt->bindParam(':time', $time, PDO::PARAM_INT);
	$stmt->execute();
	$result = $stmt->fetchAll();

	if (count($result) == 0) {
		echo -1;
	}else{
		echo $result[0]['id'];
	}	
}
catch (PDOException $e) {
	print "Error: " . $e->getMessage();
}
$conn = null;

?>