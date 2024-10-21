/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ecarlier <ecarlier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/17 12:07:57 by ecarlier          #+#    #+#             */
/*   Updated: 2024/10/17 12:16:01 by ecarlier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

class HttpRequest;
class HttpResponse;
class ServerConfig;

#include <string>
#include <iostream>
#include <fstream>

class Client
{
	public:
		Client(ServerConfig &server, int fd);
		~Client();


		int		receive();
		int		send();


	private:
		ServerConfig&		_server;

		int					_fd;
		HttpRequest*		_req;
		HttpResponse*		_resp;
		//std::string			_sendbuffer;
};

#endif
