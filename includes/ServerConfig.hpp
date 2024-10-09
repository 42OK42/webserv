/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ecarlier <ecarlier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/08 12:47:51 by ecarlier          #+#    #+#             */
/*   Updated: 2024/10/09 16:34:06 by ecarlier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include "Location.hpp"
#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <cstdlib>

class ServerConfig
{
	private:
		std::vector<int> 				_port;  // Port number to listen on (e.g., 8080)
		// get those from "listen" in config file
		std::vector<std::string>		_host;  // Host (e.g., "127.0.0.1" or "localhost")
		std::vector<std::string>		_serverNames;  // Server aliases (e.g., "example.com")
		std::string						_root; // Spécifie le répertoire racine à partir duquel le serveur sert les fichiers.
		std::map<int, std::string>		_errorPages;  // Default error pages (e.g., 404 -> "/404.html")
		size_t 							_clientMaxBodySize;  // Maximum allowed size for client request body
		//std::vector<RouteConfig> routes;  // Routes for the server
		std::map <std::string, Location>	_locations;

		/* CGI */
		bool							_cgiEnabled;
		std::string						_cgiExtension;
		std::string						_cgiBin;

		/* Location */

		//create a class location?

	public:
		//ServerConfig( const Configuration& config ); //from parsing config file
		~ServerConfig();

		/*			Setters & Getters			*/
		void			setPort( const std::vector<std::string>& tokens );
		void			setHost( const std::vector<std::string>& tokens );
		void			setServerName( const std::vector<std::string>& tokens );
		void			setRoot( const std::vector<std::string>& tokens );
		void			setErrorPage( const std::vector<std::string>& tokens );
		void 			setClientMaxBodySize(size_t token);
		void			setCgiEnabled( const std::vector<std::string>& tokens );
		void			setCgiExtension( const std::vector<std::string>& tokens );
		void			setCgiBin( const std::vector<std::string>& tokens );


		int							getListen( size_t idx ) const;
		size_t						getNbOfPorts() const;
		std::vector<int>			getListen( void ) const;
		std::string					getHost( size_t idx ) const;
		std::vector<std::string>	getHost( void ) const;
		std::string					getServerName( size_t idx ) const;
		std::vector<std::string>	getServerName( void ) const;
		std::string					getRoot( void ) const;
		std::string					getErrorPage( int code ) const;
		int							getClientMaxBodySize( void ) const;
		bool						isCgiEnabled( void ) const;
		std::string					getCgiExtension( void ) const;
		std::string					getCgiBin( void ) const;
		//Location*		getLocation( const string& path_or_flag );
		std::map<std::string, Location>& getLocations();
		void addLocation(const std::string& path, const Location& location);


};



#endif // ServerConfig_HPP
