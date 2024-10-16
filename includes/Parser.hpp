/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ecarlier <ecarlier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/08 20:25:21 by ecarlier          #+#    #+#             */
/*   Updated: 2024/10/16 15:51:35 by ecarlier         ###   ########.fr       */
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
        std::vector<ServerConfig> servers;

    public:
        Parser();
        ~Parser();


        void parseMultipleServers(std::string serverString);
        bool readFile(const std::string& filePath, std::stringstream& buffer);
        std::string removeSemicolon(const std::string& str);
        bool ParseConfigStream(std::stringstream& buffer);
        bool parseLocation(std::stringstream& buffer, Location& location);

        /* Getters */
        ServerConfig& getFirstServer();
        const std::vector<ServerConfig>& getServers() const;

};

#endif // PARSER_HPP
