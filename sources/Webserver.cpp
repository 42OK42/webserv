/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserver.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: okrahl <okrahl@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/16 15:06:19 by ecarlier          #+#    #+#             */
/*   Updated: 2024/11/13 17:30:50 by okrahl           ###   ########.fr       */
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
	std::set<int> initializedPorts;  // Track initialized ports to avoid duplicates

	for (size_t i = 0; i < _servers.size(); ++i) {
		ServerConfig& server = _servers[i];
		int port = server.getPort();

		// Skip if port already initialized
		if (initializedPorts.find(port) != initializedPorts.end()) {
			continue;
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
		// Display available server configurations
		for (size_t i = 0; i < _servers.size(); ++i) {
			std::cout << "Server configuration available: " 
					  << _servers[i].getHost() << ":" 
					  << _servers[i].getPort() 
					  << " (server_name: ";
			//const std::vector<std::string>& serverNames = _servers[i].getServerName();
			// for (size_t j = 0; j < serverNames.size(); ++j) {
			// 	std::cout << serverNames[j];
			// 	if (j < serverNames.size() - 1) std::cout << ", ";
			// }
			std::cout << ")" << std::endl;
		}
		runEventLoop();
	} else {
		throw std::runtime_error("No server sockets could be initialized");
	}
}

void Webserver::runEventLoop() {
	int poll_count = poll(&fds[0], fds.size(), 1000);  // 1 second timeout
	
	if (poll_count < 0) {
		if (errno != EINTR) {  // Ignore if interrupted by signal
			std::cerr << "Poll error: " << strerror(errno) << std::endl;
		}
		return;
	}
	
	// Check each file descriptor for events
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

bool Webserver::isServerSocket(int fd) {
	for (size_t i = 0; i < _servers.size(); ++i) {
		if (_servers[i].getSocket() == fd) {
			return true;
		}
	}
	return false;
}

void Webserver::handleNewConnection(int server_socket) {
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);
	
	int new_fd = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
	if (new_fd < 0) {
		std::cerr << "Accept error: " << strerror(errno) << std::endl;
		return;
	}
	
	setNonBlocking(new_fd);
	setSocketTimeout(new_fd, 60);  // 60 second timeout
	
	struct pollfd client_fd;
	client_fd.fd = new_fd;
	client_fd.events = POLLIN;
	fds.push_back(client_fd);
	
	client_to_server[new_fd] = server_socket;
	std::cout << "\033[0;35m[Connection]\033[0m New client " << new_fd 
			  << " connected to server socket " << server_socket << std::endl;
}

void Webserver::handleClientData(size_t index) {
	int client_fd = fds[index].fd;
	int server_socket = client_to_server[client_fd];
	
	ServerConfig* server = NULL;
	for (size_t i = 0; i < _servers.size(); ++i) {
		if (_servers[i].getSocket() == server_socket) {
			server = &_servers[i];
			break;
		}
	}
	
	if (!server) {
		#ifdef DEBUG_MODE
		std::cerr << "\033[0;31m[DEBUG] Webserver::handleClientData: No server found for socket " 
				  << server_socket << "\033[0m" << std::endl;
		#endif
		closeConnection(index);
		return;
	}
	
	try {
		bool requestComplete = server->readClientData(client_fd);
		if (!requestComplete) {
			return;  // Warte auf mehr Daten
		}

		std::string& requestData = server->getClientData(client_fd);
		
		// Prüfe auf leere Requests
		if (requestData.empty()) {
			closeConnection(index);
			return;
		}

		try {
			HttpRequest httpRequest(requestData.c_str(), requestData.length(), *server);
			
			// Prüfe Connection Header
			bool shouldClose = (httpRequest.getHeader("Connection") == "close");
			
			#ifdef DEBUG_MODE
			std::cout << "\033[0;36m[DEBUG] Webserver::handleClientData: Connection Header = " 
					  << httpRequest.getHeader("Connection") << "\033[0m" << std::endl;
			#endif

			ServerConfig* matchingServer = findMatchingServer(httpRequest.getHost(), httpRequest.getPort());
			if (matchingServer) {
				processRequest(httpRequest, matchingServer, client_fd);
			} else {
				processRequest(httpRequest, server, client_fd);
			}

			// Lösche nur die Request-Daten, nicht die Verbindung
			server->eraseClientData(client_fd);
			
			// Schließe nur wenn nötig
			if (shouldClose) {
				#ifdef DEBUG_MODE
				std::cout << "\033[0;36m[DEBUG] Webserver::handleClientData: Schließe Verbindung auf Anfrage\033[0m" << std::endl;
				#endif
				closeConnection(index);
			}
			
		} catch (const std::runtime_error& e) {
			if (std::string(e.what()) == "Request body exceeds maximum allowed size") {
				#ifdef DEBUG_MODE
				std::cerr << "\033[0;31m[DEBUG] Webserver::handleClientData: Datei zu groß, sende 413\033[0m" << std::endl;
				#endif

				// Erstelle eine neue HttpResponse ohne Request
				HttpResponse errorResponse;
				Router router(*server);
				
				// Nutze die vorhandene Fehlerbehandlung
				router.setErrorResponse(errorResponse, 413);
				
				std::string responseStr = errorResponse.toString();
				
				send(client_fd, responseStr.c_str(), responseStr.length(), MSG_NOSIGNAL);
				
				// Lösche die Request-Daten und schließe die Verbindung
				server->eraseClientData(client_fd);
				closeConnection(index);
				return;
			}
			closeConnection(index);
			return;
		}
		
	} catch (const std::exception& e) {
		#ifdef DEBUG_MODE
		std::cerr << "\033[0;31m[DEBUG] Webserver::handleClientData: Fehler: " << e.what() << "\033[0m" << std::endl;
		#endif
		closeConnection(index);
	}
}

