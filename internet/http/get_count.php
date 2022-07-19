<?php
include("connect.php");

try {
    $conn = new PDO("mysql:host=$servername;dbname=$dbname", $username, $password);
    $conn->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
    
	$stmt = $conn->prepare("SELECT * FROM live ORDER BY id DESC LIMIT 1");
    $stmt->execute();
    $result = $stmt->fetchAll();
	
	
	
    if (count($result) > 0 ) {
		$row = $result[0];
		
		// if the power gets pulled on a button while it's in use, these values should get zeroed
		
		if (time() - strtotime($row["timestamp"]) > 60){
			$row["web_users"] = 0;
			$row["phy_users"] = 0;
		}
	
		echo $row["timestamp"] . " GMT\t" . $row["phy_count"] . "\t" . $row["web_count"] . "\t" .  $row["phy_users"] . "\t" .  $row["web_users"]. "\t" .  $row["battery"] . "\t" .  $row["strength"];
	} 
  
}
catch (PDOException $e) {
    print "Error: " . $e->getMessage();
}
$conn = null;

?>