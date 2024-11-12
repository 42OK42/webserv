/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ecarlier <ecarlier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/08 20:25:19 by ecarlier          #+#    #+#             */
/*   Updated: 2024/11/12 20:16:43 by ecarlier         ###   ########.fr       */
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
		ports.push_back(8080);

	return (ports);

}
bool Parser::ParseConfigStream(std::stringstream& buffer)
{
    ServerConfig serverTemplate;   // Create a new server template for each server block
    std::string line, key, errorPage, sizeStr;
    int errorCode;
    std::vector<std::string> locationVector, portVector, hostVector, nameVector;
    bool insideServerBlock = false;

    while (std::getline(buffer, line))
    {
        std::istringstream iss(line);
        if (!(iss >> key))
            continue;

        key = removeSemicolon(key);

		if (key == "server") {
            if (line.find("{") != std::string::npos) {
                std::cout << "Entering new server block...\n";
            } else {
                std::string nextLine;
                if (std::getline(buffer, nextLine) && nextLine.find("{") != std::string::npos) {
                    std::cout << "Entering new server block...\n";
                } else {
                    std::cerr << "Error: Expected '{' after 'server' declaration.\n";
                    return false;
                }
            }

        if (insideServerBlock) {
                // If we're already inside a server block, finish the current one
               // std::cout << "Finished server block, pushing to servers.\n";
                serverTemplate.checkErrorPage();
                _serverTemplate = serverTemplate;
                parseMultipleServers(portVector, hostVector);
                serverTemplate = ServerConfig();    // Reset for the next server
                portVector.clear();  // Reset portVector and other vectors if necessary
                hostVector.clear();
                nameVector.clear();
            }
            insideServerBlock = true;  // Now inside a new server block
            continue;
        }

        // Detect the end of a server block
        if (line.find("}") != std::string::npos) {
            if (insideServerBlock) {
                //std::cout << "Closing server block...\n";
				serverTemplate.checkErrorPage();
				_serverTemplate = serverTemplate;
				parseMultipleServers(portVector, hostVector);
                serverTemplate = ServerConfig();    // Reset for next server
                insideServerBlock = false;          // Exit the server block
                portVector.clear();  // Reset vectors for the next server block
                hostVector.clear();
                nameVector.clear();
            }
            continue;
        }

        if (insideServerBlock) {
            if (key == "listen") {
                std::string port;
                while (iss >> port) {
                    port = removeSemicolon(port);
                    if (!port.empty()) {
                        std::cout << "Adding port: " << port << std::endl;
                        portVector.push_back(port);
                    }
                }
            }
            else if (key == "host") {
                std::string host;
                while (iss >> host) {
                    host = removeSemicolon(host);
                    if (!host.empty()) {
                        std::cout << "Adding host: " << host << std::endl;
                        hostVector.push_back(removeSemicolon(host));

                    }
                }
            }
            else if (key == "server_name") {
                std::string name;
                while (iss >> name) {
                    name = removeSemicolon(name);
                    if (!name.empty()) {
                        std::cout << "Adding server_name: " << name << std::endl;
						nameVector.push_back(removeSemicolon(name));
						serverTemplate.setServerName(nameVector);

                    }
                }
            }
            else if (key == "error_page") {
                while (iss >> errorCode >> errorPage) {
                    errorPage = removeSemicolon(errorPage);
                    std::cout << "Adding error page for code: " << errorCode << ", page: " << errorPage << std::endl;
                    serverTemplate.addErrorPage(errorCode, errorPage);
                }
            }
            else if (key == "client_max_body_size") {
                iss >> sizeStr;
                sizeStr = removeSemicolon(sizeStr);
                std::stringstream ss(sizeStr);
                size_t size;
                ss >> size;
                std::cout << "Setting client_max_body_size to: " << size << " MB\n";
                serverTemplate.setClientMaxBodySize(size * 1024 * 1024); // MB to bytes
            }
            else if (key == "location") {
                std::string path;
                iss >> path;
                path = removeSemicolon(path);

                Location location;
                location.setPath(path);
                parseLocation(buffer, location);
                std::cout << "Adding location: " << path << std::endl;
                serverTemplate.addLocation(path, location);
            }
        }
    }
    if (insideServerBlock) {

        std::cout << "Finalizing last server block...\n";
        serverTemplate.checkErrorPage();
		_serverTemplate = serverTemplate;
		parseMultipleServers(portVector, hostVector);
    }

	#ifdef DEBUG_MODE
    if (_servers.empty()) {
        std::cout << "Error: _servers is empty!\n";
    } else {
        std::cout << "Printing all configured servers:\n";
        for (size_t i = 0; i < _servers.size(); ++i)
            std::cout << _servers[i];
    }
	#endif
    return true;
}



bool Parser::parseLocation(std::stringstream& buffer, Location& location)
{
	std::string line, key, method, value, root, index, extension, cgi_path, _redirectTo;

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
		if (key == "return")
		{
			iss >> _redirectTo;
			location.set_redirectTo(removeSemicolon(_redirectTo));
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
