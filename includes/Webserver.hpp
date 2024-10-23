/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserver.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: okrahl <okrahl@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/16 15:06:21 by ecarlier          #+#    #+#             */
/*   Updated: 2024/10/23 19:43:29 by okrahl           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include "ServerConfig.hpp"
#include <vector>
#include <sys/poll.h>
#include <unistd.h>
#include <iostream>

extern bool sigint_flag;

class Webserver
{
	private:
		std::vector<ServerConfig> _servers;
		std::vector<struct pollfd> fds;

		Webserver(const Webserver &copy); // Private copy constructor to prevent copying

		bool isServerSocket(int fd);
		void runEventLoop();
		void handleNewConnection(int server_socket);
		void handleClientData(size_t index);
		void setNonBlocking(int sockfd);
		void setSocketTimeout(int sockfd, int timeout_seconds);

	public:
		Webserver();
		~Webserver();
		Webserver(const std::vector<ServerConfig>& servers);

		void initializeServers();
};

#endif // WEBSERV_HPP