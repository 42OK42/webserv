/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: okrahl <okrahl@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/16 17:44:54 by okrahl            #+#    #+#             */
/*   Updated: 2024/11/05 17:32:20 by okrahl           ###   ########.fr       */
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

void Router::initializeRoutes() {
	addRoute("/", &Router::handleHomeRoute);
	addRoute("/upload", &Router::handleUploadRoute);
	addRoute("/uploadSuccessful", &Router::handleUploadSuccessRoute);
}

void Router::handleRequest(const HttpRequest& request, HttpResponse& response) {
	// Prüfe zuerst den Host-Header
	std::string requestHost = request.getHeader("Host");
	
	std::ostringstream expectedHost;
	expectedHost << _serverConfig.getHost() << ":" << _serverConfig.getPort();
	
	if (requestHost != expectedHost.str()) {
		std::cout << "\033[0;31m[Router]\033[0m Invalid host: " << requestHost << std::endl;
		setErrorResponse(response, 400);  // Bad Request
		return;
	}

	std::string path = request.getUrl();
	size_t queryPos = path.find('?');
	if (queryPos != std::string::npos) {
		path = path.substr(0, queryPos);
	}

	try {
		Location location = _serverConfig.findLocation(path);
		std::string fullPath = location.getRoot();
		
		// Debug-Ausgaben
		std::cout << "\033[0;33m[Router]\033[0m Checking path: " << path << std::endl;
		std::cout << "\033[0;33m[Router]\033[0m Full path: " << fullPath << std::endl;
		std::cout << "\033[0;33m[Router]\033[0m AutoIndex: " << (location.getAutoIndex() ? "on" : "off") << std::endl;
		
		struct stat statbuf;
		if (stat(fullPath.c_str(), &statbuf) == 0) {
			std::cout << "\033[0;33m[Router]\033[0m Path exists" << std::endl;
			if (S_ISDIR(statbuf.st_mode)) {
				std::cout << "\033[0;33m[Router]\033[0m Path is a directory" << std::endl;
				
				// Prüfe auf index-Datei
				std::string indexPath = fullPath;
				if (!location.getIndex().empty()) {
					indexPath += "/" + location.getIndex();
					std::cout << "\033[0;33m[Router]\033[0m Checking for index: " << indexPath << std::endl;
					
					if (stat(indexPath.c_str(), &statbuf) == 0 && S_ISREG(statbuf.st_mode)) {
						std::cout << "\033[0;33m[Router]\033[0m Using index file" << std::endl;
						std::string content = readFile(indexPath);
						response.setStatusCode(200);
						response.setBody(content);
						response.setHeader("Content-Type", "text/html");
						return;
					}
				}
				
				// AutoIndex Check
				if (location.getAutoIndex()) {
					std::cout << "\033[0;33m[Router]\033[0m Generating directory listing for: " << fullPath << std::endl;
					std::string dirListing = generateDirectoryListing(fullPath, path);
					response.setStatusCode(200);
					response.setBody(dirListing);
					response.setHeader("Content-Type", "text/html");
					return;
				}
			}
		} else {
			std::cout << "\033[0;31m[Router]\033[0m Path does not exist: " << fullPath << std::endl;
		}

		// Normale Request-Verarbeitung fortsetzen
		if (!location.isMethodAllowed(request.getMethod())) {
			setErrorResponse(response, 405);
			return;
		}

		std::map<std::string, RouteHandler>::const_iterator it = routes.find(path);
		if (it != routes.end()) {
			RouteHandler handler = it->second;
			(this->*handler)(request, response);
		} else {
			setErrorResponse(response, 404);
		}
	} catch (const ServerConfig::LocationNotFound& e) {
		setErrorResponse(response, 404);
	}
}

void Router::handleHomeRoute(const HttpRequest& req, HttpResponse& res) {
	if (req.getMethod() == "GET") {
		Location location = _serverConfig.findLocation("/");
		std::string root = location.getRoot();
		std::string index = location.getIndex();
		std::string fullPath = root + "/" + index;
		
		std::cout << "\033[0;33m[Router]\033[0m Serving home page from: " << fullPath << std::endl;
		std::string content = readFile(fullPath);
		res.setStatusCode(200);
		res.setBody(content);
		res.setHeader("Content-Type", "text/html");
	} else {
		setErrorResponse(res, 405);
	}
}

void Router::handleUploadRoute(const HttpRequest& req, HttpResponse& res) {
	if (req.getMethod() == "GET") {
		Location location = _serverConfig.findLocation("/upload");
		std::string root = location.getRoot();
		std::string index = location.getIndex();
		std::string fullPath = root + "/" + index;
		
		std::cout << "\033[0;33m[Upload]\033[0m Serving upload form from: " << fullPath << std::endl;
		std::string content = readFile(fullPath);
		res.setStatusCode(200);
		res.setBody(content);
		res.setHeader("Content-Type", "text/html");
	}
	else if (req.getMethod() == "POST") {
		std::string uploadDir = ServerConfig::getUploadDir();
		const std::vector<std::string>& filenames = req.getFilenames();
		
		if (filenames.empty()) {
			setErrorResponse(res, 400);
			return;
		}
		
		saveUploadedFiles(req, uploadDir);
		
		res.setStatusCode(302);
		res.setHeader("Location", "/uploadSuccessful");
	}
}

