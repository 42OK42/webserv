/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ecarlier <ecarlier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/08 20:25:21 by ecarlier          #+#    #+#             */
/*   Updated: 2024/10/09 16:09:06 by ecarlier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <vector>
#include "ServerConfig.hpp"
#include "Location.hpp"

class Parser {
private:
    std::vector<ServerConfig> _servers; // Collection de serveurs configurés

public:
    // Constructeur et destructeur
    Parser();
    ~Parser();

    // Méthode principale pour lire et analyser le fichier de configuration
    bool parseConfigFile(const std::string& filePath);
    std::string removeSemicolon(const std::string& str);
    // Méthodes d'accès pour récupérer les serveurs configurés
    bool ParseConfigStream(std::stringstream& buffer);
    bool parseLocation(std::stringstream& buffer, Location& location);
    std::vector<ServerConfig> getServers() const;

private:
    // Méthodes auxiliaires pour parser les différentes sections
    bool parseServerConfig(std::stringstream& buffer, ServerConfig& server);
    bool parseLocationConfig(std::stringstream& buffer, Location& location);


    // Méthode pour lire le fichier de configuration
    bool readFile(const std::string& filePath, std::stringstream& buffer);
};

#endif // PARSER_HPP
