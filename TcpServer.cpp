/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TcpServer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ecarlier <ecarlier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/30 15:38:05 by ecarlier          #+#    #+#             */
/*   Updated: 2024/09/30 15:39:33 by ecarlier         ###   ########.fr       */
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

// TcpServer::TcpServer(const TcpServer &copy)
// {
// 	std::cout << "\033[34m" << "Copy constructor called" << "\033[0m" << std::endl;
// }

// TcpServer &TcpServer::operator=(const TcpServer &copy)
// {
// 	std::cout << "\033[35m" << "Copy assignment operator called" << "\033[0m" << std::endl;
// 	return *this;
// }


