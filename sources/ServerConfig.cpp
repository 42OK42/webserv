/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ecarlier <ecarlier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/09 15:30:30 by ecarlier          #+#    #+#             */
/*   Updated: 2024/10/21 18:50:20 by ecarlier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/ServerConfig.hpp"
#include "../includes/Router.hpp"
#include "../includes/HttpResponse.hpp"
#include "../includes/HttpRequest.hpp"

// #include "Parser.hpp"

// Destructeur
ServerConfig::~ServerConfig() {}


std::string ServerConfig::readFile(const std::string& filepath) {
	std::ifstream file(filepath.c_str());
	if (!file) {
		std::cerr << "Could not open the file: " << filepath << std::endl;
		return "";
	}
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

int ServerConfig::startServer()
{
	std::cout << "Starting server..." << std::endl;

	m_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (m_socket == -1) throw ServerConfig::SocketCreationFailed();

	server_addr.sin_family = AF_INET;
	// server_addr.sin_port = htons(8080);
    server_addr.sin_port = htons(_port[0]);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(m_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
		throw ServerConfig::SocketBindingFailed();

	if (listen(m_socket, SOMAXCONN) < 0)
		throw ServerConfig::SocketlisteningFailed();

    std::cout << "Server is listening on port " << _port[0] << std::endl;
	//std::cout << "Server is listening on port 8080..." << std::endl;

	socklen_t client_addr_len = sizeof(client_addr);

	// Initialize router
	Router router;
	router.initializeRoutes();

	while (true) {
		client_socket = accept(m_socket, (struct sockaddr*)&client_addr, &client_addr_len);
		if (client_socket < 0) throw SocketAcceptFailed();

		char buffer[1024] = {0};
		int bytes_read = recv(client_socket, buffer, sizeof(buffer), 0);
		if (bytes_read < 0) throw SocketReadFailed();

		std::cout << "Received..." << std::endl;

		HttpRequest httpRequest(buffer, bytes_read);

		//httpRequest.print();

		HttpResponse httpResponse(httpRequest);

		// Process request with the router
		router.handleRequest(httpRequest, httpResponse);

		// Send response
		std::string httpResponseString = httpResponse.toString();
		send(client_socket, httpResponseString.c_str(), httpResponseString.size(), 0);

		close(client_socket);
	}

	return 0;
}




/* ---------------------- Setters ---------------------- */

void ServerConfig::setPort(const std::vector<std::string>& tokens)
{
    _port.clear();
    for (size_t i = 0; i < tokens.size(); ++i)
    {
        _port.push_back(atoi(tokens[i].c_str()));
    }
}

void ServerConfig::setHost(const std::vector<std::string>& tokens) {
    _host = tokens;
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

int ServerConfig::getListen(size_t idx) const {
    if (idx < _port.size()) {
        return _port[idx];
    }
    return -1;
}

size_t ServerConfig::getNbOfPorts() const {
    return _port.size();
}

std::vector<int> ServerConfig::getListen() const {
    return _port;
}

std::string ServerConfig::getHost(size_t idx) const {
    if (idx < _host.size()) {
        return _host[idx];
    }
    return "";
}

std::vector<std::string> ServerConfig::getHost() const {
    return _host;
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

// std::string ServerConfig::getErrorPage(int code) const {
//     auto it = _errorPages.find(code);
//     if (it != _errorPages.end()) {
//         return it->second;
//     }
//     return "";
// }

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

/* ---------------- Error pages Accessors ---------------- */

void ServerConfig::addErrorPage(int code, const std::string& page) {
        _errorPages[code] = page;
    }

    const std::map<int, std::string>& ServerConfig::getErrorPages() const {
        return _errorPages;
    }

/*           Overload operator            */

std::ostream& operator<<(std::ostream& os, const ServerConfig& server) {

    os << "\n### Server ###" << std::endl;
    os << "Ports: ";
    for (size_t i = 0; i < server.getListen().size(); ++i) {
        os << server.getListen()[i] << " ";
    }
    os << std::endl;

    os << "Hosts: ";
    for (size_t i = 0; i < server.getHost().size(); ++i) {
        os << server.getHost()[i] << " ";
    }
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

const char* ServerConfig::SocketlisteningFailed::what() const throw () {
	return "Throwing exception: socket listening failed";
}

const char* ServerConfig::SocketAcceptFailed::what() const throw () {
	return "Throwing exception: Failed to accept connection";
}

const char* ServerConfig::SocketReadFailed::what() const throw () {
	return "Throwing exception: Failed to read from client";
}

