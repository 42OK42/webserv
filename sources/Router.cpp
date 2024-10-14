/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: okrahl <okrahl@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/08 14:54:49 by okrahl            #+#    #+#             */
/*   Updated: 2024/10/14 16:57:14 by okrahl           ###   ########.fr       */
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
	//std::cout << "Request URL: " << request.getUrl() << std::endl;
	//std::cout << "Request Method: " << request.getMethod() << std::endl;

	// Extract Path without Query-Parameter
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
	//std::cout << "handleUploadRoute called" << std::endl;
	//std::cout << "Request URL: " << req.getUrl() << std::endl;
	//std::cout << "Request Method: " << req.getMethod() << std::endl;

	if (req.getMethod() == "GET") {
		std::string content = readFile("HTMLFiles/upload.html");
		res.setStatusCode(200);
		res.setBody(content);
		res.setHeader("Content-Type", "text/html");
	} else if (req.getMethod() == "POST") {
		std::cout << "Processing POST request" << std::endl;
		
		// Angenommen, diese Methoden geben alle Dateinamen und Daten zurück
		std::vector<std::string> filenames = req.getFilenames(); 
		std::vector<std::string> fileData = req.getFileData(); 

		if (filenames.empty() || fileData.empty()) {
			std::cout << "No files found in request" << std::endl;
			res.setStatusCode(400);
			res.setBody("Bad Request: No files found");
			res.setHeader("Content-Type", "text/plain");
			return;
		}

		std::string uploadDir = "./uploads";
		ensureDirectoryExists(uploadDir);

		for (size_t i = 0; i < filenames.size(); ++i) {
			std::string originalFilename = filenames[i];
			std::string imageData = fileData[i];

			std::cout << "Original filename: " << originalFilename << std::endl;
			std::cout << "Image data size: " << imageData.size() << std::endl;

			std::string uniqueFilename = generateUniqueFilename(uploadDir, originalFilename, uploadedFiles);
			std::string savedFilename = uploadDir + "/" + uniqueFilename;
			std::cout << "Saving file to: " << savedFilename << std::endl;

			std::ofstream outFile(savedFilename.c_str(), std::ios::binary);
			if (outFile.is_open()) {
				outFile.write(imageData.c_str(), imageData.size());
				outFile.close();
				std::cout << "File saved successfully" << std::endl;
				uploadedFiles.push_back(uniqueFilename);
			} else {
				std::cout << "Failed to open file for writing" << std::endl;
				res.setStatusCode(500);
				res.setBody("<html><body><h1>500 Internal Server Error</h1></body></html>");
				res.setHeader("Content-Type", "text/html");
				return;
			}
		}

		std::string successContent = "<html><body><h1>Upload Successful</h1>";
		successContent += "<p>Your files have been uploaded successfully.</p>";
		successContent += "<h2>Uploaded Files</h2>";
		successContent += generateUploadListHTML(uploadDir);
		successContent += "<button onclick=\"location.href='/'\">Back to Home</button>";
		successContent += "<button onclick=\"location.href='/upload'\">Upload Another File</button>";
		successContent += "<button onclick=\"location.href='/form'\">Go to Form Page</button>";
		successContent += "</body></html>";

		res.setStatusCode(200);
		res.setBody(successContent);
		res.setHeader("Content-Type", "text/html");
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
			std::cout << "Error deleting file: " << strerror(errno) << std::endl;
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