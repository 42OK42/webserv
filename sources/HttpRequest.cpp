/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: okrahl <okrahl@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/08 15:49:27 by okrahl            #+#    #+#             */
/*   Updated: 2024/11/13 17:48:46 by okrahl           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Helper.hpp"
#include "HttpRequest.hpp"

HttpRequest::HttpRequest(const char* buffer, int bytesRead, const ServerConfig& serverConfig)
	: _serverConfig(serverConfig) {
	parse(buffer, bytesRead);
}

const std::string& HttpRequest::getMethod() const { return method; }
const std::string& HttpRequest::getUrl() const { return url; }
const std::string& HttpRequest::getHttpVersion() const { return httpVersion; }
const std::map<std::string, std::string>& HttpRequest::getHeaders() const { return headers; }
const std::string& HttpRequest::getBody() const { return body; }
const std::vector<std::string>& HttpRequest::getFilenames() const { return filenames; }
const std::string& HttpRequest::getHost() const { return host; }
int HttpRequest::getPort() const { return port; }

std::string HttpRequest::getHeader(const std::string& name) const {
	std::map<std::string, std::string>::const_iterator it = headers.find(name);
	if (it != headers.end()) {
		return it->second;
	}
	return "";
}

void HttpRequest::parseMultipartData(const std::string& boundary) {
	std::string delimiter = "--" + boundary;
	size_t pos = 0;
	size_t end = body.find(delimiter, pos);

	while (end != std::string::npos) {
		size_t start = pos + delimiter.length() + 2;
		pos = end + delimiter.length();
		end = body.find(delimiter, pos);

		std::string part = body.substr(start, end - start);
		
		size_t headerEnd = part.find("\r\n\r\n");
		if (headerEnd != std::string::npos) {
			std::string headers = part.substr(0, headerEnd);
			std::string content = part.substr(headerEnd + 4);
			content = content.substr(0, content.length() - 2);

			std::istringstream headerStream(headers);
			std::string line;
			while (std::getline(headerStream, line) && line != "\r") {
				if (!line.empty() && line[line.size() - 1] == '\r') {
					line.erase(line.size() - 1);
				}
				std::string key, value;
				std::istringstream headerLine(line);
				if (std::getline(headerLine, key, ':') && std::getline(headerLine, value)) {
					if (key == "Content-Disposition") {
						std::string filename = extractFilename(value);
						if (!filename.empty()) {
							filenames.push_back(filename);
							fileContents.push_back(content);
							std::cout << "Found file in request: " << filename << std::endl;
						}
					}
				}
			}
		}
	}
}

const std::vector<std::string>& HttpRequest::getFileContents() const {
	return fileContents;
}

void HttpRequest::parse(const char* buffer, int bytesRead) {
	#ifdef DEBUG_MODE
	std::cout << "\033[0;34m[DEBUG] HttpRequest::parse: Starting parsing of " << bytesRead << " bytes\033[0m" << std::endl;
	#endif

	std::string rawRequest(buffer, bytesRead);
	std::istringstream stream(rawRequest);
	std::string line;

	// Parse the request line
	std::getline(stream, line);
	std::istringstream startLine(line);
	startLine >> method >> url >> httpVersion;

	// Parse headers
	while (std::getline(stream, line) && line != "\r") {
		if (!line.empty() && line[line.size() - 1] == '\r') {
			line.erase(line.size() - 1);
		}
		std::string key, value;
		std::istringstream headerLine(line);
		if (std::getline(headerLine, key, ':') && std::getline(headerLine, value)) {
			headers[key] = value.substr(1);
			#ifdef DEBUG_MODE
			std::cout << "\033[0;34m[DEBUG] HttpRequest::parse: Header found: " << key << ": " << value.substr(1) << "\033[0m" << std::endl;
			#endif
		}
	}

	// Extract Host and Port
	std::map<std::string, std::string>::const_iterator it = headers.find("Host");
	if (it != headers.end()) {
		std::string hostHeader = it->second;
		size_t colonPos = hostHeader.find(':');
		if (colonPos != std::string::npos) {
			host = hostHeader.substr(0, colonPos);
			std::istringstream iss(hostHeader.substr(colonPos + 1));
			if (!(iss >> port)) {
				throw std::runtime_error("Invalid port in Host header");
			}
		} else {
			host = hostHeader;
			port = 8080; // Default HTTP port
		}
	} else {
		throw std::runtime_error("Missing Host header");
	}

	// Read body if Content-Length is present
	if (headers.count("Content-Length")) {
		int contentLength;
		std::istringstream(headers["Content-Length"]) >> contentLength;
		
		#ifdef DEBUG_MODE
		std::cout << "\033[0;34m[DEBUG] HttpRequest::parse: Content-Length = " << contentLength << "\033[0m" << std::endl;
		#endif
		
		if (!_serverConfig.isBodySizeAllowed(contentLength)) {
			#ifdef DEBUG_MODE
			std::cout << "\033[0;31m[DEBUG] HttpRequest::parse: Content-Length exceeds allowed size\033[0m" << std::endl;
			#endif
			throw std::runtime_error("Request body exceeds maximum allowed size");
		}
		
		body.resize(contentLength);
		stream.read(&body[0], contentLength);
	}

	// Parse multipart data if present
	if (headers.count("Content-Type")) {
		std::string contentType = headers["Content-Type"];
		size_t boundaryPos = contentType.find("boundary=");
		if (boundaryPos != std::string::npos) {
			std::string boundary = contentType.substr(boundaryPos + 9);
			parseMultipartData(boundary);
		}
	}
}

void HttpRequest::print() const {
	std::cout << "\n=== HTTP Request Details ===" << std::endl;
	std::cout << "Method: " << method << std::endl;
	std::cout << "URL: " << url << std::endl;
	std::cout << "HTTP Version: " << httpVersion << std::endl;
	std::cout << "Host: " << host << ":" << port << std::endl;
	
	std::cout << "\nHeaders:" << std::endl;
	for(std::map<std::string, std::string>::const_iterator it = headers.begin(); 
		it != headers.end(); ++it) {
		std::cout << "  " << it->first << ": " << it->second << std::endl;
	}
	
	std::cout << "\nFiles Found:" << std::endl;
	for(size_t i = 0; i < filenames.size(); ++i) {
		std::cout << "  " << filenames[i] << std::endl;
	}
	
	std::cout << "\nBody Size: " << body.size() << " bytes" << std::endl;
	if (body.size() > 0) {
		std::cout << "Body Preview (first 100 chars):" << std::endl;
		std::cout << body.substr(0, 100) << std::endl;
	}
	std::cout << "==========================\n" << std::endl;
}