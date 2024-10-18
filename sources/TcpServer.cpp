/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TcpServer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: okrahl <okrahl@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/30 15:38:05 by ecarlier          #+#    #+#             */
/*   Updated: 2024/10/18 17:21:29 by okrahl           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// #include "TcpServer.hpp"
// #include "Router.hpp"
// #include "HttpResponse.hpp"
// #include "HttpRequest.hpp"
// //#include "ServerConfig.hpp"

// // Set the socket to non-blocking mode
// int set_nonblocking(int sockfd) {
// 	int flags = fcntl(sockfd, F_GETFL, 0);
// 	if (flags == -1) {
// 		return -1;
// 	}
// 	return fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
// }

// TcpServer::TcpServer() : m_socket(-1) {
// 	std::cout << "\033[33m" << "Default constructor called" << "\033[0m" << std::endl;
// }

// TcpServer::TcpServer(const ServerConfig& config) {
// 	(void)config; // Markieren Sie den Parameter als bewusst ungenutzt
// 	std::cout << "\033[33m" << "Parameterized constructor called" << "\033[0m" << std::endl;
// }

// TcpServer& TcpServer::operator=(const ServerConfig& config) {
// 	(void)config; // Markieren Sie den Parameter als bewusst ungenutzt
// 	std::cout << "\033[33m" << "Assignment operator called" << "\033[0m" << std::endl;
// 	return *this;
// }

// TcpServer::~TcpServer() {
// 	std::cout << "\033[32m" << "Destructor called" << "\033[0m" << std::endl;
// 	if (m_socket != -1) {
// 		close(m_socket);
// 	}
// }

// // Set the socket timeout
// void TcpServer::set_socket_timeout(int sockfd, int timeout_seconds) {
// 	struct timeval timeout;
// 	timeout.tv_sec = timeout_seconds;
// 	timeout.tv_usec = 0;

// 	if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
// 		perror("setsockopt SO_RCVTIMEO");
// 	}
// 	if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0) {
// 		perror("setsockopt SO_SNDTIMEO");
// 	}
// }

// std::string TcpServer::readFile(const std::string& filepath) {
// 	std::ifstream file(filepath.c_str());
// 	if (!file) {
// 		std::cerr << "Could not open the file: " << filepath << std::endl;
// 		return "";
// 	}
// 	std::stringstream buffer;
// 	buffer << file.rdbuf();
// 	return buffer.str();
// }

// bool TcpServer::readClientData(int client_fd) {
// 	std::cout << "readClientData called for fd: " << client_fd << std::endl;
// 	char buffer[1024];
// 	int n = recv(client_fd, buffer, sizeof(buffer), 0);
// 	if (n <= 0) {
// 		// Connection closed or error
// 		if (n < 0) {
// 			std::cerr << "Error reading from client socket: " << strerror(errno) << std::endl;
// 		}
// 		close(client_fd);
// 		return false;
// 	}

// 	std::cout << "recv returned " << n << " bytes" << std::endl;

// 	// Append data to client's buffer
// 	client_data[client_fd].append(buffer, n);

// 	// Print everything received
// 	std::cout << "Received data from client (first 1000 chars): " << client_data[client_fd].substr(0, 1000) << std::endl;
// 	if (client_data[client_fd].size() > 1000) {
// 		std::cout << "  (truncated, total size: " << client_data[client_fd].size() << " bytes)\n";
// 	}

// 	// Check if the entire request has been received
// 	std::string& data = client_data[client_fd];
// 	size_t header_end_pos = data.find("\r\n\r\n");
// 	if (header_end_pos != std::string::npos) {
// 		size_t content_length = 0;
// 		size_t content_length_pos = data.find("Content-Length:");
// 		if (content_length_pos != std::string::npos) {
// 			content_length_pos += 15; // Skip "Content-Length:"
// 			content_length = std::atoi(data.c_str() + content_length_pos);
// 			std::cout << "Content-Length: " << content_length << std::endl;
// 		}

// 		if (data.size() >= header_end_pos + 4 + content_length) {
// 			std::cout << "Entire request has been received. Total size: " << data.size() << std::endl;
// 			return true; // Entire request has been received
// 		} else {
// 			std::cout << "Request not fully received yet. Current size: " << data.size() << " Expected size: " << (header_end_pos + 4 + content_length) << std::endl;
// 		}
// 	} else {
// 		std::cout << "header_end_pos not found" << std::endl;
// 	}

// 	return false; // Request not fully received yet
// }

// int TcpServer::startServer() {
// 	std::cout << "Starting server..." << std::endl;

// 	m_socket = socket(AF_INET, SOCK_STREAM, 0);
// 	if (m_socket == -1) throw TcpServer::SocketCreationFailed();

// 	server_addr.sin_family = AF_INET;
// 	server_addr.sin_port = htons(8080);
// 	server_addr.sin_addr.s_addr = INADDR_ANY;

// 	if (bind(m_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
// 		throw TcpServer::SocketBindingFailed();

// 	if (listen(m_socket, SOMAXCONN) < 0)
// 		throw TcpServer::SocketlisteningFailed();

// 	std::cout << "Server is listening on port 8080..." << std::endl;

// 	// Set the server socket to non-blocking mode
// 	if (set_nonblocking(m_socket) == -1) throw TcpServer::SocketCreationFailed();

// 	// Initialize pollfd structures
// 	struct pollfd server_fd;
// 	server_fd.fd = m_socket;
// 	server_fd.events = POLLIN;
// 	fds.push_back(server_fd);

// 	// Initialize router
// 	Router router;
// 	router.initializeRoutes();

// 	while (true) {
// 		int ret = poll(&fds[0], fds.size(), -1); // -1 means wait indefinitely
// 		if (ret < 0) {
// 			perror("poll");
// 			break;
// 		}

// 		std::cout << "Poll returned with ret: " << ret << std::endl;

// 		for (size_t i = 0; i < fds.size(); ++i) {
// 			std::cout << "Checking fd: " << fds[i].fd << " revents: " << fds[i].revents << std::endl;
// 			if (fds[i].revents & POLLIN) {
// 				if (fds[i].fd == m_socket) {
// 					// Accept new connection
// 					socklen_t client_addr_len = sizeof(client_addr);
// 					int new_fd = accept(m_socket, (struct sockaddr*)&client_addr, &client_addr_len);
// 					if (new_fd >= 0) {
// 						std::cout << "Accepted new connection: " << new_fd << std::endl;
// 						if (set_nonblocking(new_fd) == -1) {
// 							close(new_fd);
// 							continue;
// 						}
// 						set_socket_timeout(new_fd, 300); // Set a 300-second timeout for read/write operations
// 						struct pollfd new_pollfd;
// 						new_pollfd.fd = new_fd;
// 						new_pollfd.events = POLLIN;
// 						fds.push_back(new_pollfd);
// 					} else {
// 						perror("accept");
// 					}
// 				} else {
// 					// Read data from existing client socket
// 					std::cout << "Calling readClientData for fd: " << fds[i].fd << std::endl;
// 					if (readClientData(fds[i].fd)) {
// 						// Entire request has been received
// 						std::cout << "Entire request received for fd: " << fds[i].fd << std::endl;
// 						HttpRequest httpRequest(client_data[fds[i].fd].c_str(), client_data[fds[i].fd].size());

// 						// std::cout << "printing request:" << std::endl;
// 						// httpRequest.print();
// 						HttpResponse httpResponse(httpRequest);

// 						// Process request with the router
// 						router.handleRequest(httpRequest, httpResponse);

// 						// Send response
// 						std::string httpResponseString = httpResponse.toString();
// 						if (send(fds[i].fd, httpResponseString.c_str(), httpResponseString.size(), 0) < 0) {
// 							std::cerr << "Error sending response: " << strerror(errno) << std::endl;
// 						} else {
// 							std::cout << "Response sent to client: " << fds[i].fd << std::endl;
// 						}

// 						// Clean up
// 						client_data.erase(fds[i].fd);
// 						close(fds[i].fd);
// 						fds.erase(fds.begin() + i);
// 						--i;
// 					}
// 				}
// 			}
// 		}
// 	}

// 	return 0;
// }

// const char* TcpServer::SocketCreationFailed::what() const throw() {
// 	return "Throwing exception: creating server socket";
// }

// const char* TcpServer::SocketBindingFailed::what() const throw() {
// 	return "Throwing exception: socket binding failed";
// }

// const char* TcpServer::SocketlisteningFailed::what() const throw() {
// 	return "Throwing exception: socket listening failed";
// }

// const char* TcpServer::SocketAcceptFailed::what() const throw() {
// 	return "Throwing exception: Failed to accept connection";
// }

// const char* TcpServer::SocketReadFailed::what() const throw() {
// 	return "Throwing exception: Failed to read from client";
// }