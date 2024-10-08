/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Helper.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: okrahl <okrahl@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/02 17:55:55 by okrahl            #+#    #+#             */
/*   Updated: 2024/10/08 16:26:09 by okrahl           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Helper.hpp"

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
		// Start after 'filename="'
		pos += 10;
		size_t endPos = contentDisposition.find("\"", pos);
		if (endPos != std::string::npos) {
			filename = contentDisposition.substr(pos, endPos - pos);
		}
	}
	return filename;
}