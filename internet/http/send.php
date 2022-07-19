<?php
include("connect.php");

try {
	$presses = 0;
	$users = 0; 

	$conn = new PDO("mysql:host=$servername;dbname=$dbname", $username, $password);
	$conn->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
	

	$stmt = $conn->prepare("SELECT * FROM requests WHERE sent='1' AND ModifiedTime > NOW() - INTERVAL 5 SECOND LIMIT 1"); 
	$stmt->execute();
	$result = $stmt->fetchAll();
	
	if (count($result) > 0 ) {
		Print("Ratelimiting.");
	} else {
		$conn->beginTransaction();
		
		//discard anything older than 60 seconds
		$stmt = $conn->prepare("UPDATE requests SET sent='1',discarded='1' WHERE sent='0' AND CreatedTime < NOW() - INTERVAL 60 SECOND"); 
		$stmt->execute();
		
		//find unique users in the last 60 seconds
		$stmt = $conn->prepare("SELECT COUNT(DISTINCT ip) as DistinctUsers FROM requests WHERE CreatedTime > NOW() - INTERVAL 60 SECOND");
		$stmt->execute();
		$result = $stmt->fetchAll();
		
		$users = intval($result[0]["DistinctUsers"]);
		


		if ($users > 0){
			// count how many presses are left to send
			$stmt = $conn->prepare("SELECT SUM(count) as ButtonPresses FROM requests WHERE sent='0'");  
			$stmt->execute();
			$result = $stmt->fetchAll();
			$presses = intval($result[0]["ButtonPresses"]);
			
			// mark as handled
			$stmt = $conn->prepare("UPDATE requests SET sent='1' WHERE sent='0'"); 
			$stmt->execute();
		}
		
		$conn->commit();

		
		if ($presses > 0){
			print("Sending");	
			print("Users: ");	
			var_dump($users); 
			print("Presses: ");	
			var_dump($presses);
			$insta_presses = 0;
			$click_delay = 0;
			
			if ($presses > 100){
				$insta_presses = $presses - 100;
				$presses = 100;
				
			}
			if ($presses < 10){
				$click_delay = 100;
			}
			elseif ($presses < 20){
				$click_delay = 50;
			}	
			elseif ($presses < 30){
				$click_delay = 33;
			}	
			elseif ($presses < 40){
				$click_delay = 25;
			}	
			elseif ($presses < 50){
				$click_delay = 10;
			}	
			// peak message rate at delay 0 is 100 in 5 seconds.
			
			$ch = curl_init();

			curl_setopt($ch, CURLOPT_URL, 'https://api.particle.io/v1/devices/events');
			curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
			curl_setopt($ch, CURLOPT_POST, 1);
			curl_setopt($ch, CURLOPT_POSTFIELDS, "name=web_button&data=" . $presses . "," . $click_delay . "," . $insta_presses . "," . $users . "&private=true&ttl=60&access_token=");

			$headers = array();
			$headers[] = 'Content-Type: application/x-www-form-urlencoded';
			curl_setopt($ch, CURLOPT_HTTPHEADER, $headers);

			$result = curl_exec($ch);
			var_dump($result);
			if (curl_errno($ch)) {
				echo 'Error:' . curl_error($ch);
			}
			curl_close($ch);
			
		}else{
			
			print("Nothing to do");	
			print("Users: ");	
			var_dump($users); 
			print("Presses: ");	
			var_dump($presses);
			
		}
	}
}
catch (PDOException $e) {
	print "Error: " . $e->getMessage();
}
$conn = null;

?>


