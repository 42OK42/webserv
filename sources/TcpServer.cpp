/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TcpServer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ecarlier <ecarlier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/30 15:38:05 by ecarlier          #+#    #+#             */
/*   Updated: 2024/09/30 18:12:56 by ecarlier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/TcpServer.hpp"

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
	m_socket = socket(AF_INET, SOCK_STREAM, 0);

	if (m_socket == -1)
		throw TcpServer::SocketCreationFailed();
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(8080);  // Port 8080
	server_addr.sin_addr.s_addr = INADDR_ANY;  // Bind to any available interface (localhost)
	if (bind(m_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    	throw TcpServer::SocketCreationFailed();

	return 0;
}

/* --------------- Expections handling --------------- */

const char* TcpServer::SocketCreationFailed::what() const throw ()
{
	return ("Trowing exception : creating server socket");
}

