/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: okrahl <okrahl@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/18 17:04:09 by okrahl            #+#    #+#             */
/*   Updated: 2024/11/05 17:38:17 by okrahl           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/ServerConfig.hpp"
#include "../includes/Router.hpp"
#include "../includes/HttpResponse.hpp"
#include "../includes/HttpRequest.hpp"

// #include "Parser.hpp"

ServerConfig::ServerConfig() {}


// Destructeur
ServerConfig::~ServerConfig() {
	if (m_socket != -1) {
		close(m_socket);
	}
}

// Set the socket to non-blocking mode
int set_nonblocking(int sockfd) {
	int flags = fcntl(sockfd, F_GETFL, 0);
	if (flags == -1) {
		return -1;
	}
	return fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
}


void ServerConfig::set_socket_timeout(int sockfd, int timeout_seconds) {
	struct timeval timeout;
	timeout.tv_sec = timeout_seconds;
	timeout.tv_usec = 0;

	if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
		perror("setsockopt SO_RCVTIMEO");
	}
	if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0) {
		perror("setsockopt SO_SNDTIMEO");
	}
}


std::string ServerConfig::readFile(const std::string& filepath) {
	std::ifstream file(filepath.c_str());
	if (!file) {
		std::cerr << "Error: Could not open file: " << filepath << std::endl;
		return "";
	}
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

bool ServerConfig::readClientData(int client_fd) {
	char buffer[1024];
	int n = recv(client_fd, buffer, sizeof(buffer), 0);
	if (n <= 0) {
		close(client_fd);
		return false;
	}

	client_data[client_fd].append(buffer, n);

	std::string& data = client_data[client_fd];
	size_t header_end_pos = data.find("\r\n\r\n");
	if (header_end_pos != std::string::npos) {
		size_t content_length = 0;
		size_t content_length_pos = data.find("Content-Length:");
		if (content_length_pos != std::string::npos) {
			content_length_pos += 15;
			content_length = std::atoi(data.c_str() + content_length_pos);
		}

		if (data.size() >= header_end_pos + 4 + content_length) {
			return true;
		}
	}

	return false;
}

int ServerConfig::setupServerSocket()
{
	m_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (m_socket == -1)
		throw ServerConfig::SocketCreationFailed();

	int opt = 1;
	if (setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
		throw ServerConfig::SocketCreationFailed();
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(_port);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	// Bind the socket to the address and port
	if (bind(m_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
		throw ServerConfig::SocketBindingFailed();

	std::cout << "Server bound to port: " << _port << std::endl;

	// Listen for incoming connections
	if (listen(m_socket, SOMAXCONN) < 0)
		throw ServerConfig::SocketListeningFailed();

	std::cout << "Server is listening on port: " << _port << std::endl;

	// Set the server socket to non-blocking mode
	if (set_nonblocking(m_socket) == -1)
		throw ServerConfig::SocketCreationFailed();

	// Return the socket descriptor
	return m_socket;
}


/* ---------------------- Setters ---------------------- */


void ServerConfig::setHost(const std::string& host) {
	_host = host;
}

void ServerConfig::setPort(int port) {
	_port = port;
}

void ServerConfig::setServerName(const std::vector<std::string>& tokens) {
	_serverNames = tokens;
}

void ServerConfig::setRoot(const std::vector<std::string>& tokens) {
	if (!tokens.empty()) {
		_root = tokens[0];
	}
}

void ServerConfig::setErrorPage(const std::vector<std::string>& tokens) {
	if (tokens.size() >= 2) {
		int code = atoi(tokens[0].c_str());
		 _errorPages[code] = tokens[1];
	}
}

void ServerConfig::setClientMaxBodySize(size_t token)
{
	if (token < 0)
		throw std::runtime_error("Error: client_max_body_size must be a positive integer");
	else
		_clientMaxBodySize = token;
}

void ServerConfig::setCgiEnabled(const std::vector<std::string>& tokens) {
	if (!tokens.empty()) {
		std::string val = tokens[0];
		_cgiEnabled = (val == "on" || val == "1" || val == "true");
	}
}

void ServerConfig::setCgiExtension(const std::vector<std::string>& tokens) {
	if (!tokens.empty()) {
		_cgiExtension = tokens[0];
	}
}

void ServerConfig::setCgiBin(const std::vector<std::string>& tokens) {
	if (!tokens.empty()) {
		_cgiBin = tokens[0];
	}
}

/* ---------------------- Getters ---------------------- */

std::string ServerConfig::getHost() const {
	return _host;
}

int ServerConfig::getPort() const {
	return _port;
}

std::string ServerConfig::getServerName(size_t idx) const {
	if (idx < _serverNames.size()) {
		return _serverNames[idx];
	}
	return "";
}

std::vector<std::string> ServerConfig::getServerName() const {
	return _serverNames;
}

std::string ServerConfig::getRoot() const {
	return _root;
}

int ServerConfig::getClientMaxBodySize() const {
	return static_cast<int>(_clientMaxBodySize);
}

bool ServerConfig::isCgiEnabled() const {
	return _cgiEnabled;
}

std::string ServerConfig::getCgiExtension() const {
	return _cgiExtension;
}

std::string ServerConfig::getCgiBin() const {
	return _cgiBin;
}

/* ---------------- Locations Accessors ---------------- */


