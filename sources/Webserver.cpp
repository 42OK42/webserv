/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserver.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: okrahl <okrahl@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/16 15:06:19 by ecarlier          #+#    #+#             */
/*   Updated: 2024/11/05 15:52:02 by okrahl           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserver.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "Router.hpp"
#include <iostream>
#include <vector>
#include <sys/poll.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <csignal>
#include <set>
#include <utility>  // für std::pair

Webserver::Webserver() {}

Webserver::Webserver(const std::vector<ServerConfig>& servers) : _servers(servers) {
	// Initialisierungen, falls nötig
}

Webserver::~Webserver() {
	for (std::vector<struct pollfd>::iterator it = fds.begin(); it != fds.end(); ++it) {
		close(it->fd);
	}
}

void Webserver::initializeServers() {
	std::set<int> initializedPorts;  // Nur ein Socket pro Port

	for (size_t i = 0; i < _servers.size(); ++i) {
		ServerConfig& server = _servers[i];
		int port = server.getPort();

		if (initializedPorts.find(port) != initializedPorts.end()) {
			continue;  // Port bereits initialisiert
		}

		try {
			int server_socket = server.setupServerSocket();
			struct pollfd server_fd;
			server_fd.fd = server_socket;
			server_fd.events = POLLIN;
			fds.push_back(server_fd);
			initializedPorts.insert(port);
			std::cout << "Socket initialized for port " << port << std::endl;
		}
		catch (const std::exception& e) {
			std::cerr << "Error initializing socket for port " << port << ": " << e.what() << std::endl;
		}
	}

	if (!fds.empty()) {
		// Zeige alle verfügbaren Server-Konfigurationen
		for (size_t i = 0; i < _servers.size(); ++i) {
			std::cout << "Server configuration available: " 
					  << _servers[i].getHost() << ":" 
					  << _servers[i].getPort() 
					  << " (server_name: ";
			const std::vector<std::string>& serverNames = _servers[i].getServerName();
			for (size_t j = 0; j < serverNames.size(); ++j) {
				std::cout << serverNames[j];
				if (j < serverNames.size() - 1) std::cout << ", ";
			}
			std::cout << ")" << std::endl;
		}
		runEventLoop();
	} else {
		throw std::runtime_error("No server sockets could be initialized");
	}
}

void Webserver::runEventLoop() {
	while (true) {
		int ret = poll(&fds[0], fds.size(), -1);
		if (ret < 0) {
			if (errno == EINTR) {
				std::cout << "Shutting down servers due to interrupt signal." << std::endl;
				break;
			} else {
				perror("poll");
				break;
			}
		}

		for (size_t i = 0; i < fds.size(); ++i) {
			if (fds[i].revents & POLLIN) {
				if (isServerSocket(fds[i].fd)) {
					handleNewConnection(fds[i].fd);
				} else {
					handleClientData(i);
				}
			}
		}
	}
}

bool Webserver::isServerSocket(int fd) {
	for (std::vector<ServerConfig>::const_iterator it = _servers.begin(); it != _servers.end(); ++it) {
		if (it->getSocket() == fd) return true;
	}
	return false;
}

void Webserver::handleNewConnection(int server_socket) {
	struct sockaddr_in client_addr;
	socklen_t client_addr_len = sizeof(client_addr);
	int new_fd = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
	if (new_fd >= 0) {
		setNonBlocking(new_fd);
		setSocketTimeout(new_fd, 300);
		struct pollfd new_pollfd;
		new_pollfd.fd = new_fd;
		new_pollfd.events = POLLIN;
		fds.push_back(new_pollfd);
		
		client_to_server[new_fd] = server_socket;
		std::cout << "\033[0;35m[Connection]\033[0m New client " << new_fd 
				  << " connected to server socket " << server_socket << std::endl;
	} else {
		perror("accept");
	}
}

void Webserver::handleClientData(size_t index) {
	int client_fd = fds[index].fd;
	
	int server_socket = client_to_server[client_fd];
	ServerConfig* currentServer = NULL;
	
	for (size_t i = 0; i < _servers.size(); ++i) {
		if (_servers[i].getSocket() == server_socket) {
			currentServer = &_servers[i];
			break;
		}
	}
	
	if (!currentServer) {
		std::cerr << "\033[0;31m[Error]\033[0m No server found for client " << client_fd << std::endl;
		close(client_fd);
		fds.erase(fds.begin() + index);
		return;
	}

	std::string& requestData = currentServer->getClientData(client_fd);

	char buffer[1024];
	int bytesRead = recv(client_fd, buffer, sizeof(buffer), 0);
	if (bytesRead <= 0) {
		if (bytesRead < 0) {
			// std::cerr << "Error reading from client socket: " << strerror(errno) << std::endl;
		}
		close(client_fd);
		fds.erase(fds.begin() + index);
		return;
	}

	requestData.append(buffer, bytesRead);

	if (isCompleteRequest(requestData)) {
		HttpRequest httpRequest(requestData.c_str(), requestData.size());
		httpRequest.print();

		std::string requestHost = httpRequest.getHost();
		int requestPort = httpRequest.getPort();

		// Finde den richtigen Server basierend auf Host und Port
		ServerConfig* matchedServer = findMatchingServer(requestHost, requestPort);
		
		if (matchedServer != NULL) {
			processRequest(httpRequest, matchedServer, client_fd);
		} else {
			std::cerr << "No matching server configuration found for host: " 
					  << requestHost << " and port: " << requestPort << std::endl;
		}

		close(client_fd);
		fds.erase(fds.begin() + index);
	}
}

bool Webserver::isCompleteRequest(const std::string& requestData) {
	size_t headerEnd = requestData.find("\r\n\r\n");
	if (headerEnd != std::string::npos) {
		size_t contentLength = 0;
		size_t contentLengthPos = requestData.find("Content-Length:");
		if (contentLengthPos != std::string::npos) {
			contentLengthPos += 15;
			contentLength = std::atoi(requestData.c_str() + contentLengthPos);
		}
		return requestData.size() >= headerEnd + 4 + contentLength;
	}
	return false;
}

ServerConfig* Webserver::findMatchingServer(const std::string& host, int port) {
	std::cout << "\033[0;33m[Router]\033[0m Searching server for " << host << ":" << port << std::endl;  // Gelb
	
	for (size_t i = 0; i < _servers.size(); ++i) {
		const ServerConfig& server = _servers[i];
		
		if (server.getPort() == port) {
			if (server.getHost() == host) {
				std::cout << "\033[0;32m[Router]\033[0m Found matching server: " 
						  << server.getHost() << ":" << server.getPort() << std::endl;  // Grün
				return &_servers[i];
			}

			const std::vector<std::string>& serverNames = server.getServerName();
			if (std::find(serverNames.begin(), serverNames.end(), host) != serverNames.end()) {
				std::cout << "\033[0;32m[Router]\033[0m Found matching server by name: " 
						  << server.getHost() << ":" << server.getPort() << std::endl;  // Grün
				return &_servers[i];
			}
		}
	}
	
	std::cout << "\033[0;31m[Router]\033[0m No matching server found!" << std::endl;  // Rot
	return NULL;
}

void Webserver::processRequest(HttpRequest& httpRequest, ServerConfig* server, int client_fd) {
	// std::cout << "Processing request for server: " 
	//           << server->getHost() << ":" << server->getPort() << std::endl;

	HttpResponse httpResponse(httpRequest);
	Router router(*server);
	
	router.initializeRoutes();
	router.handleRequest(httpRequest, httpResponse);
	
	std::string httpResponseString = httpResponse.toString();
	if (send(client_fd, httpResponseString.c_str(), httpResponseString.size(), 0) < 0) {
		// std::cerr << "Error sending response: " << strerror(errno) << std::endl;
	} else {
		// std::cout << "Response sent to client: " << client_fd << std::endl;
	}
	
	server->eraseClientData(client_fd);
}

void Webserver::setNonBlocking(int sockfd) {
	int flags = fcntl(sockfd, F_GETFL, 0);
	if (flags == -1) {
		perror("fcntl");
		return;
	}
	if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1) {
		perror("fcntl");
	}
}

void Webserver::setSocketTimeout(int sockfd, int timeout_seconds) {
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