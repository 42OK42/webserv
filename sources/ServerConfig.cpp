/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ecarlier <ecarlier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/09 15:30:30 by ecarlier          #+#    #+#             */
/*   Updated: 2024/10/09 18:49:41 by ecarlier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerConfig.hpp"
// #include "Parser.hpp"

// Destructeur
ServerConfig::~ServerConfig() {}

/* ---------------------- Setters ---------------------- */

void ServerConfig::setPort(const std::vector<std::string>& tokens)
{
    _port.clear();
    for (size_t i = 0; i < tokens.size(); ++i)
    {
        _port.push_back(atoi(tokens[i].c_str())); // Utilisez atoi à la place de std::stoi
    }
}

void ServerConfig::setHost(const std::vector<std::string>& tokens) {
    _host = tokens;
}

void ServerConfig::setServerName(const std::vector<std::string>& tokens) {
    _serverNames = tokens;
}

void ServerConfig::setRoot(const std::vector<std::string>& tokens) {
    if (!tokens.empty()) {
        _root = tokens[0];
    }
}

void ServerConfig::setErrorPage(const std::vector<std::string>& tokens) {
    if (tokens.size() >= 2) {
        int code = atoi(tokens[0].c_str()); // Utilisez atoi à la place de std::stoi
        // std::string page = removeSemicolon(tokens[1]); // Enlevez le point-virgule
         _errorPages[code] = tokens[1];
    }
}

void ServerConfig::setClientMaxBodySize(size_t token)
{
	_clientMaxBodySize = token;
}

void ServerConfig::setCgiEnabled(const std::vector<std::string>& tokens) {
    if (!tokens.empty()) {
        std::string val = tokens[0];
        _cgiEnabled = (val == "on" || val == "1" || val == "true");
    }
}

void ServerConfig::setCgiExtension(const std::vector<std::string>& tokens) {
    if (!tokens.empty()) {
        _cgiExtension = tokens[0];
    }
}

void ServerConfig::setCgiBin(const std::vector<std::string>& tokens) {
    if (!tokens.empty()) {
        _cgiBin = tokens[0];
    }
}

/* ---------------------- Getters ---------------------- */

int ServerConfig::getListen(size_t idx) const {
    if (idx < _port.size()) {
        return _port[idx];
    }
    return -1; // Return an invalid port if index is out of range
}

size_t ServerConfig::getNbOfPorts() const {
    return _port.size();
}

std::vector<int> ServerConfig::getListen() const {
    return _port;
}

std::string ServerConfig::getHost(size_t idx) const {
    if (idx < _host.size()) {
        return _host[idx];
    }
    return "";
}

std::vector<std::string> ServerConfig::getHost() const {
    return _host;
}

std::string ServerConfig::getServerName(size_t idx) const {
    if (idx < _serverNames.size()) {
        return _serverNames[idx];
    }
    return "";
}

std::vector<std::string> ServerConfig::getServerName() const {
    return _serverNames;
}

std::string ServerConfig::getRoot() const {
    return _root;
}

// std::string ServerConfig::getErrorPage(int code) const {
//     auto it = _errorPages.find(code);
//     if (it != _errorPages.end()) {
//         return it->second;
//     }
//     return "";
// }

int ServerConfig::getClientMaxBodySize() const {
    return static_cast<int>(_clientMaxBodySize);
}

bool ServerConfig::isCgiEnabled() const {
    return _cgiEnabled;
}

std::string ServerConfig::getCgiExtension() const {
    return _cgiExtension;
}

std::string ServerConfig::getCgiBin() const {
    return _cgiBin;
}

/* ---------------- Locations Accessors ---------------- */


const std::map<std::string, Location>& ServerConfig::getLocations() const {
    return _locations; //returns the whole map
}


void ServerConfig::addLocation(const std::string& path, const Location& location) {
    _locations[path] = location;
}




/* DEBUG*/

void ServerConfig::print() const {
    std::cout << "Ports: ";
    for (size_t i = 0; i < _port.size(); ++i) {
        std::cout << _port[i] << " ";
    }
    std::cout << std::endl;

    std::cout << "Hosts: ";
    for (size_t i = 0; i < _host.size(); ++i) {
        std::cout << _host[i] << " ";
    }
    std::cout << std::endl;

    std::cout << "Server Names: ";
    for (size_t i = 0; i < _serverNames.size(); ++i) {
        std::cout << _serverNames[i] << " ";
    }
    std::cout << std::endl;

    std::cout << "Root: " << _root << std::endl;

    std::cout << "Error Pages: " << std::endl;
    for (std::map<int, std::string>::const_iterator it = _errorPages.begin(); it != _errorPages.end(); ++it) {
        std::cout << "  " << it->first << " -> " << it->second << std::endl;
    }

    std::cout << "Locations: " << std::endl;
    for (std::map<std::string, Location>::const_iterator it = _locations.begin(); it != _locations.end(); ++it) {
        std::cout << std::endl;
        std::cout << it->first << ":";
        it->second.print(); // Assurez-vous que la méthode print() existe dans la classe Location
    }
}
