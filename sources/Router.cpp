/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: okrahl <okrahl@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/08 14:54:49 by okrahl            #+#    #+#             */
/*   Updated: 2024/10/08 16:39:29 by okrahl           ###   ########.fr       */
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
	std::map<std::string, RouteHandler>::const_iterator it = routes.find(request.getUrl());
	if (it != routes.end()) {
		RouteHandler handler = it->second;
		(this->*handler)(request, response); // Call the member function
	} else {
		response.setStatusCode(404);
		response.setBody("<html><body><h1>404 Not Found</h1></body></html>");
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

void Router::handleUploadRoute(const HttpRequest& req, HttpResponse& res) {
	if (req.getMethod() == "GET") {
		std::string content = readFile("HTMLFiles/upload.html");
		res.setStatusCode(200);
		res.setBody(content);
		res.setHeader("Content-Type", "text/html");
	} else if (req.getMethod() == "POST") {
		std::string originalFilename = req.getFilename();  // Verwende getFilename()

		if (originalFilename.empty()) {
			res.setStatusCode(400);
			res.setBody("Bad Request: Filename not found");
			res.setHeader("Content-Type", "text/plain");
			return;
		}

		std::string imageData = req.getBody();

		std::string uploadDir = "./uploads";
		ensureDirectoryExists(uploadDir);

		std::string savedFilename = uploadDir + "/" + originalFilename;
		
		std::ofstream outFile(savedFilename.c_str(), std::ios::binary);
		if (outFile.is_open()) {
			outFile.write(imageData.c_str(), imageData.size());
			outFile.close();

			uploadedFiles.push_back(savedFilename);

			std::string successContent = readFile("HTMLFiles/uploadSuccessful.html");
			res.setStatusCode(200);
			res.setBody(successContent);
			res.setHeader("Content-Type", "text/html");
		} else {
			res.setStatusCode(500);
			res.setBody("<html><body><h1>500 Internal Server Error</h1></body></html>");
			res.setHeader("Content-Type", "text/html");
		}
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

void Router::handleUploadSuccessfulRoute(const HttpRequest& req, HttpResponse& res) {
	if (req.getMethod() == "DELETE") {
		std::string filename = req.getBody(); // Angenommen, der Dateiname wird im Body gesendet

		if (filename.empty()) {
			res.setStatusCode(400);
			res.setBody("Bad Request: Filename not found");
			res.setHeader("Content-Type", "text/plain");
			return;
		}

		std::string filePath = "./uploads/" + filename;
		if (remove(filePath.c_str()) == 0) {
			res.setStatusCode(200);
			res.setBody("File Deleted Successfully");
		} else {
			res.setStatusCode(404);
			res.setBody("File Not Found");
		}
		res.setHeader("Content-Type", "text/plain");
	} else {
		// Falls du GET oder andere Methoden für die Erfolgsseite unterstützen möchtest
		std::string content = readFile("HTMLFiles/uploadSuccessful.html");
		res.setStatusCode(200);
		res.setBody(content);
		res.setHeader("Content-Type", "text/html");
	}
}

void Router::initializeRoutes() {
	addRoute("/", &Router::handleHomeRoute);
	addRoute("/upload", &Router::handleUploadRoute);
	addRoute("/form", &Router::handleFormRoute);
	addRoute("/uploadSuccessful", &Router::handleUploadSuccessfulRoute); // Neue Route
	// Entferne die alte Delete-Route
}