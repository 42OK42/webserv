/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TcpServer.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: okrahl <okrahl@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/30 15:38:08 by ecarlier          #+#    #+#             */
/*   Updated: 2024/10/10 15:22:40 by okrahl           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TcpServer_HPP
#define TcpServer_HPP

#include <fstream> 
#include <string>
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/poll.h>
#include <exception>
#include <unistd.h>
#include <cerrno>
#include <fcntl.h>
#include <csignal>
#include <cstdlib>
#include <netdb.h>
#include <sstream>
#include <vector>

class TcpServer
{
	private:
		int	m_socket;
		struct sockaddr_in	server_addr;
		struct sockaddr_in client_addr;
		int client_socket;

		struct pollfd fds[200];
		int nfds;

	public:
		TcpServer();
		~TcpServer();

		int startServer();
		std::string readFile(const std::string& filepath);
		bool isRequestComplete(const std::vector<char>& buffer, int total_bytes_read);
		bool isBodyComplete(const std::vector<char>& buffer, int total_bytes_read);
		int readRequest(int client_socket, std::vector<char>& buffer);

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

#endif
