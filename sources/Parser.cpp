/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ecarlier <ecarlier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/08 20:25:19 by ecarlier          #+#    #+#             */
/*   Updated: 2024/10/10 18:37:47 by ecarlier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Parser.hpp"


Parser::Parser() {}


Parser::~Parser() {}


bool Parser::readFile(const std::string& filePath, std::stringstream& buffer)
{
	std::ifstream configFile(filePath.c_str());
	if (!configFile.is_open())
	{
		std::cerr << "Error: Unable to open configuration file" << std::endl;
		return false;
	}

	buffer << configFile.rdbuf();
	configFile.close();
	return true;
}

std::string Parser::removeSemicolon(const std::string& str)
{
	if (!str.empty() && str[str.size() - 1] == ';')
		return str.substr(0, str.size() - 1);
	return str;
}

bool Parser::ParseConfigStream(std::stringstream& buffer)
{
	ServerConfig server;
	std::string line;
	std::string host;
	std::string key;
	int errorCode;
	std::string errorPage;
	std::string sizeStr;
	std::vector<std::string> locationVector;

	while (std::getline(buffer, line))
	{
		std::istringstream iss(line);
		if (!(iss >> key))
			continue;

		key = removeSemicolon(key);
        if (key == "listen")
        {
            std::string port;
            iss >> port;
            std::vector<std::string> portVector;
            portVector.push_back(removeSemicolon(port));
            server.setPort(portVector);
        }
		if (key == "host")
        {
            std::string host;
            iss >> host;
            std::vector<std::string> hostVector;
            hostVector.push_back(removeSemicolon(host));
            server.setHost(hostVector);
        }
		else if (key == "server_name")
		{
			std::string name;
            iss >> name;
            std::vector<std::string> nameVector;
			nameVector.push_back(removeSemicolon(name));
            server.setServerName(nameVector);
        }
		else if (key == "error_page")
		{
          while (iss >> errorCode >> errorPage)
            {
                errorPage = removeSemicolon(errorPage);
                server.addErrorPage(errorCode, errorPage);
            }
		}
		else if (key == "client_max_body_size")
		{
			iss >> sizeStr;
			sizeStr = removeSemicolon(sizeStr);
			std::stringstream ss(sizeStr);
			size_t size;
			ss >> size;
			server.setClientMaxBodySize(size * 1024 * 1024);
		}
		else if (key == "location")
		{
			std::string	path;
			iss >> path;
			path = removeSemicolon(path);

			Location location;
			location.setPath(path);
			parseLocation(buffer, location);
			server.addLocation(path, location);
		}
	}

	std::cout << "Printing Server class ......\n";
	server.print();

	servers.push_back(server);

	// if (!server.serverName.empty())
	// 	data.serverConfig = server;


	return true;
}

bool Parser::parseLocation(std::stringstream& buffer, Location& location)
{
	std::string line;
	std::string key;
	std::string method;
	std::string value;
    std::string	root;
    std::string	index;
	std::string extension;
	std::string cgi_path;


	while (std::getline(buffer, line) && line.find('}') == std::string::npos)
	{
		std::istringstream iss(line);
		if (!(iss >> key))
			continue;

		key = removeSemicolon(key);

		if (key == "root")
		{
			iss >> root;
			location.setRoot(removeSemicolon(root));
		}
		else if (key == "index")
		{
			iss >> index;
			location.setIndex(removeSemicolon(index));
		}
		else if (key == "allow_methods")
		{
			std::vector<std::string> methodVector;
			while (iss >> method)
			{
				method = removeSemicolon(method);
				methodVector.push_back(method);
			}

			location.setMethods(methodVector);
        }
		else if (key == "autoindex")
		{
			iss >> value;
			value = removeSemicolon(value);
			location.setAutoIndex(value);
		}

		else if (key == "cgi_extension")
		{
			iss >> extension;
			location.setExtension(removeSemicolon(extension));
		}
		else if (key == "cgi_path")
		{
			iss >> cgi_path;
			location.setCgiPath(removeSemicolon(cgi_path));
		}
	}

	return true;
}
/* Getters */

const std::vector<ServerConfig>& Parser::getServers() const {
    return servers;
}

ServerConfig& Parser::getFirstServer() {
    if (servers.empty()) {
        throw std::runtime_error("No servers have been parsed.");
    }
    return servers[0];
}
