/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TcpServer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: okrahl <okrahl@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/30 15:38:05 by ecarlier          #+#    #+#             */
/*   Updated: 2024/10/02 18:00:59 by okrahl           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/TcpServer.hpp"
#include "Router.hpp"
#include "HttpResponse.hpp"
#include "HttpRequest.hpp"

TcpServer::TcpServer()
{
	std::cout << "\033[33m" << "Default constructor called" << "\033[0m" << std::endl;
}

TcpServer::~TcpServer()
{
	std::cout << "\033[32m" << "Destructor called" << "\033[0m" << std::endl;
}

// void TcpServer::closeServer()
// {
// 		close(m_socket);
// 		exit(0);
// }

/*
AF_INET -> IPv4
SOCK_STREAM -> TCP
0 -> default protocol -> TCP
*/
int TcpServer::startServer()
{
	std::cout << "Starting server..." << std::endl;

	m_socket = socket(AF_INET, SOCK_STREAM, 0);

	if (m_socket == -1)
		throw TcpServer::SocketCreationFailed();
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(8080);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(m_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
		throw TcpServer::SocketBindingFailed();

	if (listen(m_socket, SOMAXCONN) < 0)
		throw TcpServer::SocketlisteningFailed();

	std::cout << "Server is listening on port 8080..." << std::endl;

	socklen_t client_addr_len = sizeof(client_addr);

	// Router initialisieren
	Router router;
	initializeRoutes(router);

	while (true)
	{
		client_socket = accept(m_socket, (struct sockaddr*)&client_addr, &client_addr_len);

		if (client_socket < 0)
			throw SocketAcceptFailed();

		char buffer[1024] = {0};
		int bytes_read = recv(client_socket, buffer, sizeof(buffer), 0);
		if (bytes_read < 0)
			throw SocketReadFailed();

		std::cout << "Received: " << buffer << std::endl;

		HttpRequest httpRequest(buffer, bytes_read);
		HttpResponse httpResponse(httpRequest);

		// Anfrage mit dem Router verarbeiten
		router.handleRequest(httpRequest, httpResponse);

		// Antwort senden
		std::string httpResponseString = httpResponse.toString();
		send(client_socket, httpResponseString.c_str(), httpResponseString.size(), 0);

		close(client_socket);
	}

	return 0;
}

/* --------------- Expections handling --------------- */

const char* TcpServer::SocketCreationFailed::what() const throw ()
{
	return ("Trowing exception : creating server socket");
}

const char* TcpServer::SocketBindingFailed::what() const throw ()
{
	return ("Trowing exception : socket binding failed");
}

const char* TcpServer::SocketlisteningFailed::what() const throw ()
{
	return ("Trowing exception : socket listening failed");
}

const char* TcpServer::SocketAcceptFailed::what() const throw ()
{
	return ("Trowing exception : Failed to accept connection");
}

const char* TcpServer::SocketReadFailed::what() const throw ()
{
	return ("Trowing exception : Failed to read from client");
}
