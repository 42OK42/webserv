/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ecarlier <ecarlier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/08 12:47:51 by ecarlier          #+#    #+#             */
/*   Updated: 2024/10/16 20:14:50 by ecarlier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include "Location.hpp"
#include <map>
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
#include <unistd.h>
#include <cerrno>
#include <fcntl.h>
#include <csignal>
#include <cstdlib>
#include <netdb.h>
#include <sstream>
#include <string.h>
#include <vector>


class ServerConfig
{
	private:

		int								_port;
		std::string						_host;
		std::vector<std::string>		_serverNames;
		std::string						_root; // Spécifie le répertoire racine à partir duquel le serveur sert les fichiers.
		std::map<int, std::string>		_errorPages;
		size_t 							_clientMaxBodySize;

		std::map <std::string, Location>	_locations;

		//std::vector<RouteConfig> routes;  // Routes for the server


		/* CGI */
		bool							_cgiEnabled;
		std::string						_cgiExtension;
		std::string						_cgiBin;

		/**/
		int	m_socket; //return a socket decriptor
		struct sockaddr_in	server_addr;
		struct sockaddr_in client_addr; //contains Ip adress and client port
		int client_socket;



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


		void addLocation(const std::string& path, const Location& location);
		const std::map<std::string, Location>& getLocations() const;


		void setupServerSocket();
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
		class LocationNotFound : public std::exception {
			public:
				virtual const char* what() const throw();
		};

};

std::ostream& operator<<(std::ostream& os, const ServerConfig& server);


#endif // ServerConfig_HPP
