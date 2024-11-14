/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ecarlier <ecarlier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/08 12:47:51 by ecarlier          #+#    #+#             */
/*   Updated: 2024/11/14 21:29:51 by ecarlier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include "CommonHeader.hpp"

extern bool sigint_flag;

class Location;

class ServerConfig
{
	private:
		static const std::string 			UPLOAD_DIR;

		int									_port;
		std::string							_host;
		std::string							_serverName;
		std::string							_root;
		std::map<int, std::string>			_errorPages;
		size_t 								_clientMaxBodySize;
		std::map <std::string, Location>	_locations;

		/* CGI */
		bool								_cgiEnabled;
		std::string							_cgiExtension;
		std::string							_cgiBin;

		int									m_socket;
		struct sockaddr_in					server_addr;
		std::vector<struct pollfd>			fds;
		std::map<int, std::string>			client_data;

	public:
		ServerConfig();
		~ServerConfig();

		/* Setters */
		void	setHost(const std::string& host);
		void	setPort(int port);
		void	setServerName(const std::vector<std::string>& tokens);
		void	setRoot(const std::vector<std::string>& tokens);
		void	setErrorPage(const std::vector<std::string>& tokens);
		void	setClientMaxBodySize(size_t token);
		void	setCgiEnabled(const std::vector<std::string>& tokens);
		void	setCgiExtension(const std::vector<std::string>& tokens);
		void	setCgiBin(const std::vector<std::string>& tokens);

		/* Getters */
		int											getSocket() const;
		std::string&								getClientData(int client_fd);
		std::string									getHost() const;
		int											getPort() const;
		std::string									getServerName(void) const;
		std::string									getRoot(void) const;
		int											getClientMaxBodySize(void) const;
		bool										isCgiEnabled(void) const;
		std::string									getCgiExtension(void) const;
		std::string									getCgiBin(void) const;
		const std::map<int, std::string>&			getErrorPages() const;
		const std::map<std::string, Location>&		getLocations() const;

		std::string 						getExecutablePath();
		std::string 						getErrorFilePath(int errorCode);

		void			eraseClientData(int client_fd);
		void			addErrorPage(int code, const std::string& page);
		void			checkErrorPage();
		void			addLocation(const std::string& path, const Location& location);
		void			set_socket_timeout(int sockfd, int timeout_seconds);
		bool			isBodySizeAllowed(size_t contentLength) const;
		bool			readClientData(int client_fd);
		int				setupServerSocket();
		std::string		readFile(const std::string& filepath);
		Location		findLocation(std::string locationPath);


		static std::string getUploadDir() { return "/home/ecarlier/sgoinfre/uploads_webserv/"; }

		class SocketCreationFailed : public std::exception {
			public:
				virtual const char* what() const throw();
		};
		class SocketBindingFailed : public std::exception {
			public:
				virtual const char* what() const throw();
		};
		class SocketListeningFailed : public std::exception {
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

#endif // SERVERCONFIG_HPP
