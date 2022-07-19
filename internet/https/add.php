<?php
include("connect.php");

try {
	$conn = new PDO("mysql:host=$servername;dbname=$dbname", $username, $password);
	$conn->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);


	//get last count from log table
	$stmt = $conn->prepare("SELECT * FROM live ORDER BY id DESC LIMIT 1");
	$stmt->execute();
	$last_result = $stmt->fetchAll();
	


	$msgArray = explode(',', $_POST["data"]);

	if (count($msgArray) == 6){
		$phy_count = $msgArray[0];
		$web_count = $msgArray[1];
		$phy_users = $msgArray[2];
		$web_users = $msgArray[3];
		$battery = $msgArray[4];
		$strength = $msgArray[5];

		// handle an empty table
		$last_phy_count = 0;
		$last_web_count = 0;
		if (count($last_result) > 0 ) { // and compare timestamp (and or qty different?)
			$last_phy_count = $last_result[0]["phy_count"];
			$last_web_count = $last_result[0]["web_count"];
		}
		
		// don't let counts go backwards!
		if ($phy_count >= $last_phy_count and $web_count >= $last_web_count){
			
			//insert live count into live table
			$stmt = $conn->prepare("INSERT INTO live (ip,phy_count,web_count,phy_users,web_users,battery,strength) VALUES (:ip,:phy_count,:web_count,:phy_users,:web_users,:battery,:strength)");
			$stmt->bindParam(':ip', $_SERVER['REMOTE_ADDR'], PDO::PARAM_STR);
			$stmt->bindParam(':phy_count', $phy_count, PDO::PARAM_INT);
			$stmt->bindParam(':web_count', $web_count, PDO::PARAM_INT);
			$stmt->bindParam(':phy_users', $phy_users, PDO::PARAM_INT);
			$stmt->bindParam(':web_users', $web_users, PDO::PARAM_INT);
			$stmt->bindParam(':battery', $battery, PDO::PARAM_INT);
			$stmt->bindParam(':strength', $strength, PDO::PARAM_INT);
			$stmt->execute();		
			
			// for archive view, TBD
			if ($last_phy_count != 0 || $last_web_count != 0){
				//insert log count into log table		
				$stmt = $conn->prepare("INSERT INTO log (phy_count,web_count,phy_users,web_users,battery,strength) VALUES (:phy_count,:web_count,:phy_users,:web_users,:battery,:strength)");
				$log_phy_count = $phy_count - $last_phy_count;
				$log_web_count = $web_count - $last_web_count;		
				$stmt->bindParam(':phy_count', $log_phy_count, PDO::PARAM_INT);
				$stmt->bindParam(':web_count', $log_web_count, PDO::PARAM_INT);
				$stmt->bindParam(':phy_users', $phy_users, PDO::PARAM_INT);
				$stmt->bindParam(':web_users', $web_users, PDO::PARAM_INT);
				$stmt->bindParam(':battery', $battery, PDO::PARAM_INT);
				$stmt->bindParam(':strength', $strength, PDO::PARAM_INT);
				$stmt->execute();	
			}			
		}	
		
	}

	echo 'done';
}
catch (PDOException $e) {
	//header('HTTP/1.0 403 Forbidden');
	print "Error: " . $e->getMessage();
}
$conn = null;

?>