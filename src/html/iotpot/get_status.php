<?php
$filename = getcwd() . '/status.json';

header('Content-Type: application/json');

if(file_exists($filename)) {
	echo file_get_contents($filename);
} else {
	$data = array(
		'status' => 1
	);

	$json = json_encode($data);
	file_put_contents($filename, $json);
	echo json_encode($data);
}
?>
