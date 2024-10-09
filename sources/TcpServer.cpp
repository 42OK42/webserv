/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TcpServer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: okrahl <okrahl@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/30 15:38:05 by ecarlier          #+#    #+#             */
/*   Updated: 2024/10/09 17:37:49 by okrahl           ###   ########.fr       */
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

int TcpServer::startServer()
{
	std::cout << "Starting server..." << std::endl;

	m_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (m_socket == -1) throw TcpServer::SocketCreationFailed();

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(8080);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(m_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
		throw TcpServer::SocketBindingFailed();

	if (listen(m_socket, SOMAXCONN) < 0)
		throw TcpServer::SocketlisteningFailed();

	std::cout << "Server is listening on port 8080..." << std::endl;

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