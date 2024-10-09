/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ecarlier <ecarlier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/08 16:01:05 by ecarlier          #+#    #+#             */
/*   Updated: 2024/10/09 16:10:09 by ecarlier         ###   ########.fr       */
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

// Location::Location(const Location &copy)
// {
// 	std::cout << "\033[34m" << "Copy constructor called" << "\033[0m" << std::endl;
// }

// Location &Location::operator=(const Location &copy)
// {
// 	std::cout << "\033[35m" << "Copy assignment operator called" << "\033[0m" << std::endl;
// 	return *this;
// }

/*			Input Validation			*/

bool	Location::isValidMethod(const std::string& token)
{
	if (token != "GET"
		&& token != "POST"
		&& token != "DELETE")
		return (false);
	return (true);
}

bool	Location::isValidAutoIndex(const std::string& token)
{
	if (token != "on" && token != "1" && token != "yes"
		&& token != "off" && token != "0" && token != "no")
		return (false);
	return (true);
}


/*			Setters			*/

void Location::setPath(const std::string& token)
{
	_path = token;
}

void Location::setRoot(const std::string& token)
{
	_root = token;
}

void Location::setIndex(const std::string& token)
{
	_index = token;
}

void Location::setAutoIndex(const std::string& token)
{
	if (!isValidAutoIndex(token))
		throw (InvalidAutoIndexException());
	if (token == "on" || token == "1" || token == "yes")
		_autoIndex = true;
	else
		_autoIndex = true;

}

// void Location::setMethods(const std::string& token)
// {
// 	// maybe this needs to be a vector ?
// 	if (!isValidMethod(token))
// 		throw (InvalidMethodException());
// }

void Location::setMethods(const std::vector<std::string>& methods) {
    for (std::vector<std::string>::const_iterator it = methods.begin(); it != methods.end(); ++it) {
        if (!isValidMethod(*it)) {
            throw InvalidMethodException();
        }
    }
    _Methods = methods;
}


/*			Getters			*/

std::string Location::getPath() const
{
	return _path;
}

std::string Location::getRoot() const
{
	return _root;
}

std::string Location::getIndex() const
{
	return _index;
}

bool Location::getAutoIndex() const
{
	return _autoIndex;
}

std::vector<std::string> Location::getMethods() const
{
	return _Methods;
}


/*			Execptions			*/

const char* Location::InvalidAutoIndexException::what() const throw ()
{
	return ("Trowing exception : invalid auto index ");
}

const char* Location::InvalidMethodException::what() const throw ()
{
	return ("Trowing exception : invalid auto index ");
}
