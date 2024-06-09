window.addEventListener('DOMContentLoaded', () => {
	document.getElementById('load-btn').addEventListener('click', () => {
		const xhr = new XMLHttpRequest();
		xhr.open('GET', '/ajax/test.json');
		xhr.addEventListener('load', () => {
			let note = document.getElementById('note');

			if(xhr.status === 200) {
				const data = JSON.parse(xhr.responseText);

				let table = document.getElementById('result-table');

				Object.entries(data).forEach(([key, value]) => {
					let tr = document.createElement('tr');
					let td_key = document.createElement('td');
					let td_value = document.createElement('td');

					td_key.innerText = key;
					td_value.innerText = value;

					tr.appendChild(td_key);
					tr.appendChild(td_value);
					table.appendChild(tr);
				});

				note.innerText = 'Data loaded successfully';
			} else {
				note.innerText = 'Failed to load data: ' + xhr.status + ' ' + xhr.statusText;
			}
		});
		xhr.send();
	});
});
