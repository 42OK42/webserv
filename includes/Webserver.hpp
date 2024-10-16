/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserver.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ecarlier <ecarlier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/16 15:06:21 by ecarlier          #+#    #+#             */
/*   Updated: 2024/10/16 18:54:43 by ecarlier         ###   ########.fr       */
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

	public:
		Webserver();
		~Webserver();

		Webserver & operator=(const Webserver &assign);
		Webserver(const std::vector<ServerConfig>& servers) : _servers(servers) {}

		void initializeServers();
};

#endif
