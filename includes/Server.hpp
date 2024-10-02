/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: okrahl <okrahl@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/02 15:09:06 by okrahl            #+#    #+#             */
/*   Updated: 2024/10/02 15:09:07 by okrahl           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include "Data.hpp"
#include <string>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>

class Server
{
	public:

		Server(const ServerConf& config);
		~Server();

		bool init();
		void run();

	private:

		ServerConf config;
		int server_fd;

		bool createSocket();
		bool bindSocket();
		bool listenOnSocket();
		void acceptConnections();
};

#endif