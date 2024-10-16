/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserver.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ecarlier <ecarlier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/16 15:06:21 by ecarlier          #+#    #+#             */
/*   Updated: 2024/10/16 15:12:08 by ecarlier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
# define WEBSERV_HPP

#include "ServerConfig.hpp"
#include <vector>

class Webserver
{
	public:
		Webserver();
		~Webserver();
		Webserver & operator=(const Webserver &assign);

	private:
		void							addServer(ServerConfig s);
		std::vector<ServerConfig>::iterator	findServer(int fd);

		Webserver(const Webserver &copy);
		std::vector<ServerConfig>	_servers;
};

#endif
