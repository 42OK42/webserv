/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ecarlier <ecarlier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/08 20:25:19 by ecarlier          #+#    #+#             */
/*   Updated: 2024/11/13 17:45:57 by ecarlier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Parser.hpp"


Parser::Parser() {}


Parser::~Parser() {}

/*
	Reads the content of a configuration file into a stringstream buffer.

	@param filePath A constant reference to a `std::string` representing the
	path of the configuration file to be read.

	@param buffer A reference to a `std::stringstream` where the content of
	the configuration file will be stored.

	@returns bool Returns `true` if the file is successfully opened and read,
	otherwise returns `false` if the file cannot be opened.
*/

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
	Creates server configurations for every combination of the given hosts and ports.
	Validates the ports and hosts, then adds the generated servers to the _servers list.

	@param portVector: A vector of strings representing the ports for the servers.
	@param hostVector: A vector of strings representing the hosts (IP addresses or domain names).

	@return: None. (Modifies the _servers list and outputs creation details to the console).
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
	Checks a vector of host strings for duplicates and ensures that "localhost"
	or "127.0.0.1" is included if no hosts are provided. If a host is a duplicate,
	it is not added to the result. If no hosts are provided, "localhost" is assigned.

	@param tokens: A vector of strings representing host names or IP addresses to check.

	@return: A vector of unique host strings, with "localhost" included if no other hosts are provided.
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
	Gets the vector of all the ports and checks for any duplicates.
	If no ports are provided, assigns port 8080 by default.
	Iterates through the input ports, validates that they are within the valid range (1024-65535),
	and adds them to the ports vector if they are not duplicates.

	Returns:
		std::vector<int> - A vector containing the valid, non-duplicate ports.
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

/*
	Parses the configuration stream to configure multiple servers.
	Reads through the stream line by line, detecting server blocks,
	and extracting configuration directives.
	For each server block, the function builds a corresponding `ServerConfig` object and adds
	it to the `_servers` vector after parsing the specified configurations.

	@param buffer A reference to a `std::stringstream` object containing the configuration data.

	@returns bool Returns `true` if the configuration was successfully parsed and all server blocks
	are correctly processed. Returns `false` if there is an error in the configuration (e.g.,
	missing '{' after a 'server' declaration or unbalanced server block).
*/

bool Parser::ParseConfigStream(std::stringstream& buffer)
{
    ServerConfig serverTemplate;
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
               // std::cout << "Entering new server block...\n";
            } else {
                std::string nextLine;
                if (std::getline(buffer, nextLine) && nextLine.find("{") != std::string::npos) {
                    //std::cout << "Entering new server block...\n";
                } else {
                    std::cerr << "Error: Expected '{' after 'server' declaration.\n";
                    return false;
                }
            }

        if (insideServerBlock) {
                // If we're already inside a server block, finish the current one
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
        if (line.find("}") != std::string::npos) {
            if (insideServerBlock) {
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
                        portVector.push_back(port);
                    }
                }
            }
            else if (key == "host") {
                std::string host;
                while (iss >> host) {
                    host = removeSemicolon(host);
                    if (!host.empty()) {
                        hostVector.push_back(removeSemicolon(host));

                    }
                }
            }
            else if (key == "error_page") {
                while (iss >> errorCode >> errorPage) {
                    errorPage = removeSemicolon(errorPage);
                    serverTemplate.addErrorPage(errorCode, errorPage);
                }
            }
            else if (key == "client_max_body_size") {
                iss >> sizeStr;
                sizeStr = removeSemicolon(sizeStr);
                std::stringstream ss(sizeStr);
                size_t size;
                ss >> size;
                serverTemplate.setClientMaxBodySize(size * 1024 * 1024); // MB to bytes
            }
            else if (key == "location") {
                std::string path;
                iss >> path;
                path = removeSemicolon(path);

                Location location;
                location.setPath(path);
                parseLocation(buffer, location);
                serverTemplate.addLocation(path, location);
            }
        }
    }
    if (insideServerBlock) {
        serverTemplate.checkErrorPage();
		_serverTemplate = serverTemplate;
		parseMultipleServers(portVector, hostVector);
    }

	#ifdef PARSER_MODE
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


/*
	Parses the configuration stream for location-specific settings
	within a server block. The function processes lines related to the
	location configuration, such as 'root', 'index', 'allow_methods',
	'autoindex', 'cgi_extension', 'cgi_path', and 'return'. These values
	are extracted from the stream and set on the provided `Location` object.

	@param buffer A reference to a `std::stringstream` containing the location
	configuration to parse. The function reads the configuration until it
	encounters a closing brace `}` to indicate the end of the location block.

	@param location A reference to the `Location` object where the parsed
	settings will be stored.

	@returns bool Returns `true` if the configuration for the location block
	was successfully parsed. Returns `false` if an error occurs during parsing
	(e.g., if the closing brace `}` is never found.
*/

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

/* ---------------------- Getters ---------------------- */

const std::vector<ServerConfig>& Parser::getServers() const {
    return _servers;
}

ServerConfig& Parser::getFirstServer() {
    if (_servers.empty()) {
        throw std::runtime_error("No servers have been parsed.");
    }
    return _servers[0];
}
