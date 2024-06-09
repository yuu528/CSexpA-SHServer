<?php
if(isset($_POST['start'])) {
	$filename = getcwd() . '/status.json';

	if(file_exists($filename)) {
		$data = json_decode(file_get_contents($filename), true);
		if($data['status'] != 1) { // not ready
			exit;
		}
	}

	$data = array(
		'status' => 2 // working
	);

	file_put_contents($filename, json_encode($data));
	system(getcwd() . '/pour.py >/dev/null 2>/dev/null &');
}
?>
