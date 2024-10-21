/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ecarlier <ecarlier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/17 12:07:58 by ecarlier          #+#    #+#             */
/*   Updated: 2024/10/17 12:16:25 by ecarlier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "../includes/Client.hpp"

Client::Client(ServerConfig &server, int fd) : _server(server)
{
	_fd = fd;
	_req = NULL;
	_resp = NULL;
}

// Client::Client()
// {
// 	//std::cout << "\033[33m" << "Default constructor called" << "\033[0m" << std::endl;
// }

Client::~Client()
{
	//close(_fd);
	//std::cout << "\033[32m" << "Destructor called" << "\033[0m" << std::endl;
}

// Client::Client(const Client &copy)
// {
// 	//std::cout << "\033[34m" << "Copy constructor called" << "\033[0m" << std::endl;
// }

// Client &Client::operator=(const Client &copy)
// {
// 	//std::cout << "\033[35m" << "Copy assignment operator called" << "\033[0m" << std::endl;
// 	return *this;
// }


