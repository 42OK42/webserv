/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ecarlier <ecarlier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/09 15:30:30 by ecarlier          #+#    #+#             */
/*   Updated: 2024/10/16 15:57:44 by ecarlier         ###   ########.fr       */
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


// /* SHOULD ONLY GET ONE PORT AND ONE HOST AND SET THOSE */
// void ServerConfig::setupServerSocket()
// {
// 	m_socket = socket(AF_INET, SOCK_STREAM, 0);
// 	if (m_socket == -1)
//         throw ServerConfig::SocketCreationFailed();

// 	server_addr.sin_family = AF_INET;
//     server_addr.sin_port = htons(_port[0]);
//     server_addr.sin_addr.s_addr = inet_addr(_host[0].c_str());
//     if (_host[0] == "localhost")
//         server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

// 	if (bind(m_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
// 		throw ServerConfig::SocketBindingFailed();

// 	if (listen(m_socket, SOMAXCONN) < 0)
// 		throw ServerConfig::SocketlisteningFailed();
// }


/* SHOULD DIVIDE THIS FUNCTION IN TWO DIFFERENT FUNCTION, THE SOCKET PART THEN THE LISTENING */
int ServerConfig::startServer()
{
	std::cout << "Starting server ..." << std::endl;

	m_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (m_socket == -1) throw ServerConfig::SocketCreationFailed();

    /*need to create a socket for each port*/

	server_addr.sin_family = AF_INET;
	// server_addr.sin_port = htons(8080);
    server_addr.sin_port = htons(_port[0]);

	//server_addr.sin_addr.s_addr = INADDR_ANY;

   server_addr.sin_addr.s_addr = inet_addr(_host[0].c_str());
    // if (_host[0] == "localhost")
	// 	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");


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
/*
    - Check if any port(s) are provided, if none are provided it is set to 8080 (default)
    - Check if there is any duplicates, in that case, it ignores the duplicate and does not
        add it to the ports lists
*/
// void ServerConfig::setPort(const std::vector<std::string>& tokens)
// {

//     for (size_t i = 0; i < tokens.size(); ++i) {
//         std::cout << "Token " << i + 1 << ": " << tokens[i] << std::endl;
//     }

//     _port.clear();
//     if (!tokens.empty())
//     {
//         for (size_t i = 0; i < tokens.size(); ++i)
//         {
//             int port = atoi(tokens[i].c_str());
//             bool isDuplicate = false;
//             for (size_t j = 0; j < _port.size(); ++j)
//             {
//                 if (_port[j] == port)
//                 {
//                     isDuplicate = true;
//                     break;
//                 }
//             }
//             if (!isDuplicate)
//             {
//             _port.push_back(port);
//             }
//         }
//     }
//     else
//         _port[0] = 8080;

//}


/* If there is no host specified, it is set to localhost */
void ServerConfig::setHost(const std::vector<std::string>& tokens) {
    if (!tokens.empty())
        _host = tokens;
    else
        _host[0] = "localhost";
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


/* Methods */

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


const char* ServerConfig::LocationNotFound::what() const throw () {
	return "Throwing exception: Location not found";
}

