/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ecarlier <ecarlier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/08 20:25:21 by ecarlier          #+#    #+#             */
/*   Updated: 2024/11/13 17:07:14 by ecarlier         ###   ########.fr       */
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

class Parser
{

    private:
        std::vector<ServerConfig>   _servers;
        ServerConfig                _serverTemplate;

    public:
        Parser();
        ~Parser();

        std::vector<int>                    checkPorts(std::vector<std::string>& tokens);
        std::vector<std::string>            checkHosts(std::vector<std::string>& tokens);

        void                                parseMultipleServers(std::vector<std::string> portVector, std::vector<std::string> hostVector );
        bool                                readFile(const std::string& filePath, std::stringstream& buffer);
        std::string                         removeSemicolon(const std::string& str);
        bool                                ParseConfigStream(std::stringstream& buffer);
        bool                                parseLocation(std::stringstream& buffer, Location& location);

        /* Getters */
        ServerConfig&                       getFirstServer();
        const std::vector<ServerConfig>&    getServers() const;

};

#endif // PARSER_HPP

