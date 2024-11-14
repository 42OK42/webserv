/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ecarlier <ecarlier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/08 15:49:54 by ecarlier          #+#    #+#             */
/*   Updated: 2024/11/14 21:30:27 by ecarlier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATION_HPP
#define LOCATION_HPP

#include "CommonHeader.hpp"

#include "Parser.hpp"

class ServerConfig;

class Location
{
	private:
		std::string					_path;
		std::string					_root;
		std::string					_index;
		bool 						_autoIndex;
		std::vector<std::string>	_Methods;
		std::string					_cgi_extension;
		std::string					_cgi_path;
		std::string					_redirectTo;
		bool _cgiEnabled;
		std::string _cgiExtension;
		std::string _cgiBin;

	public:
		Location();
		~Location();

		bool isValidMethod(const std::string& token);
		bool isValidAutoIndex(const std::string& token);

		/* Getters */
		std::string getPath() const;
		std::string get_redirectTo() const;
		std::string getRoot() const;
		std::string getIndex() const;
		bool getAutoIndex() const;
		std::vector<std::string> getMethods() const;
		bool isCgiEnabled() const { return _cgiEnabled; }
		std::string getCgiExtension() const { return _cgiExtension; }
		std::string getCgiBin() const { return _cgiBin; }

		/* Setters */
		void setExtension(const std::string& token);
		void setCgiPath(const std::string& token);
		void setPath(const std::string& token);
		void setRoot(const std::string& token);
		void setIndex(const std::string& token);
		void set_redirectTo(const std::string& token);
		void setAutoIndex(const std::string& token);
		void setMethods(const std::vector<std::string>& tokens);
		void setCgiEnabled(bool enabled) { _cgiEnabled = enabled; }
		void setCgiExtension(const std::string& ext) { _cgiExtension = ext; }
		void setCgiBin(const std::string& bin) { _cgiBin = bin; }

		bool isMethodAllowed(const std::string& method) const;

		class InvalidAutoIndexException: public std::exception {
			public:
				virtual const char* what() const throw();
		};

		class InvalidMethodException : public std::exception {
			public:
				virtual const char* what() const throw();
		};
};

std::ostream& operator<<(std::ostream& os, const Location& location);

#endif // LOCATION_HPP
