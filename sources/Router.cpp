/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: okrahl <okrahl@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/16 17:44:54 by okrahl            #+#    #+#             */
/*   Updated: 2024/11/05 18:12:28 by okrahl           ###   ########.fr       */
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
		// Zuerst prüfen, ob wir eine spezielle Route haben
		std::map<std::string, RouteHandler>::iterator routeIt = routes.find(path);
		if (routeIt != routes.end()) {
			std::cout << "[Router] Found special route for: " << path << std::endl;
			(this->*(routeIt->second))(request, response);
			return;
		}

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
				if (request.getMethod() == "GET") {
					std::string indexPath = fullPath;
					if (!location.getIndex().empty()) {
						indexPath += "/" + location.getIndex();
						
						if (stat(indexPath.c_str(), &statbuf) == 0 && S_ISREG(statbuf.st_mode)) {
							std::cout << "\033[0;33m[Router]\033[0m Using index file" << std::endl;
							std::string content = readFile(indexPath);
							response.setStatusCode(200);
							response.setBody(content);
							response.setHeader("Content-Type", "text/html");
							return;
						}
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

void Router::handleUploadRoute(const HttpRequest& request, HttpResponse& response) {
	std::cout << "\n[Upload] === Starting handleUploadRoute ===" << std::endl;
	
	// Print complete request details
	request.print();
	
	if (request.getMethod() == "POST") {
		std::string uploadDir = "/home/okrahl/sgoinfre/uploads_webserv/";
		
		std::cout << "[Upload] Starting upload process..." << std::endl;
		std::cout << "[Upload] Content-Type: " << request.getHeader("Content-Type") << std::endl;
		std::cout << "[Upload] Content-Length: " << request.getHeader("Content-Length") << std::endl;
		std::cout << "[Upload] Upload directory: " << uploadDir << std::endl;
		
		ensureDirectoryExists(uploadDir);
		
		const std::vector<std::string>& filenames = request.getFilenames();
		std::cout << "[Upload] Number of files found: " << filenames.size() << std::endl;
		
		const std::string& body = request.getBody();
		std::cout << "[Upload] Body size: " << body.size() << " bytes" << std::endl;
		std::cout << "[Upload] First 100 chars of body: \n" << body.substr(0, 100) << std::endl;
		
		size_t pos = 0;

		for (size_t i = 0; i < filenames.size(); ++i) {
			std::cout << "\n[Upload] Processing file " << (i+1) << ": " << filenames[i] << std::endl;
			
			size_t start = body.find("\r\n\r\n", pos) + 4;
			std::cout << "[Upload] Content start position: " << start << std::endl;
			if (start == std::string::npos) {
				std::cout << "[Upload] ERROR: Could not find start of file content" << std::endl;
				continue;
			}
			
			size_t end = body.find("\r\n--", start);
			if (end == std::string::npos) {
				std::cout << "[Upload] Using body length as end position" << std::endl;
				end = body.length();
			}
			std::cout << "[Upload] Content end position: " << end << std::endl;
			
			std::string fileContent = body.substr(start, end - start);
			std::cout << "[Upload] File content size: " << fileContent.size() << " bytes" << std::endl;
			pos = end + 4;

			if (fileContent.size() >= 2 && fileContent.compare(fileContent.size() - 2, 2, "\r\n") == 0) {
				fileContent.erase(fileContent.size() - 2);
				std::cout << "[Upload] Removed trailing CRLF" << std::endl;
			}

			std::string savedFilename = uploadDir + filenames[i];
			std::cout << "[Upload] Attempting to save file to: " << savedFilename << std::endl;
			
			std::ofstream outFile(savedFilename.c_str(), std::ios::binary);
			if (outFile.is_open()) {
				outFile.write(fileContent.c_str(), fileContent.size());
				outFile.close();
				std::cout << "[Upload] File saved successfully: " << savedFilename << std::endl;
			} else {
				std::cerr << "[Upload] Failed to save file: " << savedFilename << std::endl;
				std::cerr << "[Upload] Error: " << strerror(errno) << std::endl;
			}
		}
		
		std::cout << "[Upload] Setting response for redirect" << std::endl;
		response.setStatusCode(303);
		response.setHeader("Location", "/uploadSuccessful");
	} else if (request.getMethod() == "GET") {
		std::cout << "[Upload] Handling GET request" << std::endl;
		Location location = _serverConfig.findLocation("/upload");
		if (location.getAutoIndex()) {
			std::string uploadDir = "/home/okrahl/sgoinfre/uploads_webserv/";
			std::cout << "[Upload] Generating directory listing for: " << uploadDir << std::endl;
			std::string dirListing = generateDirectoryListing(uploadDir, "/upload");
			response.setStatusCode(200);
			response.setBody(dirListing);
			response.setHeader("Content-Type", "text/html");
		} else {
			std::string formPath = location.getRoot() + "/upload.html";
			std::cout << "[Upload] Serving upload form from: " << formPath << std::endl;
			std::string content = readFile(formPath);
			response.setStatusCode(200);
			response.setBody(content);
			response.setHeader("Content-Type", "text/html");
		}
	}
	std::cout << "[Upload] === Finished handleUploadRoute ===" << std::endl;
}

void Router::handleUploadSuccessRoute(const HttpRequest& request, HttpResponse& response) {
	std::cout << "[UploadSuccess] Starting upload success route" << std::endl;
	std::cout << "[UploadSuccess] Method: " << request.getMethod() << std::endl;
	
	if (request.getMethod() == "GET") {
		Location location = _serverConfig.findLocation("/uploadSuccessful");
		std::string root = location.getRoot();
		std::string successPath = root + "/uploadSuccessful.html";
		
		std::cout << "[UploadSuccess] Loading success page from: " << successPath << std::endl;
		std::string successContent = readFile(successPath);
		
		std::string uploadDir = "/home/okrahl/sgoinfre/uploads_webserv/";  // Hardcoded für Test
		std::cout << "[UploadSuccess] Reading files from directory: " << uploadDir << std::endl;
		
		std::vector<std::string> files = getFilesInDirectory(uploadDir);
		std::cout << "[UploadSuccess] Found " << files.size() << " files:" << std::endl;
		for (size_t i = 0; i < files.size(); ++i) {
			std::cout << "  - " << files[i] << std::endl;
		}
		
		std::ostringstream json;
		json << "[";
		for (size_t i = 0; i < files.size(); ++i) {
			if (i > 0) json << ", ";
			json << "\"" << files[i] << "\"";
		}
		json << "]";
		
		std::string script = "<script>const fileList = " + json.str() + ";</script>";
		std::cout << "[UploadSuccess] Inserting script: " << script << std::endl;
		
		size_t pos = successContent.find("</head>");
		if (pos != std::string::npos) {
			successContent.insert(pos, script);
			std::cout << "[UploadSuccess] Script inserted successfully" << std::endl;
		} else {
			std::cout << "[UploadSuccess] ERROR: Could not find </head> tag!" << std::endl;
		}

		response.setStatusCode(200);
		response.setBody(successContent);
		response.setHeader("Content-Type", "text/html");
	}
	else if (request.getMethod() == "DELETE") {
		std::string uploadDir = "/home/okrahl/sgoinfre/uploads_webserv/";
		std::string url = request.getUrl();
		
		// Nutze die Helper-Funktion
		std::string filename = extractFilenameFromUrl(url);
		
		if (!filename.empty()) {
			std::string fullPath = uploadDir + filename;
			std::cout << "[UploadSuccess] Attempting to delete file: " << fullPath << std::endl;
			
			if (remove(fullPath.c_str()) == 0) {
				std::cout << "[UploadSuccess] File deleted successfully" << std::endl;
				response.setStatusCode(200);
				response.setBody("File deleted successfully");
			} else {
				std::cerr << "[UploadSuccess] Error deleting file: " << strerror(errno) << std::endl;
				response.setStatusCode(500);
				response.setBody("Error deleting file");
			}
		} else {
			std::cout << "[UploadSuccess] No filename provided in DELETE request" << std::endl;
			response.setStatusCode(400);
			response.setBody("No filename provided");
		}
		response.setHeader("Content-Type", "text/plain");
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