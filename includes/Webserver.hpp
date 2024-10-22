/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserver.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: okrahl <okrahl@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/16 15:06:21 by ecarlier          #+#    #+#             */
/*   Updated: 2024/10/22 15:38:57 by okrahl           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
# define WEBSERV_HPP

#include "ServerConfig.hpp"
#include <vector>
#include <unistd.h>
#include <sys/wait.h>

extern bool sigint_flag;

class Webserver
{

	private:


		Webserver(const Webserver &copy);
		std::vector<ServerConfig>	_servers;

	public:
		Webserver();
		~Webserver();

		//Webserver & operator=(const Webserver &assign);
		Webserver(const std::vector<ServerConfig>& servers) : _servers(servers) {}

		void initializeServers();
};

#endif
