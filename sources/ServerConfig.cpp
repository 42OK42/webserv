/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: okrahl <okrahl@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/18 17:04:09 by okrahl            #+#    #+#             */
/*   Updated: 2024/10/18 17:04:26 by okrahl           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerConfig.hpp"
#include "Router.hpp"
#include "HttpResponse.hpp"
#include "HttpRequest.hpp"
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>

// Set the socket to non-blocking mode
int set_nonblocking(int sockfd) {
	int flags = fcntl(sockfd, F_GETFL, 0);
	if (flags == -1) {
		return -1;
	}
	return fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
}

// Destructeur
ServerConfig::~ServerConfig() {
	if (m_socket != -1) {
		close(m_socket);
	}
}

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

bool ServerConfig::readClientData(int client_fd) {
	std::cout << "readClientData called for fd: " << client_fd << std::endl;
	char buffer[1024];
	int n = recv(client_fd, buffer, sizeof(buffer), 0);
	if (n <= 0) {
		// Connection closed or error
		if (n < 0) {
			std::cerr << "Error reading from client socket: " << strerror(errno) << std::endl;
		}
		close(client_fd);
		return false;
	}

	std::cout << "recv returned " << n << " bytes" << std::endl;

	// Append data to client's buffer
	client_data[client_fd].append(buffer, n);

	// Print everything received
	std::cout << "Received data from client (first 1000 chars): " << client_data[client_fd].substr(0, 1000) << std::endl;
	if (client_data[client_fd].size() > 1000) {
		std::cout << "  (truncated, total size: " << client_data[client_fd].size() << " bytes)\n";
	}

	// Check if the entire request has been received
	std::string& data = client_data[client_fd];
	size_t header_end_pos = data.find("\r\n\r\n");
	if (header_end_pos != std::string::npos) {
		size_t content_length = 0;
		size_t content_length_pos = data.find("Content-Length:");
		if (content_length_pos != std::string::npos) {
			content_length_pos += 15; // Skip "Content-Length:"
			content_length = std::atoi(data.c_str() + content_length_pos);
			std::cout << "Content-Length: " << content_length << std::endl;
		}

		if (data.size() >= header_end_pos + 4 + content_length) {
			std::cout << "Entire request has been received. Total size: " << data.size() << std::endl;
			return true; // Entire request has been received
		} else {
			std::cout << "Request not fully received yet. Current size: " << data.size() << " Expected size: " << (header_end_pos + 4 + content_length) << std::endl;
		}
	} else {
		std::cout << "header_end_pos not found" << std::endl;
	}

	return false; // Request not fully received yet
}

int ServerConfig::startServer() {
	std::cout << "Starting server..." << std::endl;

	m_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (m_socket == -1) throw ServerConfig::SocketCreationFailed();

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(_port[0]);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(m_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
		throw ServerConfig::SocketBindingFailed();

	if (listen(m_socket, SOMAXCONN) < 0)
		throw ServerConfig::SocketlisteningFailed();

	std::cout << "Server is listening on port " << _port[0] << std::endl;

	// Set the server socket to non-blocking mode
	if (set_nonblocking(m_socket) == -1) throw ServerConfig::SocketCreationFailed();

	// Initialize pollfd structures
	struct pollfd server_fd;
	server_fd.fd = m_socket;
	server_fd.events = POLLIN;
	fds.push_back(server_fd);

	// Initialize router
	Router router;
	router.initializeRoutes();

	while (true) {
		int ret = poll(&fds[0], fds.size(), -1); // -1 means wait indefinitely
		if (ret < 0) {
			perror("poll");
			break;
		}

		std::cout << "Poll returned with ret: " << ret << std::endl;

		for (size_t i = 0; i < fds.size(); ++i) {
			std::cout << "Checking fd: " << fds[i].fd << " revents: " << fds[i].revents << std::endl;
			if (fds[i].revents & POLLIN) {
				if (fds[i].fd == m_socket) {
					// Accept new connection
					socklen_t client_addr_len = sizeof(client_addr);
					int new_fd = accept(m_socket, (struct sockaddr*)&client_addr, &client_addr_len);
					if (new_fd >= 0) {
						std::cout << "Accepted new connection: " << new_fd << std::endl;
						if (set_nonblocking(new_fd) == -1) {
							close(new_fd);
							continue;
						}
						set_socket_timeout(new_fd, 300); // Set a 300-second timeout for read/write operations
						struct pollfd new_pollfd;
						new_pollfd.fd = new_fd;
						new_pollfd.events = POLLIN;
						fds.push_back(new_pollfd);
					} else {
						perror("accept");
					}
				} else {
					// Read data from existing client socket
					std::cout << "Calling readClientData for fd: " << fds[i].fd << std::endl;
					if (readClientData(fds[i].fd)) {
						// Entire request has been received
						std::cout << "Entire request received for fd: " << fds[i].fd << std::endl;
						HttpRequest httpRequest(client_data[fds[i].fd].c_str(), client_data[fds[i].fd].size());

						HttpResponse httpResponse(httpRequest);

						// Process request with the router
						router.handleRequest(httpRequest, httpResponse);

						// Send response
						std::string httpResponseString = httpResponse.toString();
						if (send(fds[i].fd, httpResponseString.c_str(), httpResponseString.size(), 0) < 0) {
							std::cerr << "Error sending response: " << strerror(errno) << std::endl;
						} else {
							std::cout << "Response sent to client: " << fds[i].fd << std::endl;
						}

						// Clean up
						client_data.erase(fds[i].fd);
						close(fds[i].fd);
						fds.erase(fds.begin() + i);
						--i;
					}
				}
			}
		}
	}

	return 0;
}

/* ---------------------- Setters ---------------------- */

void ServerConfig::setPort(const std::vector<std::string>& tokens) {
    _port.clear();
    for (size_t i = 0; i < tokens.size(); ++i) {
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

void ServerConfig::setClientMaxBodySize(size_t token) {
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

const char* ServerConfig::SocketCreationFailed::what() const throw() {
	return "Throwing exception: creating server socket";
}

const char* ServerConfig::SocketBindingFailed::what() const throw() {
	return "Throwing exception: socket binding failed";
}

const char* ServerConfig::SocketlisteningFailed::what() const throw() {
	return "Throwing exception: socket listening failed";
}

const char* ServerConfig::SocketAcceptFailed::what() const throw() {
	return "Throwing exception: Failed to accept connection";
}

const char* ServerConfig::SocketReadFailed::what() const throw() {
	return "Throwing exception: Failed to read from client";
}