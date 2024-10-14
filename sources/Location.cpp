/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ecarlier <ecarlier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/08 16:01:05 by ecarlier          #+#    #+#             */
/*   Updated: 2024/10/10 19:49:13 by ecarlier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Location.hpp"

Location::Location()
{
	//std::cout << "\033[33m" << "Default constructor called" << "\033[0m" << std::endl;
}

Location::~Location()
{
	//std::cout << "\033[32m" << "Destructor called" << "\033[0m" << std::endl;
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


void Location::setExtension(const std::string& token)
{
	_cgi_extension = token;
}


void Location::setCgiPath(const std::string& token)
{
	_cgi_path = token;
}


void Location::setMethods(const std::vector<std::string>& methods)
{
	for (std::vector<std::string>::const_iterator it = methods.begin(); it != methods.end(); ++it)
	{
		try
		{
			if (!isValidMethod(*it))
				throw InvalidMethodException();
			else
				_Methods.push_back(*it);
		}
		catch (const InvalidMethodException& e)
		{
			std::cerr << "Error: " << e.what() << std::endl;
		}
	}
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
	return ("Trowing exception : invalid method");
}

/*			 Overload operators			*/
std::ostream& operator<<(std::ostream& os, const Location& location) {
	os << "\n### Location ###" << std::endl;
	os << "Path: " << location.getPath() << "\n";
	os << "Root: " << location.getRoot() << "\n";
	os << "Index: " << location.getIndex() << "\n";
	os << "AutoIndex: " << (location.getAutoIndex() ? "on" : "off") << "\n";
	os << "Allowed Methods: ";

	const std::vector<std::string>& methods = location.getMethods();
	for (size_t i = 0; i < methods.size(); ++i) {
	os << methods[i] << " ";
	}

	os << std::endl;
	return os;
}

