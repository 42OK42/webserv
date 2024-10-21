/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserver.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ecarlier <ecarlier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/16 15:06:21 by ecarlier          #+#    #+#             */
/*   Updated: 2024/10/21 20:08:09 by ecarlier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
# define WEBSERV_HPP

#include "ServerConfig.hpp"
#include <vector>
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
