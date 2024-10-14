/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TcpServer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: okrahl <okrahl@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/30 15:38:05 by ecarlier          #+#    #+#             */
/*   Updated: 2024/10/14 16:13:40 by okrahl           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "TcpServer.hpp"

TcpServer::TcpServer() {
	std::cout << "\033[33m" << "Default constructor called" << "\033[0m" << std::endl;
}

TcpServer::~TcpServer() {
	std::cout << "\033[32m" << "Destructor called" << "\033[0m" << std::endl;
}

// Function to read an HTML file
std::string TcpServer::readFile(const std::string& filepath) {
	std::ifstream file(filepath.c_str());
	if (!file) {
		std::cerr << "Could not open the file: " << filepath << std::endl;
		return "";
	}
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

int TcpServer::startServer() {
	std::cout << "Starting server..." << std::endl;

	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket == -1) throw TcpServer::SocketCreationFailed();

	int opt = 1;
	if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
		throw TcpServer::SocketBindingFailed();
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(8080);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
		throw TcpServer::SocketBindingFailed();

	if (listen(server_socket, SOMAXCONN) < 0)
		throw TcpServer::SocketListeningFailed();

	std::cout << "Server is listening on port 8080..." << std::endl;

	// Initialize pollfd structures
	fds[0].fd = server_socket;
	fds[0].events = POLLIN;
	nfds = 1;

	Router router;
	router.initializeRoutes();

	while (true) {
		int poll_count = poll(fds, nfds, -1);

		if (poll_count < 0) {
			perror("poll");
			break;
		}

		for (int i = 0; i < nfds; i++) {
			if (fds[i].revents & POLLIN) {
				if (fds[i].fd == server_socket) {
					// Accept new connection
					socklen_t client_addr_len = sizeof(client_addr);
					client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
					if (client_socket >= 0) {
						fcntl(client_socket, F_SETFL, O_NONBLOCK);
						fds[nfds].fd = client_socket;
						fds[nfds].events = POLLIN;
						nfds++;
					}
				} else {
					// Handle the client request
					HttpRequest httpRequest;
					if (httpRequest.readRequest(fds[i].fd)) {
						std::cout << "Received..." << std::endl;

						HttpResponse httpResponse(httpRequest);

						// Process request with the router
						router.handleRequest(httpRequest, httpResponse);

						// Send response
						std::string httpResponseString = httpResponse.toString();
						send(fds[i].fd, httpResponseString.c_str(), httpResponseString.size(), 0);

						// Close connection
						close(fds[i].fd);

						// Remove the client socket from the poll list
						fds[i] = fds[nfds - 1];
						nfds--;
					}
				}
			}
		}
	}

	return 0;
}

const char* TcpServer::SocketCreationFailed::what() const throw () {
	return "Throwing exception: creating server socket";
}

const char* TcpServer::SocketBindingFailed::what() const throw () {
	return "Throwing exception: socket binding failed";
}

const char* TcpServer::SocketListeningFailed::what() const throw () {
	return "Throwing exception: socket listening failed";
}

const char* TcpServer::SocketAcceptFailed::what() const throw () {
	return "Throwing exception: Failed to accept connection";
}

const char* TcpServer::SocketReadFailed::what() const throw () {
	return "Throwing exception: Failed to read from client";
}