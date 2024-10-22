/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: okrahl <okrahl@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/16 17:44:54 by okrahl            #+#    #+#             */
/*   Updated: 2024/10/22 17:31:40 by okrahl           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Router.hpp"
#include "Helper.hpp"
#include <sstream>
#include <ctime>

Router::Router(ServerConfig& config) : _serverConfig(config) {
	// Initialisierungscode
}

Router::~Router() {
	// Bereinigungscode
}

void Router::addRoute(const std::string& path, RouteHandler handler) {
	routes[path] = handler;
}

void Router::handleRequest(const HttpRequest& request, HttpResponse& response) {
	std::string path = request.getUrl();
	size_t queryPos = path.find('?');
	if (queryPos != std::string::npos) {
		path = path.substr(0, queryPos);
	}

	std::map<std::string, RouteHandler>::const_iterator it = routes.find(path);
	if (it != routes.end()) {
		RouteHandler handler = it->second;
		(this->*handler)(request, response); // Call the member function
	} else {
		int errorCode = 404;
		const std::map<int, std::string>& errorPages = _serverConfig.getErrorPages();
		std::map<int, std::string>::const_iterator errorPageIt = errorPages.find(errorCode);

		std::string errorPageContent;
		if (errorPageIt != errorPages.end())
			errorPageContent = readFile(errorPageIt->second);
		else
			errorPageContent = "<html><body><h1>404 Not Found</h1></body></html>";

		response.setStatusCode(errorCode);
		response.setBody(errorPageContent);
		response.setHeader("Content-Type", "text/html");
	}
}

void Router::handleHomeRoute(const HttpRequest& req, HttpResponse& res) {
	if (req.getMethod() == "GET") {
		std::string content = readFile("HTMLFiles/welcome.html");
		res.setStatusCode(200);
		res.setBody(content);
		res.setHeader("Content-Type", "text/html");
	} else {
		res.setStatusCode(405);
		res.setBody("<html><body><h1>405 Method Not Allowed</h1></body></html>");
		res.setHeader("Content-Type", "text/html");
	}
}

void Router::handleFormRoute(const HttpRequest& req, HttpResponse& res) {
	if (req.getMethod() == "GET") {
		std::string content = readFile("HTMLFiles/form.html");
		res.setStatusCode(200);
		res.setBody(content);
		res.setHeader("Content-Type", "text/html");
	} else {
		res.setStatusCode(405);
		res.setBody("<html><body><h1>405 Method Not Allowed</h1></body></html>");
		res.setHeader("Content-Type", "text/html");
	}
}

void Router::handleUploadRoute(const HttpRequest& req, HttpResponse& res) {
	std::cout << "handleUploadRoute called" << std::endl;
	std::cout << "Request URL: " << req.getUrl() << std::endl;
	std::cout << "Request Method: " << req.getMethod() << std::endl;

	if (req.getMethod() == "GET") {
		std::string content = readFile("HTMLFiles/upload.html");
		res.setStatusCode(200);
		res.setBody(content);
		res.setHeader("Content-Type", "text/html");
		std::cout << "Response: 200 OK (Upload GET)" << std::endl;
	} else if (req.getMethod() == "POST") {
		std::cout << "Processing POST request" << std::endl;

		const std::vector<std::string>& filenames = req.getFilenames();
		if (filenames.empty()) {
			std::cout << "No files uploaded" << std::endl;
			res.setStatusCode(400);
			res.setBody("Bad Request: No files uploaded");
			res.setHeader("Content-Type", "text/plain");
			std::cout << "Response: 400 Bad Request (No files uploaded)" << std::endl;
			return;
		}

		saveUploadedFiles(req);

		std::string successContent = readFile("HTMLFiles/uploadSuccessful.html");
		res.setStatusCode(200);
		res.setBody(successContent);
		res.setHeader("Content-Type", "text/html");
		std::cout << "Response: 200 OK (Upload POST)" << std::endl;
	} else if (req.getMethod() == "DELETE") {
		std::cout << "Processing DELETE request" << std::endl;
		std::string filename = extractFilenameFromUrl(req.getUrl());
		std::cout << "Extracted filename: " << filename << std::endl;

		if (filename.empty()) {
			std::cout << "Filename not found in DELETE request" << std::endl;
			res.setStatusCode(400);
			res.setBody("Bad Request: Filename not found");
			res.setHeader("Content-Type", "text/plain");
			std::cout << "Response: 400 Bad Request (Filename not found)" << std::endl;
			return;
		}

		std::string filePath = "/home/okrahl/sgoinfre/uploads_webserv/" + filename;
		std::cout << "Attempting to delete file: " << filePath << std::endl;

		if (remove(filePath.c_str()) == 0) {
			std::cout << "File deleted successfully" << std::endl;
			uploadedFiles.erase(std::remove(uploadedFiles.begin(), uploadedFiles.end(), filename), uploadedFiles.end());
			res.setStatusCode(200);
			res.setBody("File Deleted Successfully");
			std::cout << "Response: 200 OK (File Deleted)" << std::endl;
		} else {
			std::cout << "Error deleting file: " << strerror(errno) << std::endl;
			res.setStatusCode(404);
			res.setBody("File Not Found");
			std::cout << "Response: 404 Not Found (Error deleting file)" << std::endl;
		}
		res.setHeader("Content-Type", "text/plain");
	} else {
		std::cout << "Method not allowed: " << req.getMethod() << std::endl;
		res.setStatusCode(405);
		res.setBody("<html><body><h1>405 Method Not Allowed</h1></body></html>");
		res.setHeader("Content-Type", "text/html");
		std::cout << "Response: 405 Method Not Allowed (Upload Route)" << std::endl;
	}
}

void Router::saveUploadedFiles(const HttpRequest& req) {
	const std::vector<std::string>& filenames = req.getFilenames();
	const std::string& body = req.getBody();
	size_t pos = 0;

	std::cout << "Full body received (first 1000 chars): " << body.substr(0, 1000) << std::endl;
	if (body.size() > 1000) {
		std::cout << "  (truncated, total size: " << body.size() << " bytes)\n";
	}

	ensureDirectoryExists("/home/okrahl/sgoinfre/uploads_webserv");

	for (size_t i = 0; i < filenames.size(); ++i) {
		size_t start = body.find("\r\n\r\n", pos) + 4;
		if (start == std::string::npos) {
			std::cerr << "Failed to find the start of the file content for file: " << filenames[i] << std::endl;
			continue;
		}

		size_t end = body.find("\r\n--", start);
		if (end == std::string::npos) {
			std::cerr << "Failed to find the end of the file content for file: " << filenames[i] << std::endl;
			continue;
		}

		std::string fileContent = body.substr(start, end - start);
		pos = end + 4;

		if (fileContent.size() >= 2 && fileContent.compare(fileContent.size() - 2, 2, "\r\n") == 0) {
			fileContent.erase(fileContent.size() - 2);
		}

		std::string savedFilename = "/home/okrahl/sgoinfre/uploads_webserv/" + filenames[i].substr(filenames[i].find_last_of("\\/") + 1);
		std::ofstream outFile(savedFilename.c_str(), std::ios::binary);
		if (outFile.is_open()) {
			outFile.write(fileContent.c_str(), fileContent.size());
			outFile.close();
			std::cout << "File saved successfully: " << savedFilename << std::endl;
			uploadedFiles.push_back(savedFilename);
		} else {
			std::cerr << "Failed to open file for writing: " << savedFilename << std::endl;
		}
	}
}

void Router::initializeRoutes() {
	addRoute("/", &Router::handleHomeRoute);
	addRoute("/upload", &Router::handleUploadRoute);
	addRoute("/form", &Router::handleFormRoute);
}
