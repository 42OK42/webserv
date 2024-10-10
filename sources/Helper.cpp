/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Helper.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: okrahl <okrahl@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/02 17:55:55 by okrahl            #+#    #+#             */
/*   Updated: 2024/10/10 18:24:09 by okrahl           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Helper.hpp"

pthread_mutex_t upload_mutex = PTHREAD_MUTEX_INITIALIZER;

void ensureDirectoryExists(const std::string& path) {
	struct stat info;
	if (stat(path.c_str(), &info) != 0) {
		mkdir(path.c_str(), 0777);
	}
}

std::string readFile(const std::string& filePath) {
	std::ifstream file(filePath.c_str());
	if (!file.is_open()) {
		return "<html><body><h1>404 Not Found</h1></body></html>";
	}
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

std::string extractFilename(const std::string& contentDisposition) {
	std::string filename;
	size_t pos = contentDisposition.find("filename=");
	if (pos != std::string::npos) {
		pos += 10; // Start after 'filename="'
		size_t endPos = contentDisposition.find("\"", pos);
		if (endPos != std::string::npos) {
			filename = contentDisposition.substr(pos, endPos - pos);
		}
	}
	return filename;
}

std::string extractFilenameFromUrl(const std::string& url) {
	size_t pos = url.find("filename=");
	if (pos != std::string::npos) {
		return url.substr(pos + 9);
	}
	return "";
}

std::string generateUniqueFilename(const std::string& uploadDir, const std::string& baseName, std::vector<std::string>& uploadedFiles) {
	pthread_mutex_lock(&upload_mutex);

	std::string uniqueName = baseName;
	int counter = 1;

	// Extrahiere Dateierweiterung
	size_t dotPos = baseName.find_last_of('.');
	std::string namePart = baseName.substr(0, dotPos);
	std::string extension = (dotPos != std::string::npos) ? baseName.substr(dotPos) : "";

	// Überprüfen, ob der Dateiname bereits verwendet wird
	while (std::find(uploadedFiles.begin(), uploadedFiles.end(), uniqueName) != uploadedFiles.end() || std::ifstream((uploadDir + "/" + uniqueName).c_str()).good()) {
		std::ostringstream newName;
		newName << namePart << "_" << counter << extension;
		uniqueName = newName.str();
		counter++;
	}

	pthread_mutex_unlock(&upload_mutex);

	return uniqueName;
}