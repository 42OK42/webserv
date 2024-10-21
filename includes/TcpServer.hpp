/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TcpServer.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ecarlier <ecarlier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/30 15:38:08 by ecarlier          #+#    #+#             */
/*   Updated: 2024/10/18 20:36:40 by ecarlier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TcpServer_HPP
#define TcpServer_HPP

#include <fstream>
#include <string>
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/poll.h>
#include <sys/ioctl.h>
#include <exception>
#include <unistd.h>
#include <fcntl.h>
#include <csignal>
#include <cstdlib>
#include <netdb.h>
#include <sstream>
#include <string.h>
#include <vector>

class TcpServer
{
	private:
		int	m_socket; //return a socket decriptor
		struct sockaddr_in	server_addr;
		struct sockaddr_in client_addr; //contains Ip adress and client port
		int client_socket;

	public:
		TcpServer();
		~TcpServer();

		int startServer();
		std::string readFile(const std::string& filepath);

		class SocketCreationFailed : public std::exception {
			public:
				virtual const char* what() const throw();
		};
		class SocketBindingFailed : public std::exception {
			public:
				virtual const char* what() const throw();
		};
		class SocketlisteningFailed : public std::exception {
			public:
				virtual const char* what() const throw();
		};
		class SocketAcceptFailed : public std::exception {
			public:
				virtual const char* what() const throw();
		};
		class SocketReadFailed : public std::exception {
			public:
				virtual const char* what() const throw();
		};


};

std::ostream &operator<<(std::ostream &o, TcpServer const &i);

#endif // TcpServer_HPP
