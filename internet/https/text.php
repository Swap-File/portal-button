<?php

try {
		
		$ch = curl_init();

		curl_setopt($ch, CURLOPT_URL, 'https://api.particle.io/v1/devices//web_text');
		curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
		curl_setopt($ch, CURLOPT_POST, 1);
		curl_setopt($ch, CURLOPT_POSTFIELDS, "name=" . $_POST["new_text"] . "&access_token=");

		$headers = array();
		$headers[] = 'Content-Type: application/x-www-form-urlencoded';
		curl_setopt($ch, CURLOPT_HTTPHEADER, $headers);

		$result = curl_exec($ch);
		var_dump($result);
		if (curl_errno($ch)) {
			echo 'Error:' . curl_error($ch);
		}
		curl_close($ch);
			
	
}
catch (PDOException $e) {
	print "Error: " . $e->getMessage();
}
?>


