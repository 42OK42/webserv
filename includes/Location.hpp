/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ecarlier <ecarlier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/08 15:49:54 by ecarlier          #+#    #+#             */
/*   Updated: 2024/10/08 16:59:35 by ecarlier         ###   ########.fr       */
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
		std::string	_path;                       // Le chemin URL (ex: "/uploads")
		std::string	_root;                       // Répertoire racine pour cette location
		std::string	_index;                      // Fichier à servir par défaut (ex: "index.html")
		bool 		_autoIndex;
		std::vector<std::string>	_allowedMethods; // Méthodes HTTP acceptées (GET, POST, etc.)

	public:
		Location();
		// Location(const Location &copy);
		// Location &operator=(const Location &copy);
		~Location();

		class CustomException : public std::exception {
			public:
				virtual const char* what() const throw();
		};
};


#endif // Location_HPP
