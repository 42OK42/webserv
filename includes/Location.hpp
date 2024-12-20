/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ecarlier <ecarlier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/08 15:49:54 by ecarlier          #+#    #+#             */
/*   Updated: 2024/11/17 03:35:56 by ecarlier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATION_HPP
#define LOCATION_HPP


#include "Common.hpp"

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
		size_t 						_clientMaxBodySize;


		bool						_cgiEnabled;
		std::string 				_cgiExtension;
		std::string 				_cgiBin;

	public:
		Location();
		~Location();

		bool isValidMethod(const std::string& token);
		bool isValidAutoIndex(const std::string& token);

		/* Getters */
		int							getClientMaxBodySize(void) const;
		std::string					getPath() const;
		std::string					get_redirectTo() const;
		std::string					getRoot() const;
		std::string					getIndex() const;
		std::string					getCgiExtension() const { return _cgiExtension; }
		std::string					getCgiBin() const { return _cgiBin; }
		bool						getAutoIndex() const;
		bool						isCgiEnabled() const { return _cgiEnabled; }
		std::vector<std::string>	getMethods() const;

		/* Setters */

		void	setClientMaxBodySize(size_t token);
		void	setExtension(const std::string& token);
		void	setCgiPath(const std::string& token);
		void	setPath(const std::string& token);
		void	setRoot(const std::string& token);
		void	setIndex(const std::string& token);
		void	set_redirectTo(const std::string& token);
		void	setAutoIndex(const std::string& token);
		void	setMethods(const std::vector<std::string>& tokens);
		void	setCgiEnabled(bool enabled) { _cgiEnabled = enabled; }
		void	setCgiExtension(const std::string& ext) { _cgiExtension = ext; }
		void	setCgiBin(const std::string& bin) { _cgiBin = bin; }

		bool	isMethodAllowed(const std::string& method) const;

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
