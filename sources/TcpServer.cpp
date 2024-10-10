/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TcpServer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: okrahl <okrahl@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/30 15:38:05 by ecarlier          #+#    #+#             */
/*   Updated: 2024/10/10 16:47:21 by okrahl           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/TcpServer.hpp"
#include "Router.hpp"
#include "HttpResponse.hpp"
#include "HttpRequest.hpp"

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

bool TcpServer::isRequestComplete(const std::vector<char>& buffer, int total_bytes_read) {
	std::string headers(buffer.begin(), buffer.begin() + total_bytes_read);
	size_t pos = headers.find("\r\n\r\n");
	if (pos != std::string::npos) {
		return true;
	}
	return false;
}

bool TcpServer::isBodyComplete(const std::vector<char>& buffer, int total_bytes_read) {
	std::string headers(buffer.begin(), buffer.begin() + total_bytes_read);
	size_t content_length_pos = headers.find("Content-Length: ");
	if (content_length_pos != std::string::npos) {
		content_length_pos += 16;
		size_t end_pos = headers.find("\r\n", content_length_pos);
		std::string content_length_str = headers.substr(content_length_pos, end_pos - content_length_pos);
		
		std::stringstream ss(content_length_str);
		int content_length;
		if (!(ss >> content_length)) {
			std::cerr << "Error parsing Content-Length: " << content_length_str << std::endl;
			return false;
		}

		size_t pos = headers.find("\r\n\r\n");
		if (total_bytes_read >= static_cast<int>(pos + 4 + content_length)) {
			return true;
		}
	}
	return false;
}

int TcpServer::readRequest(int client_socket, std::vector<char>& buffer) {
	int total_bytes_read = 0;

	while (true) {
		int bytes_read = recv(client_socket, &buffer[total_bytes_read], buffer.size() - total_bytes_read, 0);
		if (bytes_read < 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				// Keine Daten verfügbar, einfach weitermachen
				continue;
			} else {
				throw SocketReadFailed();
			}
		}
		if (bytes_read == 0) break; // Verbindung geschlossen
		total_bytes_read += bytes_read;

		// If the buffer is full, increase its size
		if (total_bytes_read == static_cast<int>(buffer.size())) {
			buffer.resize(buffer.size() + 8192); // Increase buffer size
		}

		// Check if the headers are complete
		if (isRequestComplete(buffer, total_bytes_read)) {
			std::string headers(buffer.begin(), buffer.begin() + total_bytes_read);
			if (headers.find("GET ") != std::string::npos || headers.find("DELETE ") != std::string::npos) {
				std::cout << "GET or DELETE request detected, no body expected." << std::endl;
				return total_bytes_read;
			}

			// For other methods like POST, which have a body
			if (isBodyComplete(buffer, total_bytes_read)) {
				break; // Entire request read
			}
		}
	}

	return total_bytes_read;
}

int TcpServer::startServer()
{
	std::cout << "Starting server..." << std::endl;

	m_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (m_socket == -1) throw TcpServer::SocketCreationFailed();

	int opt = 1;
	if (setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
		throw TcpServer::SocketBindingFailed();
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(8080);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(m_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
		throw TcpServer::SocketBindingFailed();

	if (listen(m_socket, SOMAXCONN) < 0)
		throw TcpServer::SocketlisteningFailed();

	std::cout << "Server is listening on port 8080..." << std::endl;

	// Initialize pollfd structures
	fds[0].fd = m_socket;
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
				if (fds[i].fd == m_socket) {
					// Accept new connection
					socklen_t client_addr_len = sizeof(client_addr);
					client_socket = accept(m_socket, (struct sockaddr*)&client_addr, &client_addr_len);
					if (client_socket >= 0) {
						fcntl(client_socket, F_SETFL, O_NONBLOCK);
						fds[nfds].fd = client_socket;
						fds[nfds].events = POLLIN;
						nfds++;
					}
				} else {
					// Read data from a client
					std::vector<char> buffer(8192);
					int total_bytes_read = readRequest(fds[i].fd, buffer);

					std::cout << "Received..." << std::endl;

					HttpRequest httpRequest(&buffer[0], total_bytes_read);
					HttpResponse httpResponse(httpRequest);

					// Process request with the router
					router.handleRequest(httpRequest, httpResponse);

					// Send response
					std::string httpResponseString = httpResponse.toString();
					send(fds[i].fd, httpResponseString.c_str(), httpResponseString.size(), 0);

					// Close connection
					close(fds[i].fd);
					fds[i] = fds[nfds - 1];
					nfds--;
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

const char* TcpServer::SocketlisteningFailed::what() const throw () {
	return "Throwing exception: socket listening failed";
}

const char* TcpServer::SocketAcceptFailed::what() const throw () {
	return "Throwing exception: Failed to accept connection";
}

const char* TcpServer::SocketReadFailed::what() const throw () {
	return "Throwing exception: Failed to read from client";
}