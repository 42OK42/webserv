/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: okrahl <okrahl@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/16 17:44:54 by okrahl            #+#    #+#             */
/*   Updated: 2024/11/12 17:51:34 by okrahl           ###   ########.fr       */
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
	addRoute("/form", &Router::handleFormRoute);
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

		std::map<std::string, RouteHandler>::iterator routeIt = routes.find(path);
		if (routeIt != routes.end()) {
			(this->*(routeIt->second))(request, response);
			return;
		}

		if (handleDirectoryRequest(path, request, response)) {
			return;
		}

		setErrorResponse(response, 404);
	} catch (const ServerConfig::LocationNotFound& e) {
		setErrorResponse(response, 404);
	}
}

void Router::handleHomeRoute(const HttpRequest& req, HttpResponse& res) {
	if (req.getMethod() == "GET") {
		if (!handleDirectoryRequest("/", req, res)) {
			setErrorResponse(res, 404);
		}
	} else {
		setErrorResponse(res, 405);
	}
}

void Router::handleFormRoute(const HttpRequest& request, HttpResponse& response) {
	if (request.getMethod() == "GET") {
		if (!handleDirectoryRequest("/form", request, response)) {
			setErrorResponse(response, 404);
		}
	} else {
		setErrorResponse(response, 405);
	}
}

void Router::handleUploadRoute(const HttpRequest& request, HttpResponse& response) {
	if (request.getMethod() == "GET") {
		if (!handleDirectoryRequest("/upload", request, response)) {
			setErrorResponse(response, 404);
		}
		return;
	}

	if (request.getMethod() == "POST") {
		std::string contentType = request.getHeader("Content-Type");
		std::string uploadDir = "/home/okrahl/sgoinfre/uploads_webserv/";
		
		ensureDirectoryExists(uploadDir);
		bool uploadSuccess = false;

		if (contentType.find("multipart/form-data") != std::string::npos) {
			const std::vector<std::string>& filenames = request.getFilenames();
			
			#ifdef DEBUG_MODE
			std::cout << "\033[0;35m[DEBUG] Router::handleUploadRoute: Anzahl Dateien: " 
					  << filenames.size() << "\033[0m" << std::endl;
			#endif

			if (!filenames.empty()) {
				const std::string& body = request.getBody();
				size_t pos = 0;
				
				for (size_t i = 0; i < filenames.size(); ++i) {
					size_t start = body.find("\r\n\r\n", pos) + 4;
					if (start == std::string::npos + 4) continue;
					
					size_t end = body.find("\r\n--", start);
					if (end == std::string::npos) {
						end = body.length();
					}
					
					std::string fileContent = body.substr(start, end - start);
					pos = end + 4;

					if (fileContent.size() >= 2 && fileContent.compare(fileContent.size() - 2, 2, "\r\n") == 0) {
						fileContent.erase(fileContent.size() - 2);
					}

					std::string savedFilename = uploadDir + filenames[i];
					std::ofstream outFile(savedFilename.c_str(), std::ios::binary);
					if (outFile.is_open()) {
						outFile.write(fileContent.c_str(), fileContent.size());
						outFile.close();
						uploadSuccess = true;
						
						#ifdef DEBUG_MODE
						std::cout << "\033[0;32m[DEBUG] Router::handleUploadRoute: Datei erfolgreich gespeichert: " 
								  << savedFilename << "\033[0m" << std::endl;
						#endif
					}
				}
			}
		} else {
			std::string filename = "text_upload_" + getCurrentTimestamp() + ".txt";
			std::string fullPath = uploadDir + filename;
			std::ofstream outFile(fullPath.c_str(), std::ios::binary);
			if (outFile.is_open()) {
				outFile.write(request.getBody().c_str(), request.getBody().size());
				outFile.close();
				uploadSuccess = true;
			}
		}

		if (uploadSuccess) {
			response.setStatusCode(303);
			response.setHeader("Location", "/uploadSuccessful");
				response.setHeader("Content-Type", "text/html");
				response.setHeader("Content-Length", "0");
				response.setBody("");
		} else {
			setErrorResponse(response, 500);
		}
	} else {
		setErrorResponse(response, 405);
	}
}

