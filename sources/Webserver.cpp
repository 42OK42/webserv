/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserver.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: okrahl <okrahl@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/16 15:06:19 by ecarlier          #+#    #+#             */
/*   Updated: 2024/10/22 15:40:37 by okrahl           ###   ########.fr       */
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
		pid_t pid = fork();
		if (pid < 0) {
			std::cerr << "Fork failed (Olli)" << std::endl;
			exit(EXIT_FAILURE);
		}
		else if (pid == 0) {
			// Child process
			ServerConfig& server = _servers[i];
			try
			{
				server.setupServerSocket(); // This will run the event loop
				std::cout << "Server initialized on " << server.getHost() << ":" << server.getPort() << " (Olli)" << std::endl;
			}
			catch (const std::exception& e) {
				std::cerr << "Error initializing server: " << e.what() << " (Olli)" << std::endl;
			}
			exit(0); // Exit child process after server stops
		}
	}

	// Parent process waits for all child processes to finish
	for (size_t i = 0; i < _servers.size(); ++i) {
		wait(NULL);
	}
}




/*			Exceptions 			*/


