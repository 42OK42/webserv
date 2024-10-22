/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: okrahl <okrahl@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/16 17:44:54 by okrahl            #+#    #+#             */
/*   Updated: 2024/10/22 16:48:47 by okrahl           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Router.hpp"
#include "Helper.hpp"
#include <sstream>
#include <ctime>

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
	} else if (req.getMethod() == "POST") {
		std::cout << "Processing POST request" << std::endl;
		std::string originalFilename = req.getFilename();
		std::cout << "Original filename: " << originalFilename << std::endl;

		if (originalFilename.empty()) {
			std::cout << "Filename not found" << std::endl;
			res.setStatusCode(400);
			res.setBody("Bad Request: Filename not found");
			res.setHeader("Content-Type", "text/plain");
			return;
		}

		std::string imageData = req.getBody();
		std::cout << "Image data size: " << imageData.size() << std::endl;

		std::string uploadDir = "./uploads";
		ensureDirectoryExists(uploadDir);

		std::string savedFilename = uploadDir + "/" + originalFilename;
		std::cout << "Saving file to: " << savedFilename << std::endl;

		std::ofstream outFile(savedFilename.c_str(), std::ios::binary);
		if (outFile.is_open()) {
			outFile.write(imageData.c_str(), imageData.size());
			outFile.close();
			std::cout << "File saved successfully" << std::endl;

			uploadedFiles.push_back(savedFilename);

			std::string successContent = readFile("HTMLFiles/uploadSuccessful.html");
			res.setStatusCode(200);
			res.setBody(successContent);
			res.setHeader("Content-Type", "text/html");
		} else {
			std::cout << "Failed to open file for writing" << std::endl;
			res.setStatusCode(500);
			res.setBody("<html><body><h1>500 Internal Server Error</h1></body></html>");
			res.setHeader("Content-Type", "text/html");
		}
	} else if (req.getMethod() == "DELETE") {
		std::cout << "Processing DELETE request" << std::endl;
		std::string filename = extractFilenameFromUrl(req.getUrl());
		std::cout << "Extracted filename: " << filename << std::endl;

		if (filename.empty()) {
			std::cout << "Filename not found in DELETE request" << std::endl;
			res.setStatusCode(400);
			res.setBody("Bad Request: Filename not found");
			res.setHeader("Content-Type", "text/plain");
			return;
		}

		std::string filePath = "./uploads/" + filename;
		std::cout << "Attempting to delete file: " << filePath << std::endl;

		if (remove(filePath.c_str()) == 0) {
			std::cout << "File deleted successfully" << std::endl;
			uploadedFiles.erase(std::remove(uploadedFiles.begin(), uploadedFiles.end(), filename), uploadedFiles.end());
			res.setStatusCode(200);
			res.setBody("File Deleted Successfully");
		} else {
			std::cout << "Error deleting file: " << strerror(errno) << std::endl; //@olli oh oh oh
			res.setStatusCode(404);
			res.setBody("File Not Found");
		}
		res.setHeader("Content-Type", "text/plain");
	} else {
		std::cout << "Method not allowed: " << req.getMethod() << std::endl;
		res.setStatusCode(405);
		res.setBody("<html><body><h1>405 Method Not Allowed</h1></body></html>");
		res.setHeader("Content-Type", "text/html");
	}
}

void Router::initializeRoutes() {
	addRoute("/", &Router::handleHomeRoute);
	addRoute("/upload", &Router::handleUploadRoute);
	addRoute("/form", &Router::handleFormRoute);
}