void Webserver::closeConnection(size_t index) {
	int client_fd = fds[index].fd;
	#ifdef DEBUG_MODE
	std::cout << "\033[0;36m[DEBUG] Webserver::closeConnection: Schließe Client " << client_fd << "\033[0m" << std::endl;
	#endif
	close(client_fd);
	fds.erase(fds.begin() + index);
	client_to_server.erase(client_fd);
}

void Webserver::setNonBlocking(int sockfd) {
	int flags = fcntl(sockfd, F_GETFL, 0);
	if (flags == -1) {
		std::cerr << "Error getting socket flags: " << strerror(errno) << std::endl;
		return;
	}
	if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1) {
		std::cerr << "Error setting socket to non-blocking: " << strerror(errno) << std::endl;
	}
}

void Webserver::setSocketTimeout(int sockfd, int timeout_seconds) {
	struct timeval timeout;
	timeout.tv_sec = timeout_seconds;
	timeout.tv_usec = 0;
	
	if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
		std::cerr << "Error setting receive timeout: " << strerror(errno) << std::endl;
	}
	if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0) {
		std::cerr << "Error setting send timeout: " << strerror(errno) << std::endl;
	}
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

			// const std::vector<std::string>& serverNames = server.getServerName();
			// if (std::find(serverNames.begin(), serverNames.end(), host) != serverNames.end()) {
			// 	std::cout << "\033[0;32m[Router]\033[0m Found matching server by name: " 
			// 			  << server.getHost() << ":" << server.getPort() << std::endl;  // Grün
				// return &_servers[i];
			
		}
	}
	
	std::cout << "\033[0;31m[Router]\033[0m No matching server found!" << std::endl;  // Rot
	return NULL;
}

void Webserver::processRequest(HttpRequest& httpRequest, ServerConfig* server, int client_fd) {
	#ifdef DEBUG_MODE
	std::cout << "\033[0;36m[DEBUG] Webserver::processRequest: Processing request for client " 
			  << client_fd << "\033[0m" << std::endl;
	#endif

	HttpResponse httpResponse(httpRequest);
	Router router(*server);
	
	router.handleRequest(httpRequest, httpResponse);
	
	std::string responseStr = httpResponse.toString();
	
	#ifdef DEBUG_MODE
	std::cout << "\033[0;36m[DEBUG] Webserver::processRequest: Sending response:\n" 
			  << responseStr << "\033[0m" << std::endl;
	#endif

	ssize_t total_sent = 0;
	while (total_sent < static_cast<ssize_t>(responseStr.length())) {
		ssize_t sent = send(client_fd, responseStr.c_str() + total_sent, 
						  responseStr.length() - total_sent, MSG_NOSIGNAL);
		if (sent <= 0) {
			#ifdef DEBUG_MODE
			std::cerr << "\033[0;31m[DEBUG] Webserver::processRequest: Error while sending\033[0m" << std::endl;
			#endif
			break;
		}
		total_sent += sent;
	}
}