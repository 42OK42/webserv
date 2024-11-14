/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: okrahl <okrahl@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/16 17:44:54 by okrahl            #+#    #+#             */
/*   Updated: 2024/11/14 16:29:51 by okrahl           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Router.hpp"
#include "Helper.hpp"
#include <sstream>
#include <ctime>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cerrno>
#include <vector>
#include <dirent.h>
#include <sys/stat.h>

Router::Router(ServerConfig& config) : _serverConfig(config) {}

Router::~Router() {}

std::string Router::readFile(const std::string& filepath) {
	std::ifstream file(filepath.c_str(), std::ios::binary);
	if (!file.is_open()) {
		return "<html><body><h1>404 Not Found</h1></body></html>";
	}
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

void Router::ensureDirectoryExists(const std::string& path) {
	size_t pos = 0;
	do {
		pos = path.find('/', pos + 1);
		std::string subpath = path.substr(0, pos);
		if (!subpath.empty()) {
			mkdir(subpath.c_str(), 0777);
		}
	} while (pos != std::string::npos);
}

std::string Router::extractFilenameFromUrl(const std::string& url) {
	size_t pos = url.find("filename=");
	if (pos != std::string::npos) {
		return url.substr(pos + 9);
	}
	return "";
}

std::string Router::extractFilename(const std::string& contentDisposition) {
	std::string filename;
	size_t pos = contentDisposition.find("filename=");
	if (pos != std::string::npos) {
		pos += 10;
		size_t endPos = contentDisposition.find("\"", pos);
		if (endPos != std::string::npos) {
			filename = contentDisposition.substr(pos, endPos - pos);
		}
	}
	return filename;
}

std::vector<std::string> Router::getFilesInDirectory(const std::string&) {
	std::vector<std::string> files;
	const std::map<std::string, Location>& locations = _serverConfig.getLocations();
	
	for (std::map<std::string, Location>::const_iterator it = locations.begin(); it != locations.end(); ++it) {
		std::string uploadDir = it->second.getRoot() + "/uploads/";
		std::string locationPath = it->first;  // z.B. "/upload" oder "/form"
		DIR* dir = opendir(uploadDir.c_str());
		if (dir) {
			struct dirent* entry;
			while ((entry = readdir(dir)) != NULL) {
				if (entry->d_type == DT_REG) {
					files.push_back(locationPath + "|" + it->second.getRoot() + "|" + entry->d_name);
				}
			}
			closedir(dir);
		}
	}
	return files;
}

void Router::handleRequest(const HttpRequest& request, HttpResponse& response) {
	std::string requestHost = request.getHeader("Host");
	std::ostringstream expectedHost;
	expectedHost << _serverConfig.getHost() << ":" << _serverConfig.getPort();

	if (requestHost != expectedHost.str()) {
		setErrorResponse(response, 400);
		return;
	}

	std::string path = request.getUrl();
	size_t queryPos = path.find('?');
	if (queryPos != std::string::npos) {
		path = path.substr(0, queryPos);
	}

	try {
		Location location = _serverConfig.findLocation(path);
		if (!location.isMethodAllowed(request.getMethod())) {
			setErrorResponse(response, 405);
			return;
		}

		if (request.getMethod() == "GET") {
			handleGET(request, response, location);
		}
		else if (request.getMethod() == "POST") {
			handlePOST(request, response, location);
		}
		else if (request.getMethod() == "DELETE") {
			handleDELETE(request, response, location);
		}
		else {
			setErrorResponse(response, 405);
		}
	} catch (const ServerConfig::LocationNotFound& e) {
		setErrorResponse(response, 404);
	}
}

void Router::handleGET(const HttpRequest& request, HttpResponse& response, const Location& location) {
	if (!location.isMethodAllowed("GET")) {
		setErrorResponse(response, 405);
		return;
	}

	std::string path = request.getUrl();
	size_t queryPos = path.find("?file=");
	
	if (queryPos != std::string::npos) {
		std::string filename = path.substr(queryPos + 6); // "file=" ist 5 Zeichen lang
		std::string uploadPath = location.getRoot() + "/uploads/" + filename;
		
		#ifdef DEBUG_MODE
		std::cout << "Suche Datei: " << uploadPath << std::endl;
		#endif
		
		struct stat statbuf;
		if (stat(uploadPath.c_str(), &statbuf) == 0 && S_ISREG(statbuf.st_mode)) {
			std::ifstream file(uploadPath.c_str(), std::ios::binary);
			if (file) {
				std::vector<char> buffer((std::istreambuf_iterator<char>(file)),
									   std::istreambuf_iterator<char>());
				
				response.setStatusCode(200);
				response.setBody(std::string(buffer.begin(), buffer.end()));
				
				std::string extension = filename.substr(filename.find_last_of(".") + 1);
				if (extension == "jpg" || extension == "jpeg")
					response.setHeader("Content-Type", "image/jpeg");
				else if (extension == "png")
					response.setHeader("Content-Type", "image/png");
				else if (extension == "gif")
					response.setHeader("Content-Type", "image/gif");
				return;
			}
		}
		setErrorResponse(response, 404);
		return;
	}

	if (!location.get_redirectTo().empty()) {
		response.setStatusCode(301);
		response.setHeader("Location", location.get_redirectTo());
		return;
	}

	std::string fullPath = location.getRoot();
	
	// Prüfe auf Bildanfrage im uploads Verzeichnis
	if (path.find("/uploads/") != std::string::npos) {
		std::string filename = path.substr(path.find_last_of("/") + 1);
		std::string uploadPath = location.getRoot() + "/uploads/" + filename;
		
		// Prüfe ob die Datei existiert
		struct stat statbuf;
		if (stat(uploadPath.c_str(), &statbuf) == 0 && S_ISREG(statbuf.st_mode)) {
			std::ifstream file(uploadPath.c_str(), std::ios::binary);
			if (file) {
				std::vector<char> buffer((std::istreambuf_iterator<char>(file)),
									   std::istreambuf_iterator<char>());
				
				response.setStatusCode(200);
				response.setBody(std::string(buffer.begin(), buffer.end()));
				
				// Setze den korrekten Content-Type basierend auf der Dateiendung
				std::string extension = filename.substr(filename.find_last_of(".") + 1);
				if (extension == "jpg" || extension == "jpeg")
					response.setHeader("Content-Type", "image/jpeg");
				else if (extension == "png")
					response.setHeader("Content-Type", "image/png");
				else if (extension == "gif")
					response.setHeader("Content-Type", "image/gif");
				return;
			}
		}
		setErrorResponse(response, 404);
		return;
	}

	// Normale GET-Request Verarbeitung
	struct stat statbuf;
	if (stat(fullPath.c_str(), &statbuf) == 0) {
		if (S_ISDIR(statbuf.st_mode)) {
			if (!location.getIndex().empty()) {
				std::string indexPath = fullPath + "/" + location.getIndex();
				if (stat(indexPath.c_str(), &statbuf) == 0 && S_ISREG(statbuf.st_mode)) {
					std::string content = readFile(indexPath);
					
					// Füge die Dateiliste für die uploadSuccessful Seite hinzu
					if (request.getUrl() == "/uploadSuccessful") {
						std::vector<std::string> files = getFilesInDirectory("");
						std::string fileListJS = "\n<script>\nconst fileList = [";
						for (size_t i = 0; i < files.size(); ++i) {
							if (i > 0) fileListJS += ",";
							fileListJS += "\n    \"" + files[i] + "\"";
						}
						fileListJS += "\n];\n</script>\n</head>";
						
						// Füge das Script vor dem schließenden </head> Tag ein
						size_t pos = content.find("</head>");
						if (pos != std::string::npos) {
							content.insert(pos, fileListJS);
						}
					}
					
					response.setStatusCode(200);
					response.setBody(content);
					response.setHeader("Content-Type", "text/html");
					return;
				}
			}
			if (location.getAutoIndex()) {
				response.setStatusCode(200);
				response.setBody(generateDirectoryListing(fullPath, request.getUrl()));
				response.setHeader("Content-Type", "text/html");
				return;
			}
		}
		else if (S_ISREG(statbuf.st_mode)) {
			response.setStatusCode(200);
			response.setBody(readFile(fullPath));
			response.setHeader("Content-Type", "text/html");
			return;
		}
	}
	setErrorResponse(response, 404);
}

void Router::handlePOST(const HttpRequest& request, HttpResponse& response, const Location& location) {
	std::string uploadDir = location.getRoot() + "/uploads/";
	ensureDirectoryExists(uploadDir);

	const std::vector<std::string>& filenames = request.getFilenames();
	const std::vector<std::string>& fileContents = request.getFileContents();
	
	if (!filenames.empty() && filenames.size() == fileContents.size()) {
		for (size_t i = 0; i < filenames.size(); ++i) {
			std::string fullPath = uploadDir + filenames[i];
			std::ofstream outFile(fullPath.c_str(), std::ios::binary);
			if (outFile.is_open()) {
				outFile.write(fileContents[i].c_str(), fileContents[i].length());
				outFile.close();
			}
		}
		
		response.setStatusCode(303);
		response.setHeader("Location", "/uploadSuccessful");
		response.setHeader("Content-Type", "text/html");
		response.setBody("<html><body>Redirecting to /uploadSuccessful</body></html>");
	} else {
		setErrorResponse(response, 400);
	}
}

void Router::handleDELETE(const HttpRequest& request, HttpResponse& response, const Location& location) {
	if (!location.isMethodAllowed("DELETE")) {
		setErrorResponse(response, 405);
		return;
	}

	std::string filename = extractFilenameFromUrl(request.getUrl());
	if (!filename.empty()) {
		size_t separatorPos = filename.find("|");
		if (separatorPos != std::string::npos) {
			std::string root = filename.substr(0, separatorPos);
			std::string file = filename.substr(separatorPos + 1);
			std::string fullPath = root + "/uploads/" + file;
			
			// Prüfe zuerst, ob die Datei existiert
			struct stat statbuf;
			if (stat(fullPath.c_str(), &statbuf) != 0) {
				// Datei existiert nicht - sende 204
				response.setStatusCode(204);
				response.setBody("File not found");
				response.setHeader("Content-Type", "text/plain");
				return;
			}
			
			// Versuche die Datei zu löschen
			if (remove(fullPath.c_str()) == 0) {
				response.setStatusCode(200);
				response.setBody("File deleted successfully");
			} else {
				response.setStatusCode(500);
				response.setBody("Error deleting file");
			}
		}
	} else {
		response.setStatusCode(400);
		response.setBody("No filename provided");
	}
	response.setHeader("Content-Type", "text/plain");
}

void Router::setErrorResponse(HttpResponse& response, int errorCode) {
	const std::map<int, std::string>& errorPages = _serverConfig.getErrorPages();
	std::map<int, std::string>::const_iterator errorPageIt = errorPages.find(errorCode);

	if (errorPageIt != errorPages.end()) {
		std::string errorPageContent = readFile(errorPageIt->second);
		response.setStatusCode(errorCode);
		response.setBody(errorPageContent);
		response.setHeader("Content-Type", "text/html");
		response.setHeader("Error-Page-Path", errorPageIt->second);
	} else {
		response.setStatusCode(errorCode);
		response.setBody("");
		response.setHeader("Content-Type", "text/html");
	}
}

std::string Router::generateDirectoryListing(const std::string& dirPath, const std::string& requestPath) {
	std::stringstream html;
	html << "<!DOCTYPE html>\n<html>\n<head>\n"
		 << "<title>Index of " << requestPath << "</title>\n"
		 << "<style>\n"
		 << "body { font-family: Arial, sans-serif; margin: 20px; }\n"
		 << "table { border-collapse: collapse; width: 100%; }\n"
		 << "th, td { text-align: left; padding: 8px; border-bottom: 1px solid #ddd; }\n"
		 << "th { background-color: #f2f2f2; }\n"
		 << "tr:hover { background-color: #f5f5f5; }\n"
		 << "a { text-decoration: none; color: #0366d6; }\n"
		 << "a:hover { text-decoration: underline; }\n"
		 << "</style>\n"
		 << "</head>\n<body>\n"
		 << "<h1>Index of " << requestPath << "</h1>\n"
		 << "<table>\n"
		 << "<tr><th>Name</th><th>Last Modified</th><th>Size</th></tr>\n";

	DIR* dir = opendir(dirPath.c_str());
	if (dir) {
		struct dirent* entry;
		std::vector<std::string> entries;

		while ((entry = readdir(dir)) != NULL) {
			entries.push_back(entry->d_name);
		}

		std::sort(entries.begin(), entries.end());

		for (std::vector<std::string>::const_iterator it = entries.begin();
			 it != entries.end(); ++it) {
			std::string name = *it;
			struct stat statbuf;
			std::string fullPath = dirPath + "/" + name;

			if (stat(fullPath.c_str(), &statbuf) == 0) {
				bool isDir = S_ISDIR(statbuf.st_mode);

				std::ostringstream size;
				if (isDir)
					size << "-";
				else
					size << statbuf.st_size;

				char timeStr[80];
				strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S",
						localtime(&statbuf.st_mtime));

				html << "<tr><td><a href=\""
					 << (requestPath == "/" ? "" : requestPath) << "/" << name
					 << (isDir ? "/" : "") << "\">" << name
					 << (isDir ? "/" : "") << "</a></td>"
					 << "<td>" << timeStr << "</td>"
					 << "<td>" << size.str() << "</td></tr>\n";
			}
		}
		closedir(dir);
	} else {
		html << "<tr><td colspan='3'>Error reading directory</td></tr>\n";
	}

	html << "</table>\n</body>\n</html>";
	return html.str();
}

std::string Router::getCurrentTimestamp() const {
	std::time_t now = std::time(NULL);
	char buffer[20];
	std::strftime(buffer, sizeof(buffer), "%Y%m%d_%H%M%S", std::localtime(&now));
	return std::string(buffer);
}