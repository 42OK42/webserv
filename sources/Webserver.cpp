/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserver.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: okrahl <okrahl@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/16 15:06:19 by ecarlier          #+#    #+#             */
/*   Updated: 2024/11/05 14:30:37 by okrahl           ###   ########.fr       */
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
	for (size_t i = 0; i < _servers.size(); ++i) {
		ServerConfig& server = _servers[i];
		try {
			int server_socket = server.setupServerSocket();
			struct pollfd server_fd;
			server_fd.fd = server_socket;
			server_fd.events = POLLIN;
			fds.push_back(server_fd);
			std::cout << "Server initialized on " << server.getHost() << ":" << server.getPort() << std::endl;
		}
		catch (const std::exception& e) {
			std::cerr << "Error initializing server: " << e.what() << std::endl;
		}
	}

	runEventLoop();
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
		// std::cout << "Accepted new connection: " << new_fd << std::endl;
		setNonBlocking(new_fd);
		setSocketTimeout(new_fd, 300);
		struct pollfd new_pollfd;
		new_pollfd.fd = new_fd;
		new_pollfd.events = POLLIN;
		fds.push_back(new_pollfd);
	} else {
		perror("accept");
	}
}

void Webserver::handleClientData(size_t index) {
	int client_fd = fds[index].fd;
	std::string& requestData = _servers[0].getClientData(client_fd);

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

		ServerConfig* matchedServer = findMatchingServer(requestHost, requestPort);

		if (matchedServer != NULL) {
			processRequest(httpRequest, matchedServer, client_fd);
		} else {
			std::cerr << "No matching server configuration found for host: " << requestHost << " and port: " << requestPort << std::endl;
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
	for (size_t i = 0; i < _servers.size(); ++i) {
		const ServerConfig& server = _servers[i];
		
		// Check if the port matches
		if (server.getPort() == port) {
			// Check if the host matches
			if (server.getHost() == host) {
				return &_servers[i];
			}

			// Check if the host matches any of the server names
			const std::vector<std::string>& serverNames = server.getServerName();
			if (std::find(serverNames.begin(), serverNames.end(), host) != serverNames.end()) {
				return &_servers[i];
			}
		}
	}
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