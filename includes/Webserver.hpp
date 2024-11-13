/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserver.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ecarlier <ecarlier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/16 15:06:21 by ecarlier          #+#    #+#             */
/*   Updated: 2024/11/13 18:38:09 by ecarlier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include "ServerConfig.hpp"
#include "HttpResponse.hpp"
#include "HttpRequest.hpp"
#include "Router.hpp"
#include <vector>
#include <sys/poll.h>
#include <unistd.h>
#include <iostream>
#include <map>

extern bool sigint_flag;

class Webserver
{
	private:
		Webserver(const Webserver &copy); // Private copy constructor to prevent copying
		std::vector<ServerConfig>		_servers;
		std::vector<struct pollfd>		fds;
		std::map<int, int>				client_to_server;



		bool	isServerSocket(int fd);
		void	handleNewConnection(int server_socket);
		void	handleClientData(size_t index);
		void	setNonBlocking(int sockfd);
		void	setSocketTimeout(int sockfd, int timeout_seconds);
		void	closeConnection(size_t index);

		ServerConfig*	findMatchingServer(const std::string& host, int port);
		void			processRequest(HttpRequest& httpRequest, ServerConfig* server, int client_fd);

	public:
		Webserver();
		~Webserver();
		Webserver(const std::vector<ServerConfig>& servers);

		void	runEventLoop();
		void	initializeServers();
};

#endif
