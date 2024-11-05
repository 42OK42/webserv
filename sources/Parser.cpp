/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: okrahl <okrahl@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/08 20:25:19 by ecarlier          #+#    #+#             */
/*   Updated: 2024/11/05 16:04:09 by okrahl           ###   ########.fr       */
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

/*
	Gets the vectors of hosts and ports and create a server with every possible combinaison
*/
void Parser::parseMultipleServers(std::vector<std::string> portVector, std::vector<std::string> hostVector )
{
	std::vector<int> ports =  checkPorts(portVector);
	std::vector<std::string> hosts = checkHosts(hostVector);

	for (size_t i = 0; i < hosts.size(); ++i)
	{
		for (size_t j = 0; j < ports.size(); ++j)
		{
			ServerConfig server = _serverTemplate;
			server.setHost(hosts[i]);
			server.setPort(ports[j]);
			_servers.push_back(server);


			std::cout << "Created server on host " << hosts[i] << " and port " << ports[j] << std::endl;
		}
	}
}


/*
	Gets the vector of all the hosts and check for any duplicates
	If no hosts are provided, localhost is assigned
*/
std::vector<std::string> Parser::checkHosts(std::vector<std::string>& tokens)
{
	std::vector<std::string> hosts;

	bool isLocalHost = false;

	if (!tokens.empty())
	{
		for (size_t i = 0; i < tokens.size(); ++i)
		{
			std::string host = tokens[i];
			bool isDuplicate = false;
			isLocalHost = false;
			if (host == "localhost" || host == "127.0.0.1" )
			{
				isLocalHost = true;
			}
			for (size_t j = 0; j < hosts.size(); ++j)
			{
				if (hosts[j] == host)
				{
					isDuplicate = true;
					break;
				}
			}
			if (!isDuplicate && !isLocalHost)
				hosts.push_back(host);
		}
		if (isLocalHost)
			hosts.push_back("localhost");
	}
	else
		hosts.push_back("localhost");

	return hosts;
}


/*
	Gets the vector of all the ports and check for any duplicates
	If no ports are provided, port 8080 is assigned
*/
std::vector<int> Parser::checkPorts( std::vector<std::string>& tokens)
{

	std::vector<int> ports;

	if (!tokens.empty())
	{
		for (size_t i = 0; i < tokens.size(); ++i)
		{
			int port = atoi(tokens[i].c_str());
			if (port < 1024 || port > 65535)
				continue;
			bool isDuplicate = false;
			for (size_t j = 0; j < ports.size(); ++j)
			{
				if (ports[j] == port)
				{
					isDuplicate = true;
					break;
				}
			}
			if (!isDuplicate)
				ports.push_back(port);
		}
	}
	else
		ports[0] = 8080;

	return (ports);

}


bool Parser::ParseConfigStream(std::stringstream& buffer)
{
	std::string line;
	ServerConfig* currentServer = NULL;

	while (std::getline(buffer, line))
	{
		std::istringstream iss(line);
		std::string key;
		if (!(iss >> key))
			continue;

		if (key == "server")
		{
			if (currentServer != NULL)
			{
				currentServer->checkErrorPage();
				_servers.push_back(*currentServer);
				delete currentServer;
			}
			currentServer = new ServerConfig();
			continue;
		}

		if (currentServer)
		{
			if (key == "listen")
			{
				std::string port;
				iss >> port;
				port = removeSemicolon(port);
				currentServer->setPort(atoi(port.c_str()));
			}
			else if (key == "host")
			{
				std::string host;
				iss >> host;
				host = removeSemicolon(host);
				currentServer->setHost(host);
			}
			else if (key == "server_name")
			{
				std::vector<std::string> serverNames;
				std::string name;
				while (iss >> name)
				{
					name = removeSemicolon(name);
					serverNames.push_back(name);
				}
				currentServer->setServerName(serverNames);
			}
			else if (key == "error_page")
			{
				int errorCode;
				std::string errorPath;
				if (iss >> errorCode >> errorPath)
				{
					errorPath = removeSemicolon(errorPath);
					currentServer->addErrorPage(errorCode, errorPath);
				}
			}
			else if (key == "client_max_body_size")
			{
				std::string sizeStr;
				iss >> sizeStr;
				sizeStr = removeSemicolon(sizeStr);
				size_t size;
				std::istringstream(sizeStr) >> size;
				currentServer->setClientMaxBodySize(size * 1024 * 1024);
			}
			else if (key == "location")
			{
				std::string path;
				iss >> path;
				path = removeSemicolon(path);

				Location location;
				location.setPath(path);
				parseLocation(buffer, location);
				currentServer->addLocation(path, location);
			}
		}
	}

	if (currentServer != NULL)
	{
		currentServer->checkErrorPage();
		_servers.push_back(*currentServer);
		delete currentServer;
	}

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
    return _servers;
}

ServerConfig& Parser::getFirstServer() {
    if (_servers.empty()) {
        throw std::runtime_error("No servers have been parsed.");
    }
    return _servers[0];
}
