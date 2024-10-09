/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ecarlier <ecarlier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/08 15:49:54 by ecarlier          #+#    #+#             */
/*   Updated: 2024/10/09 22:16:03 by ecarlier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATION_HPP
#define LOCATION_HPP

#include <string>
#include <iostream>
#include <vector>

class Location
{
	private:
		std::string					_path;	// Le chemin URL (ex: "/uploads")
		std::string					_root;	// Répertoire racine pour cette location
		std::string					_index;	// Fichier à servir par défaut (ex: "index.html")
		bool 						_autoIndex;
		std::vector<std::string>	_Methods;
		std::string					_cgi_extension;
		std::string					_cgi_path;

	public:
		Location();
		// Location(const Location &copy);
		// Location &operator=(const Location &copy);
		~Location();

		bool isValidMethod(const std::string& token);
		bool isValidAutoIndex(const std::string& token);

		/*Getters and Setters */
		std::string getPath() const;
		std::string getRoot() const;
		std::string getIndex() const;
		std::string getExtension() const;
		std::string getCgiEXtension() const;
		bool getAutoIndex() const;
		std::vector<std::string> getMethods() const;

		void setExtension(const std::string& token);
		void setCgiPath(const std::string& token);
		void setPath(const std::string& token);
		void setRoot(const std::string& token);
		void setIndex(const std::string& token);
		void setAutoIndex(const std::string& token);
		void setMethods(const std::vector<std::string>& tokens);

		//void	setMethods();
		void print() const;

		class InvalidAutoIndexException: public std::exception {
			public:
				virtual const char* what() const throw();
		};

		class InvalidMethodException : public std::exception {
			public:
				virtual const char* what() const throw();
		};
};


#endif // Location_HPP
