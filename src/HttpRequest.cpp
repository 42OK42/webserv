#include "HttpRequest.hpp"

HttpRequest::HttpRequest(const std::string& rawRequest) {
	parse(rawRequest);
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

void HttpRequest::parse(const std::string& rawRequest) {
	std::istringstream stream(rawRequest);
	std::string line;

	// Parse Startzeile
	std::getline(stream, line);
	std::istringstream startLine(line);
	startLine >> method >> url >> httpVersion;

	// Parse Header
	while (std::getline(stream, line) && line != "\r") {
		if (line.back() == '\r') line.pop_back(); // Entferne \r am Ende
		std::string key, value;
		std::istringstream headerLine(line);
		if (std::getline(headerLine, key, ':') && std::getline(headerLine, value)) {
			headers[key] = value.substr(1); // Entferne führendes Leerzeichen
		}
	}

	// Body ist bei GET normalerweise nicht vorhanden, hier nur der Vollständigkeit halber
	if (headers.count("Content-Length")) {
		int contentLength = std::stoi(headers["Content-Length"]);
		body.resize(contentLength);
		stream.read(&body[0], contentLength);
	}
}