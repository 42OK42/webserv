#include "HttpRequest.hpp"

HttpRequest::HttpRequest(const char* buffer, int bytesRead) {
	parse(buffer, bytesRead);
}

const std::string& HttpRequest::getMethod() const { return method; }
const std::string& HttpRequest::getUrl() const { return url; }
const std::string& HttpRequest::getHttpVersion() const { return httpVersion; }
const std::map<std::string, std::string>& HttpRequest::getHeaders() const { return headers; }
const std::string& HttpRequest::getBody() const { return body; }

std::string HttpRequest::getHeader(const std::string& name) const {
	std::map<std::string, std::string>::const_iterator it = headers.find(name);
	if (it != headers.end()) {
		return it->second;
	}
	return "";
}

std::string HttpRequest::getHost() const {
	return getHeader("Host");
}

std::string HttpRequest::getUserAgent() const {
	return getHeader("User-Agent");
}

std::string HttpRequest::getConnection() const {
	return getHeader("Connection");
}

std::string HttpRequest::getAccept() const {
	return getHeader("Accept");
}

void HttpRequest::parse(const char* buffer, int bytesRead)
{
	std::string rawRequest(buffer, bytesRead);
	std::istringstream stream(rawRequest);
	std::string line;

	std::getline(stream, line);
	std::istringstream startLine(line);
	startLine >> method >> url >> httpVersion;

	while (std::getline(stream, line) && line != "\r")
	{
		if (!line.empty() && line[line.size() - 1] == '\r') {
			line.erase(line.size() - 1);
		}
		std::string key, value;
		std::istringstream headerLine(line);
		if (std::getline(headerLine, key, ':') && std::getline(headerLine, value))
			headers[key] = value.substr(1);
	}

	if (headers.count("Content-Length"))
	{
		int contentLength;
		std::istringstream(headers["Content-Length"]) >> contentLength;
		body.resize(contentLength);
		stream.read(&body[0], contentLength);
	}
}

void HttpRequest::print() const
{
	std::cout << "Method: " << method << "\n";
	std::cout << "URL: " << url << "\n";
	std::cout << "HTTP Version: " << httpVersion << "\n";
	// std::cout << "Headers:\n";
	// for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
	// 	std::cout << "  " << it->first << ": " << it->second << "\n";
	// if (!body.empty())
	// 	std::cout << "Body: " << body << "\n";
}
