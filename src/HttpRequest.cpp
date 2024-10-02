#include "HttpRequest.hpp"

HttpRequest::HttpRequest(const std::string& rawRequest)
{
	parse(rawRequest);
}

const std::string& HttpRequest::getMethod() const { return method; }
const std::string& HttpRequest::getUrl() const { return url; }
const std::string& HttpRequest::getHttpVersion() const { return httpVersion; }
const std::map<std::string, std::string>& HttpRequest::getHeaders() const { return headers; }
const std::string& HttpRequest::getBody() const { return body; }

void HttpRequest::parse(const std::string& rawRequest)
{
	std::istringstream stream(rawRequest);
	std::string line;

	// Parse Startzeile
	std::getline(stream, line);
	std::istringstream startLine(line);
	startLine >> method >> url >> httpVersion;

	// Parse Header
	while (std::getline(stream, line) && line != "\r") {
		std::string key, value;
		std::istringstream headerLine(line);
		if (std::getline(headerLine, key, ':') && std::getline(headerLine, value)) {
			headers[key] = value.substr(1); // Entferne führendes Leerzeichen
		}
	}

	// Parse Body (falls vorhanden)
	if (headers.count("Content-Length")) {
		int contentLength = std::stoi(headers["Content-Length"]);
		body.resize(contentLength);
		stream.read(&body[0], contentLength);
	}
}