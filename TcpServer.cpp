/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TcpServer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ecarlier <ecarlier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/30 15:38:05 by ecarlier          #+#    #+#             */
/*   Updated: 2024/09/30 16:32:22 by ecarlier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "TcpServer.hpp"

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

		return 0;
}

/* --------------- Expections handling --------------- */

const char* TcpServer::SocketCreationFailed::what() const throw ()
{
	return ("Trowing exception : creating server socket");
}

