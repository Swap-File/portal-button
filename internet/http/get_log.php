<?php
include("connect.php");

try {
	
	$time1 = intval($_POST["time1"]) / 1000;
	$time2 = intval($_POST["time2"]) / 1000;
	
    $conn = new PDO("mysql:host=$servername;dbname=$dbname", $username, $password);
    $conn->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
		

	$stmt = $conn->prepare("SELECT * FROM live WHERE (timestamp BETWEEN FROM_UNIXTIME(:time1) AND FROM_UNIXTIME(:time2)) LIMIT 5000");
	$stmt->bindParam(':time1', $time1, PDO::PARAM_INT);
	$stmt->bindParam(':time2', $time2, PDO::PARAM_INT);
	
    $stmt->execute();
    $result = $stmt->fetchAll();

    foreach ($result as $row) {
	    echo  $row["timestamp"] . " GMT\t" . $row["phy_count"] . "\t" . $row["web_count"] . "\t" .  $row["phy_users"] . "\t" .  $row["web_users"]. "\t" .  $row["battery"] . "\t" .  $row["strength"] . "\n";
	} 
}
catch (PDOException $e) {
    print "Error: " . $e->getMessage();
}
$conn = null;
?>