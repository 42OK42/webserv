document.addEventListener('DOMContentLoaded', () => {
	fetch('/listUploads') // Endpoint to get the list of files
		.then(response => response.json())
		.then(data => {
			const fileList = document.getElementById('fileList');
			data.files.forEach(file => {
				const li = document.createElement('li');
				li.textContent = file;

				const deleteButton = document.createElement('button');
				deleteButton.textContent = 'Delete';
				deleteButton.onclick = () => deleteFile(file);

				li.appendChild(deleteButton);
				fileList.appendChild(li);
			});
		})
		.catch(error => console.error('Error fetching file list:', error));
});

function deleteFile(filename) {
	if (confirm(`Are you sure you want to delete ${filename}?`)) {
		fetch(`/upload?filename=${encodeURIComponent(filename)}`, {
			method: 'DELETE'
		})
		.then(response => response.text())
		.then(data => {
			alert(data);
			location.reload();
		})
		.catch(error => console.error('Error:', error));
	}
}