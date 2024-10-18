/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: okrahl <okrahl@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/08 12:47:51 by ecarlier          #+#    #+#             */
/*   Updated: 2024/10/18 19:29:23 by okrahl           ###   ########.fr       */
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
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/poll.h>
#include <sys/ioctl.h>
#include <exception>
#include <unistd.h>
#include <cerrno>
#include <fcntl.h>
#include <csignal>
#include <netdb.h>
#include <sstream>
#include <vector>

class ServerConfig
{
	private:
		std::vector<int>                _port;  // Port number to listen on (e.g., 8080)
		std::vector<std::string>        _host;  // Host (e.g., "127.0.0.1" or "localhost")
		std::vector<std::string>        _serverNames;  // Server aliases (e.g., "example.com")
		std::string                     _root; // Spécifie le répertoire racine à partir duquel le serveur sert les fichiers.
		std::map<int, std::string>      _errorPages;  // Default error pages (e.g., 404 -> "/404.html")
		size_t                          _clientMaxBodySize;  // Maximum allowed size for client request body
		std::map<std::string, Location> _locations;
		std::string 					_path_to_html_files;

		/* CGI */
		bool                            _cgiEnabled;
		std::string                     _cgiExtension;
		std::string                     _cgiBin;

		/**/
		int m_socket; // Socket descriptor
		struct sockaddr_in server_addr;
		struct sockaddr_in client_addr; // Contains IP address and client port
		int client_socket;
		std::vector<struct pollfd> fds; // Poll file descriptors
		std::map<int, std::string> client_data; // Client data buffer

	public:
		~ServerConfig();

		/*          Setters & Getters            */
		void            setPort(const std::vector<std::string>& tokens);
		void            setHost(const std::vector<std::string>& tokens);
		void            setServerName(const std::vector<std::string>& tokens);
		void            setRoot(const std::vector<std::string>& tokens);
		void            setErrorPage(const std::vector<std::string>& tokens);
		void            setClientMaxBodySize(size_t token);
		void            setCgiEnabled(const std::vector<std::string>& tokens);
		void            setCgiExtension(const std::vector<std::string>& tokens);
		void            setCgiBin(const std::vector<std::string>& tokens);

		int                         getListen(size_t idx) const;
		size_t                      getNbOfPorts() const;
		std::vector<int>            getListen(void) const;
		std::string                 getHost(size_t idx) const;
		std::vector<std::string>    getHost(void) const;
		std::string                 getServerName(size_t idx) const;
		std::vector<std::string>    getServerName(void) const;
		std::string                 getRoot(void) const;
		int                         getClientMaxBodySize(void) const;
		bool                        isCgiEnabled(void) const;
		std::string                 getCgiExtension(void) const;
		std::string                 getCgiBin(void) const;

		void addErrorPage(int code, const std::string& page);
		const std::map<int, std::string>& getErrorPages() const;

		void addLocation(const std::string& path, const Location& location);
		const std::map<std::string, Location>& getLocations() const;

		void print() const;

		int startServer();
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
};

std::ostream& operator<<(std::ostream& os, const ServerConfig& server);

#endif