void Router::handleUploadSuccessRoute(const HttpRequest& req, HttpResponse& res) {
	if (req.getMethod() == "GET") {
		Location location = _serverConfig.findLocation("/uploadSuccessful");
		std::string root = location.getRoot();
		std::string index = location.getIndex();
		std::string fullPath = root + "/" + index;
		
		std::cout << "\033[0;33m[Upload]\033[0m Serving success page from: " << fullPath << std::endl;
		std::string successContent = readFile(fullPath);
		
		std::vector<std::string> files = getFilesInDirectory(ServerConfig::getUploadDir());
		std::ostringstream json;
		json << "[";
		for (size_t i = 0; i < files.size(); ++i) {
			if (i > 0) json << ", ";
			json << "\"" << files[i] << "\"";
		}
		json << "]";

		std::string script = "<script>const fileList = " + json.str() + ";</script>";
		size_t pos = successContent.find("</head>");
		if (pos != std::string::npos) {
			successContent.insert(pos, script);
		}

		res.setStatusCode(200);
		res.setBody(successContent);
		res.setHeader("Content-Type", "text/html");
	}
	else if (req.getMethod() == "DELETE") {
		std::string filename = extractFilenameFromUrl(req.getUrl());
		if (filename.empty()) {
			setErrorResponse(res, 400);
			return;
		}

		std::string filePath = ServerConfig::getUploadDir() + filename;
		
		if (remove(filePath.c_str()) == 0) {
			res.setStatusCode(200);
			res.setBody("File Deleted Successfully");
		} else {
			setErrorResponse(res, 404);
		}
		res.setHeader("Content-Type", "text/plain");
	}
	else {
		setErrorResponse(res, 405);
	}
}

void Router::setErrorResponse(HttpResponse& response, int errorCode) {
	const std::map<int, std::string>& errorPages = _serverConfig.getErrorPages();
	std::map<int, std::string>::const_iterator errorPageIt = errorPages.find(errorCode);

	if (errorPageIt != errorPages.end()) {
		std::cout << "\033[0;33m[Router]\033[0m Using error page: " << errorPageIt->second << std::endl;
		std::string errorPageContent = readFile(errorPageIt->second);
		
		response.setStatusCode(errorCode);
		response.setBody(errorPageContent);
		response.setHeader("Content-Type", "text/html");
	} else {
		std::cout << "\033[0;31m[Router]\033[0m No error page configured for code: " << errorCode << std::endl;
		response.setStatusCode(errorCode);
		response.setBody("");
		response.setHeader("Content-Type", "text/html");
	}
}

std::vector<std::string> Router::getFilesInDirectory(const std::string& directory) {
	std::vector<std::string> files;
	DIR* dir = opendir(directory.c_str());
	if (dir) {
		struct dirent* entry;
		while ((entry = readdir(dir)) != NULL) {
			if (entry->d_type == DT_REG) {
				files.push_back(entry->d_name);
			}
		}
		closedir(dir);
	}
	return files;
}

void Router::saveUploadedFiles(const HttpRequest& req, const std::string& uploadDir) {
	const std::vector<std::string>& filenames = req.getFilenames();
	const std::string& body = req.getBody();
	size_t pos = 0;

	for (size_t i = 0; i < filenames.size(); ++i) {
		size_t start = body.find("\r\n\r\n", pos) + 4;
		if (start == std::string::npos) continue;

		size_t end = body.find("\r\n--", start);
		if (end == std::string::npos) continue;

		std::string fileContent = body.substr(start, end - start);
		pos = end + 4;

		std::string savedFilename = uploadDir + filenames[i];
		std::ofstream outFile(savedFilename.c_str(), std::ios::binary);
		if (outFile.is_open()) {
			outFile.write(fileContent.c_str(), fileContent.size());
			outFile.close();
			std::cout << "\033[0;32m[Upload]\033[0m File saved: " << savedFilename << std::endl;
		}
	}
}

void Router::addRoute(const std::string& path, RouteHandler handler) {
	routes[path] = handler;
}

// Neue Hilfsmethode für die Verzeichnisauflistung
std::string Router::generateDirectoryListing(const std::string& dirPath, const std::string& requestPath) {
	std::cout << "\033[0;33m[Router]\033[0m Generating listing for: " << dirPath << std::endl;
	
	std::ostringstream html;
	html << "<html>\n<head>\n"
		 << "<title>Index of " << requestPath << "</title>\n"
		 << "<style>\n"
		 << "body { font-family: monospace; padding: 20px; }\n"
		 << "table { width: 100%; border-collapse: collapse; }\n"
		 << "th, td { text-align: left; padding: 8px; border-bottom: 1px solid #ddd; }\n"
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
		std::cout << "\033[0;33m[Router]\033[0m Directory opened successfully" << std::endl;
		struct dirent* entry;
		std::vector<std::string> entries;
		
		while ((entry = readdir(dir)) != NULL) {
			entries.push_back(entry->d_name);
		}
		
		std::cout << "\033[0;33m[Router]\033[0m Found " << entries.size() << " entries" << std::endl;
		std::sort(entries.begin(), entries.end());
		
		for (std::vector<std::string>::const_iterator it = entries.begin(); 
			 it != entries.end(); ++it) {
			std::string name = *it;
			struct stat statbuf;
			std::string fullPath = dirPath + "/" + name;
			
			if (stat(fullPath.c_str(), &statbuf) == 0) {
				bool isDir = S_ISDIR(statbuf.st_mode);
				
				// Formatiere die Größe
				std::ostringstream size;
				if (isDir)
					size << "-";
				else
					size << statbuf.st_size;
				
				// Formatiere das Datum
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
		std::cout << "\033[0;31m[Router]\033[0m Failed to open directory: " << dirPath << std::endl;
		html << "<tr><td colspan='3'>Error reading directory</td></tr>\n";
	}

	html << "</table>\n</body>\n</html>";
	return html.str();
}