const std::map<std::string, Location>& ServerConfig::getLocations() const {
	return _locations; //returns the whole map
}


void ServerConfig::addLocation(const std::string& path, const Location& location) {
	_locations[path] = location;
}

/* ---------------- Error pages methods and accesors ---------------- */

void ServerConfig::addErrorPage(int code, const std::string& page)
{
		_errorPages[code] = page;
}

	const std::map<int, std::string>& ServerConfig::getErrorPages() const {
		return _errorPages;
	}

/*
	Goes through the errorPage map and check if all the error pages are there and if their path is accesible
	if the errorcode is not there or if the path is not accesible, the default page and path will be added
*/
void  ServerConfig::checkErrorPage()
{
	std::vector<int> errorsToCheck;
	errorsToCheck.push_back(400);
	errorsToCheck.push_back(403);
	errorsToCheck.push_back(404);
	errorsToCheck.push_back(405);
	errorsToCheck.push_back(413);
	errorsToCheck.push_back(415);
	errorsToCheck.push_back(500);

	std::map<int, std::string> defaultErrorPaths;
	defaultErrorPaths[400] = "/default/error/400.html";
	defaultErrorPaths[403] = "/default/error/403.html";
	defaultErrorPaths[404] = "/default/error/404.html";
	defaultErrorPaths[405] = "/default/error/405.html";
	defaultErrorPaths[413] = "/default/error/413.html";
	defaultErrorPaths[415] = "/default/error/415.html";
	defaultErrorPaths[500] = "/default/error/500.html";

	for (size_t i = 0; i < errorsToCheck.size(); ++i)
	{
		int errorCode = errorsToCheck[i];
		if (_errorPages.find(errorCode) == _errorPages.end())
		{
			std::string defaultErrorPagePath = getErrorFilePath(errorCode);
			_errorPages[errorCode] = defaultErrorPagePath;
			std::cout << "Added default error page for code " << errorCode << ": " << defaultErrorPagePath << std::endl;
		}
		else
		{
			const std::string& filePath = _errorPages[errorCode];
			if (access(filePath.c_str(), F_OK) == -1)
			{
				std::string defaultErrorPagePath = getErrorFilePath(errorCode);
				_errorPages[errorCode] = defaultErrorPagePath;
				std::cout << "Replaced with default error page: " << defaultErrorPagePath << std::endl;
			}
			else
				std::cout << "Error page for code " << errorCode << " is valid: " << filePath << std::endl;
		}
	}
}

std::string ServerConfig::getExecutablePath() {
	char result[PATH_MAX];
	ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
	return std::string(result, (count > 0) ? count : 0);
}

int ServerConfig::getSocket() const {
	return m_socket;
}

std::string& ServerConfig::getClientData(int client_fd) {
	return client_data[client_fd];
}

void ServerConfig::eraseClientData(int client_fd) {
	client_data.erase(client_fd);
}

std::string ServerConfig::getErrorFilePath(int errorCode)
{
	std::string execPath = getExecutablePath();
	std::string execDir = execPath.substr(0, execPath.find_last_of("/"));

	std::ostringstream oss;
	oss << errorCode;

	std::string errorFilePath = execDir + "/default/error/" + oss.str() + ".html";

	return errorFilePath;
}

/* ---------------- Methods ---------------- */

/*Try to find a location with the given location path, if no location found throws an error */
Location ServerConfig::findLocation(std::string locationPath)
{
	 std::map<std::string, Location>::iterator it = _locations.find(locationPath);

	if (it != _locations.end())
		return (it->second);
	else
		throw  ServerConfig::LocationNotFound();

}

/*           Overload operator            */

std::ostream& operator<<(std::ostream& os, const ServerConfig& server) {

	os << "\n### Server ###" << std::endl;

	os << "Port: " << server.getPort() << std::endl;
	os << "Host: " << server.getHost() << std::endl;

	os << std::endl;

	os << "Server Names: ";
	for (size_t i = 0; i < server.getServerName().size(); ++i) {
		os << server.getServerName()[i] << " ";
	}
	os << std::endl;

	os << "Root: " << server.getRoot() << std::endl;

	os << "Error Pages:\n";
	for (std::map<int, std::string>::const_iterator it = server.getErrorPages().begin(); it != server.getErrorPages().end(); ++it) {
		os << "Error Code " << it->first << " -> " << it->second << std::endl;
	}

	os << "Locations: " << std::endl;
	for (std::map<std::string, Location>::const_iterator it = server.getLocations().begin(); it != server.getLocations().end(); ++it) {
		os << it->first << ":";
		os << it->second;
	}

	return os;
}

/*           Exceptions         */

const char* ServerConfig::SocketCreationFailed::what() const throw () {
	return "Throwing exception: creating server socket";
}

const char* ServerConfig::SocketBindingFailed::what() const throw () {
	return "Throwing exception: socket binding failed";
}

const char* ServerConfig::SocketAcceptFailed::what() const throw () {
	return "Throwing exception: Failed to accept connection";
}

const char* ServerConfig::SocketReadFailed::what() const throw () {
	return "Throwing exception: Failed to read from client";
}

const char* ServerConfig::LocationNotFound::what() const throw () {
	return "Throwing exception: Location not found";
}

const char* ServerConfig::SocketListeningFailed::what() const throw () {
	return "Throwing exception: socket listening failed";
}