void Router::handleUploadSuccessRoute(const HttpRequest& request, HttpResponse& response) {
	if (request.getMethod() == "GET") {
		Location location = _serverConfig.findLocation("/uploadSuccessful");
		std::string root = location.getRoot();
		std::string index = location.getIndex();
		std::string fullPath = root + "/" + index;
		
		struct stat statbuf;
		if (stat(fullPath.c_str(), &statbuf) == 0 && S_ISREG(statbuf.st_mode)) {
			std::string successContent = readFile(fullPath);
			
			std::string uploadDir = "/home/okrahl/sgoinfre/uploads_webserv/";
			std::vector<std::string> files = getFilesInDirectory(uploadDir);
			
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
			
			response.setStatusCode(200);
			response.setBody(successContent);
			response.setHeader("Content-Type", "text/html");
		} else if (location.getAutoIndex()) {
			std::string dirListing = generateDirectoryListing(root, "/uploadSuccessful");
			response.setStatusCode(200);
			response.setBody(dirListing);
			response.setHeader("Content-Type", "text/html");
		} else {
			setErrorResponse(response, 404);
		}
	} else if (request.getMethod() == "DELETE") {
		std::string uploadDir = "/home/okrahl/sgoinfre/uploads_webserv/";
		std::string filename = extractFilenameFromUrl(request.getUrl());
		
		if (!filename.empty()) {
			std::string fullPath = uploadDir + filename;
			if (remove(fullPath.c_str()) == 0) {
				response.setStatusCode(200);
				response.setBody("File deleted successfully");
			} else {
				response.setStatusCode(500);
				response.setBody("Error deleting file");
			}
		} else {
			response.setStatusCode(400);
			response.setBody("No filename provided");
		}
		response.setHeader("Content-Type", "text/plain");
	} else {
		setErrorResponse(response, 405);
	}
}

void Router::setErrorResponse(HttpResponse& response, int errorCode) {
	const std::map<int, std::string>& errorPages = _serverConfig.getErrorPages();
	std::map<int, std::string>::const_iterator errorPageIt = errorPages.find(errorCode);

	if (errorPageIt != errorPages.end()) {
		std::string errorPageContent = readFile(errorPageIt->second);
		
		response.setStatusCode(errorCode);
		response.setBody(errorPageContent);
		response.setHeader("Content-Type", "text/html");
	} else {
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

std::string Router::generateDirectoryListing(const std::string& dirPath, const std::string& requestPath) {
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

bool Router::handleDirectoryRequest(const std::string& path, const HttpRequest& request, HttpResponse& response) {
	Location location = _serverConfig.findLocation(path);
	std::string fullPath = location.getRoot();
	
	struct stat statbuf;
	if (stat(fullPath.c_str(), &statbuf) == 0) {
		if (S_ISDIR(statbuf.st_mode)) {
			if (request.getMethod() == "GET") {
				std::string indexPath = fullPath;
				if (!location.getIndex().empty()) {
					indexPath += "/" + location.getIndex();
					
					if (stat(indexPath.c_str(), &statbuf) == 0 && S_ISREG(statbuf.st_mode)) {
						std::string content = readFile(indexPath);
						response.setStatusCode(200);
						response.setBody(content);
						response.setHeader("Content-Type", "text/html");
						return true;
					}
				}
				
				if (location.getAutoIndex()) {
					std::string dirListing = generateDirectoryListing(fullPath, path);
					response.setStatusCode(200);
					response.setBody(dirListing);
					response.setHeader("Content-Type", "text/html");
					return true;
				}
			}
		}
	}
	return false;
}

std::string Router::getCurrentTimestamp() const {
	std::time_t now = std::time(NULL);
	char buffer[20];
	std::strftime(buffer, sizeof(buffer), "%Y%m%d_%H%M%S", std::localtime(&now));
	return std::string(buffer);
}