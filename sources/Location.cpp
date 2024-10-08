/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ecarlier <ecarlier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/08 16:01:05 by ecarlier          #+#    #+#             */
/*   Updated: 2024/10/08 16:01:24 by ecarlier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Location.hpp"

Location::Location()
{
	std::cout << "\033[33m" << "Default constructor called" << "\033[0m" << std::endl;
}

Location::~Location()
{
	std::cout << "\033[32m" << "Destructor called" << "\033[0m" << std::endl;
}

Location::Location(const Location &copy)
{
	std::cout << "\033[34m" << "Copy constructor called" << "\033[0m" << std::endl;
}

Location &Location::operator=(const Location &copy)
{
	std::cout << "\033[35m" << "Copy assignment operator called" << "\033[0m" << std::endl;
	return *this;
}



/* Input Validation */

