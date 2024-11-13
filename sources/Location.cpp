/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ecarlier <ecarlier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/08 16:01:05 by ecarlier          #+#    #+#             */
/*   Updated: 2024/11/13 18:28:13 by ecarlier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Location.hpp"

Location::Location() {}

Location::~Location() {}

/*
	@note The valid HTTP methods are constrained to "GET", "POST", and "DELETE".
	Any other method will be considered invalid and will raise an exception.
	(see subject)
*/
bool Location::isValidMethod(const std::string& token)
{
	if (token != "GET"
		&& token != "POST"
		&& token != "DELETE"
		&& token != "")
		return false;
	return true;
}

bool Location::isValidAutoIndex(const std::string& token)
{
	if (token != "on" && token != "1" && token != "yes"
		&& token != "off" && token != "0" && token != "no")
		return false;
	return true;
}

/* ---------------------- Setters ---------------------- */

void Location::setPath(const std::string& token)
{
	_path = token;
}

void Location::setRoot(const std::string& token)
{
	_root = token;
}
void Location::set_redirectTo(const std::string& token)
{
	_redirectTo = token;
}

void Location::setIndex(const std::string& token)
{
	_index = token;
}

void Location::setAutoIndex(const std::string& token)
{
	if (!isValidAutoIndex(token))
		throw InvalidAutoIndexException();
	if (token == "on" || token == "1" || token == "yes")
		_autoIndex = true;
	else
		_autoIndex = false;
}

void Location::setExtension(const std::string& token)
{
	_cgi_extension = token;
}

void Location::setCgiPath(const std::string& token)
{
	_cgi_path = token;
}

/*
	Sets the allowed HTTP methods for a location.

	The function validates each HTTP method in the provided vector and adds valid methods
	to the `_Methods` vector. Invalid methods trigger an `InvalidMethodException` and an error
	message is printed, but the program continues execution.

	@param methods A vector of HTTP methods to be set for the location.
	@returns void
*/
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
            std::cerr << "Error : " << e.what() << std::endl;
        }
    }
}

/* ---------------------- Getters ---------------------- */

std::string Location::getPath() const
{
	return _path;
}

std::string Location::get_redirectTo() const
{
	return _redirectTo;
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

bool Location::isMethodAllowed(const std::string& method) const
{
	return std::find(_Methods.begin(), _Methods.end(), method) != _Methods.end();
}

/* ---------------------- Exceptions ---------------------- */

const char* Location::InvalidAutoIndexException::what() const throw ()
{
	return "Throwing exception: invalid auto index";
}

const char* Location::InvalidMethodException::what() const throw ()
{
	return "Throwing exception: invalid method";
}

/* ---------------------- Overload Operators ---------------------- */

std::ostream& operator<<(std::ostream& os, const Location& location)
{
	os << "\n" << BOLD << BLUE << "# Location " << RESET << location.getPath()  << std::endl;

	os << YELLOW << std::setw(15) << "Path:" << RESET << location.getPath() << "\n";
	os << YELLOW << std::setw(15) << "Root:" << RESET << location.getRoot() << "\n";
	os << YELLOW << std::setw(15) << "Index:" << RESET << location.getIndex() << "\n";
	os << YELLOW << std::setw(15) << "Redirect to:" << RESET << location.get_redirectTo() << "\n";

	os << YELLOW << std::setw(15) << "AutoIndex:" << RESET
		<< (location.getAutoIndex() ? GREEN "on" RESET : RED "off" RESET) << "\n";

	os << YELLOW << std::setw(15) << "Allowed Methods:" << RESET;
	const std::vector<std::string>& methods = location.getMethods();
	if (!methods.empty()) {
		for (size_t i = 0; i < methods.size(); ++i) {
			os << CYAN << methods[i] << RESET;
			if (i < methods.size() - 1) os << ", ";
		}
	} else {
		os << RED << "None" << RESET;
	}
	os << "\n";

	return os;
}
