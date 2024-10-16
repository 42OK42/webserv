/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TcpServer.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: okrahl <okrahl@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/30 15:38:08 by ecarlier          #+#    #+#             */
/*   Updated: 2024/10/16 17:19:19 by okrahl           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TCPSERVER_HPP
#define TCPSERVER_HPP

#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <stdexcept>

class TcpServer {
private:
	int m_socket;
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	std::vector<struct pollfd> fds;
	std::map<int, std::string> client_data;

	bool readClientData(int client_fd);
	void set_socket_timeout(int sockfd, int timeout_seconds);

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

#endif // TCPSERVER_HPP