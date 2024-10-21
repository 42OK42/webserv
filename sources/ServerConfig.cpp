/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ecarlier <ecarlier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/09 15:30:30 by ecarlier          #+#    #+#             */
/*   Updated: 2024/10/18 20:40:15 by ecarlier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/ServerConfig.hpp"
#include "../includes/Router.hpp"
#include "../includes/HttpResponse.hpp"
#include "../includes/HttpRequest.hpp"

// #include "Parser.hpp"

ServerConfig::ServerConfig() {}


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


void ServerConfig::setupServerSocket()
{
    m_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_socket == -1)
        throw ServerConfig::SocketCreationFailed();

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(_port);

    // Set the IP address based on the host
    if (_host == "localhost")
        server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    else
        server_addr.sin_addr.s_addr = inet_addr(_host.c_str());

    // Bind the socket to the address and port
    if (bind(m_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
        throw ServerConfig::SocketBindingFailed();

    // Listen for incoming connections
    if (listen(m_socket, SOMAXCONN) < 0)
        throw ServerConfig::SocketlisteningFailed();

    std::cout << "Server is listening on port " << _port << std::endl;


    	// socklen_t client_addr_len = sizeof(client_addr);

	// Initialize router
	Router router;
	router.initializeRoutes(); //@olli should it initialise from the location??

}



/* SHOULD DIVIDE THIS FUNCTION IN TWO DIFFERENT FUNCTION, THE SOCKET PART THEN THE LISTENING */
int ServerConfig::startServer()
{
	std::cout << "Starting server ..." << std::endl;

	m_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (m_socket == -1) throw ServerConfig::SocketCreationFailed();

    /*need to create a socket for each port*/

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(8080);
    //server_addr.sin_port = htons(_port[0]);

	server_addr.sin_addr.s_addr = INADDR_ANY;

  // server_addr.sin_addr.s_addr = inet_addr(_host[0].c_str());
    // if (_host[0] == "localhost")
	// 	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");


	if (bind(m_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
		throw ServerConfig::SocketBindingFailed();

	if (listen(m_socket, SOMAXCONN) < 0)
		throw ServerConfig::SocketlisteningFailed();

    //std::cout << "Server is listening on port " << _port[0] << std::endl;


	socklen_t client_addr_len = sizeof(client_addr);

	// Initialize router
	Router router;
	router.initializeRoutes(); //@olli should it initialise from the location??


	while (true) {
		client_socket = accept(m_socket, (struct sockaddr*)&client_addr, &client_addr_len);
		if (client_socket < 0) throw SocketAcceptFailed();

		char buffer[1024] = {0};
		int bytes_read = recv(client_socket, buffer, sizeof(buffer), 0);
		if (bytes_read < 0) throw SocketReadFailed(); //should check fro both, maybe diff errors

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

/* ---------------- Error pages Accessors ---------------- */

void ServerConfig::addErrorPage(int code, const std::string& page)
{
        _errorPages[code] = page;
}

    const std::map<int, std::string>& ServerConfig::getErrorPages() const {
        return _errorPages;
    }


// void  ServerConfig::checkErrorPage()
// {

// }

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

const char* ServerConfig::SocketlisteningFailed::what() const throw () {
	return "Throwing exception: socket listening failed";
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

