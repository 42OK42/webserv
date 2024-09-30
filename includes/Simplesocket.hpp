/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Simplesocket.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ecarlier <ecarlier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/30 15:18:09 by ecarlier          #+#    #+#             */
/*   Updated: 2024/09/30 15:18:31 by ecarlier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef Simplesocket_HPP
#define Simplesocket_HPP

#include <string>
#include <iostream>

class Simplesocket {
	private:

	public:
		Simplesocket();
		Simplesocket(const Simplesocket &copy);
		Simplesocket &operator=(const Simplesocket &copy);
		~Simplesocket();

		// class CustomException : public std::exception {
		// 	public:
		// 		virtual const char* what() const throw();
		// };
};

std::ostream &operator<<(std::ostream &o, Simplesocket const &i);

#endif // Simplesocket_HPP
