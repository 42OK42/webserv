/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: okrahl <okrahl@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/08 12:47:51 by ecarlier          #+#    #+#             */
/*   Updated: 2024/10/22 16:48:11 by okrahl           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include "Location.hpp"
#include <map>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstdlib>
#include <fstream>
#include <string>
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
#include <csignal>
#include <cstdlib>
#include <netdb.h>
#include <sstream>
#include <string.h>
#include <vector>
#include <limits.h>
extern bool sigint_flag;

class ServerConfig
{
	private:

		int								_port;
		std::string						_host;
		std::vector<std::string>		_serverNames;
		std::string						_root;
		std::map<int, std::string>		_errorPages;
		size_t 							_clientMaxBodySize;
		std::map <std::string, Location>	_locations;

		/* CGI */
		bool                            _cgiEnabled;
		std::string                     _cgiExtension;
		std::string                     _cgiBin;

		/**/
		int	m_socket; //return a socket decriptor
		struct sockaddr_in	server_addr;
		struct sockaddr_in client_addr; //contains Ip adress and client port
		// int client_socket;
		int m_socket; // Socket descriptor
		struct sockaddr_in server_addr;
		struct sockaddr_in client_addr; // Contains IP address and client port
		int client_socket;
		std::vector<struct pollfd> fds; // Poll file descriptors
		std::map<int, std::string> client_data; // Client data buffer



	public:
		//ServerConfig( const Configuration& config ); //from parsing config file
		ServerConfig();
		~ServerConfig();


		Location findLocation(std::string locationPath);

		/*			Setters & Getters			*/
		void setHost(const std::string& host);
		void setPort(int port);

		std::string getHost() const;
		int getPort() const;


		void			setServerName( const std::vector<std::string>& tokens );
		void			setRoot( const std::vector<std::string>& tokens );
		void			setErrorPage( const std::vector<std::string>& tokens );
		void 			setClientMaxBodySize(size_t token);
		void			setCgiEnabled( const std::vector<std::string>& tokens );
		void			setCgiExtension( const std::vector<std::string>& tokens );
		void			setCgiBin( const std::vector<std::string>& tokens );

		std::string					getServerName( size_t idx ) const;
		std::vector<std::string>	getServerName( void ) const;
		std::string					getRoot( void ) const;
		int							getClientMaxBodySize( void ) const;
		bool						isCgiEnabled( void ) const;
		std::string					getCgiExtension( void ) const;
		std::string					getCgiBin( void ) const;



		void addErrorPage(int code, const std::string& page);
		const std::map<int, std::string>& getErrorPages() const;
		void  checkErrorPage();
		std::string getExecutablePath();
		std::string getErrorFilePath(int errorCode);

		void addLocation(const std::string& path, const Location& location);
		const std::map<std::string, Location>& getLocations() const;


		void setupServerSocket();
		std::string readFile(const std::string& filepath);
		void set_socket_timeout(int sockfd, int timeout_seconds);
		bool readClientData(int client_fd);


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
		class LocationNotFound : public std::exception {
			public:
				virtual const char* what() const throw();
		};

};

std::ostream& operator<<(std::ostream& os, const ServerConfig& server);


#endif // ServerConfig_HPP
