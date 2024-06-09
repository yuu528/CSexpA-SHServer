document.addEventListener('DOMContentLoaded', () => {
	// dont use setInterval to avoid multiple requests
	setTimeout(updateStatusLoop, 500);

	document.getElementById('start-btn').addEventListener('click', () => {
		let xhr = new XMLHttpRequest();
		xhr.open('POST', '/iotpot/pot.php');
		xhr.onload = () => {
			if (xhr.status === 200) {
				updateStatus();
			}
		};
		xhr.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
		xhr.send('start=1');
	});
});

function updateStatusLoop() {
	updateStatus();
	setTimeout(updateStatusLoop, 500);
}

function updateStatus() {
	let xhr = new XMLHttpRequest();
	xhr.open('GET', '/iotpot/get_status.php');
	xhr.onload = () => {
		if (xhr.status === 200) {
			let data = JSON.parse(xhr.responseText);

			if('status' in data && 1 <= data.status && data.status <= 2) {
				updateStatusView(data.status);

				if('progress' in data) {
					document.getElementById('progress').value = data.progress;
				}
			} else {
				updateStatusView(0);
			}
		} else {
			updateStatusView(0);
		}
	};
	xhr.send();
}

function updateStatusView(id) {
	let status = document.getElementById('status');

	switch(id) {
		case 0: // not connected
			status.innerText = 'Not connected';
			document.getElementById('start-btn').disabled = true;
		break;
		case 1: // ready
			status.innerText = 'Ready';
			document.getElementById('start-btn').disabled = false;
		break;
		case 2: // working
			status.innerText = 'Working';
			document.getElementById('start-btn').disabled = true;
		break;
	}
}
