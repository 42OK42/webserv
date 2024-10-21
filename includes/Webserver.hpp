/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserver.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ecarlier <ecarlier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/16 15:06:21 by ecarlier          #+#    #+#             */
/*   Updated: 2024/10/16 20:10:11 by ecarlier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
# define WEBSERV_HPP

#include "ServerConfig.hpp"
#include <vector>

class Webserver
{

	private:

		// void							addServer(ServerConfig s);
		// std::vector<ServerConfig>::iterator	findServer(int fd);

		Webserver(const Webserver &copy);
		std::vector<ServerConfig>	_servers;
		// int					_nfds;
		// struct pollfd		_fds[4096]; //

	public:
		Webserver();
		~Webserver();

		Webserver & operator=(const Webserver &assign);
		Webserver(const std::vector<ServerConfig>& servers) : _servers(servers) {}

		void initializeServers();
};

#endif
