/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserver.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: okrahl <okrahl@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/16 15:06:21 by ecarlier          #+#    #+#             */
/*   Updated: 2024/11/19 21:18:12 by okrahl           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include "ServerConfig.hpp"
#include "HttpResponse.hpp"
#include "HttpRequest.hpp"
#include "Router.hpp"
#include "Common.hpp"

class Webserver
{
	public:
		static const int SOCKET_TIMEOUT_SECONDS = 10;
		static const int READ_TIMEOUT_SECONDS = 10;

		Webserver();
		~Webserver();
		Webserver(const std::vector<ServerConfig>& servers);

		void	runEventLoop();
		void	initializeServers();
		void	cleanup();

	private:
		Webserver(const Webserver &copy);

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
};

#endif
