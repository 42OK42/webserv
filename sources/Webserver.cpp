/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserver.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ecarlier <ecarlier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/16 15:06:19 by ecarlier          #+#    #+#             */
/*   Updated: 2024/10/21 21:37:07 by ecarlier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Webserver.hpp"

Webserver::Webserver()
{
	//std::cout << "\033[33m" << "Default constructor called" << "\033[0m" << std::endl;
}

Webserver::~Webserver()
{
	//std::cout << "\033[32m" << "Destructor called" << "\033[0m" << std::endl;
}

// Webserver::Webserver(const Webserver &copy)
// {
// 	//std::cout << "\033[34m" << "Copy constructor called" << "\033[0m" << std::endl;
// }

// Webserver &Webserver::operator=(const Webserver &copy)
// {
// 	std::cout << "\033[35m" << "Copy assignment operator called" << "\033[0m" << std::endl;
// 	return *this;
// }


void Webserver::initializeServers()
{

	if (sigint_flag)
	{
		std::cerr << "SIGINT received. Not initializing servers." << std::endl;
		return;  // If SIGINT was caught, do not reinitialize the servers.
    }
	for (size_t i = 0; i < _servers.size(); ++i)
	{
		ServerConfig& server = _servers[i]; // Get the server configuration

		try
		{
			server.setupServerSocket(); // Initialize the server socket for each configuration
			std::cout << "Server initialized on " << server.getHost() << ":" << server.getPort() << std::endl;
		} catch (const std::exception& e) {
			std::cerr << "Error initializing server: " << e.what() << std::endl;
		}
	}
}




/*			Exceptions 			*/


