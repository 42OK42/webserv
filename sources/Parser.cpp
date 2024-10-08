/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ecarlier <ecarlier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/08 20:25:19 by ecarlier          #+#    #+#             */
/*   Updated: 2024/10/08 20:25:32 by ecarlier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Parser.hpp"

// Constructeur
Parser::Parser() {}

// Destructeur
Parser::~Parser() {}

// Méthode pour lire le fichier de configuration
bool Parser::readFile(const std::string& filePath, std::stringstream& buffer) {
    std::ifstream configFile(filePath.c_str());
    if (!configFile.is_open()) {
        std::cerr << "Error: Unable to open configuration file" << std::endl;
        return false;
    }

    buffer << configFile.rdbuf();
    configFile.close();
    return true;
}

// Supprime le point-virgule en fin de chaîne
std::string Parser::removeSemicolon(const std::string& str) {
    if (!str.empty() && str.back() == ';')
        return str.substr(0, str.size() - 1);
    return str;
}

// Méthode principale pour lire et analyser le fichier de configuration
bool Parser::parseConfigFile(const std::string& filePath) {
    std::stringstream buffer;
    if (!readFile(filePath, buffer))
        return false;

    std::string line, key;
    while (std::getline(buffer, line)) {
        std::istringstream iss(line);
        if (!(iss >> key))
            continue;

        if (key == "server") {
            ServerConfig server;
            if (!parseServerConfig(buffer, server))
                return false;
            _servers.push_back(server);
        }
    }
    return true;
}

// Récupère les serveurs configurés
std::vector<ServerConfig> Parser::getServers() const {
    return _servers;
}

// Analyse la configuration d'un serveur
bool Parser::parseServerConfig(std::stringstream& buffer, ServerConfig& server) {
    std::string line, key;

    while (std::getline(buffer, line) && line.find('}') == std::string::npos) {
        std::istringstream iss(line);
        if (!(iss >> key))
            continue;

        key = removeSemicolon(key);

        if (key == "listen") {
            std::string port;
            iss >> port;
            server.setPort({port});
        }
        else if (key == "server_name") {
            std::string name;
            iss >> name;
            server.setServerName({name});
        }
        else if (key == "error_page") {
            int code;
            std::string path;
            iss >> code >> path;
            server.setErrorPage({std::to_string(code), removeSemicolon(path)});
        }
        else if (key == "client_max_body_size") {
            std::string sizeStr;
            iss >> sizeStr;
            sizeStr = removeSemicolon(sizeStr);
            server.setClientMaxBodySize({sizeStr});
        }
        else if (key == "location") {
            Location location;
            std::string path;
            iss >> path;
            location.setPath(removeSemicolon(path));
            if (!parseLocationConfig(buffer, location))
                return false;
            server._locations[location.getPath()] = location;
        }
    }
    return true;
}

// Analyse la configuration d'une location
bool Parser::parseLocationConfig(std::stringstream& buffer, Location& location) {
    std::string line, key, value;

    while (std::getline(buffer, line) && line.find('}') == std::string::npos) {
        std::istringstream iss(line);
        if (!(iss >> key))
            continue;

        key = removeSemicolon(key);

        if (key == "root") {
            iss >> value;
            location.setRoot(removeSemicolon(value));
        }
        else if (key == "index") {
            iss >> value;
            location.setIndex(removeSemicolon(value));
        }
        else if (key == "allow_methods") {
            std::vector<std::string> methods;
            while (iss >> value) {
                methods.push_back(removeSemicolon(value));
            }
            location.setMethods(methods);
        }
        else if (key == "autoindex") {
            iss >> value;
            location.setAutoIndex(value == "on");
        }
    }
    return true;
}
