<?php
include("connect.php");

try {
	$conn = new PDO("mysql:host=$servername;dbname=$dbname", $username, $password);
	$conn->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);

	$phy_count = 1;
	//insert live count into live table
	$stmt = $conn->prepare("INSERT INTO requests (ip,count) VALUES (:ip,:count)");
	$stmt->bindParam(':ip', $_SERVER['REMOTE_ADDR'], PDO::PARAM_STR);
	$stmt->bindParam(':count', $phy_count, PDO::PARAM_INT);
	$stmt->execute();		
	
	echo 'done';
}
catch (PDOException $e) {
	header('HTTP/1.0 403 Forbidden');
	print "Error: " . $e->getMessage();
}
$conn = null;

?>