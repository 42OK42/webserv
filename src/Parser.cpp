#include "Parser.hpp"

std::string removeSemicolon(const std::string& str)
{
	if (!str.empty() && str[str.size() - 1] == ';')
		return str.substr(0, str.size() - 1);
	return str;
}

bool readConfigFile(const std::string& filePath, std::stringstream& buffer)
{
	std::ifstream configFile(filePath.c_str());
	if (!configFile.is_open()) {
		std::cerr << "Error: File not readable" << std::endl;
		return false;
	}

	buffer << configFile.rdbuf();
	configFile.close();
	return true;
}

bool parseConfigStream(std::stringstream& buffer, Data& data)
{
	ServerConf server;
	std::string line;
	std::string key;
	int errorCode;
	std::string errorPage;
	std::string sizeStr;
	LocationConf location;

	while (std::getline(buffer, line))
	{
		std::istringstream iss(line);
		if (!(iss >> key))
			continue;

		key = removeSemicolon(key);

		if (key == "listen")
			iss >> server.listenPort;
		else if (key == "server_name")
			iss >> server.serverName;
		else if (key == "error_page")
		{
			while (iss >> errorCode >> errorPage) {
				errorPage = removeSemicolon(errorPage);
				server.errorPages[errorCode] = errorPage;
			}
		}
		else if (key == "client_max_body_size")
		{
			iss >> sizeStr;
			sizeStr = removeSemicolon(sizeStr);
			std::stringstream ss(sizeStr);
			size_t size;
			ss >> size;
			server.clientMaxBodySize = size * 1024 * 1024;
		}
		else if (key == "location")
		{
			iss >> location.path;
			location.path = removeSemicolon(location.path);
			parseLocation(buffer, location);
			server.locations.push_back(location);
		}
	}

	if (!server.serverName.empty())
		data.serverConfig = server;

	return true;
}

bool parseLocation(std::stringstream& buffer, LocationConf& location)
{
	std::string line;
	std::string key;
	std::string method;
	std::string value;

	while (std::getline(buffer, line) && line.find('}') == std::string::npos)
	{
		std::istringstream iss(line);
		if (!(iss >> key))
			continue;

		key = removeSemicolon(key);

		if (key == "root")
		{
			iss >> location.root;
			location.root = removeSemicolon(location.root);
		}
		else if (key == "index")
		{
			iss >> location.index;
			location.index = removeSemicolon(location.index);
		}
		else if (key == "allow_methods")
		{
			location.allowedMethods.clear();
			while (iss >> method)
			{
				method = removeSemicolon(method);
				location.allowedMethods.push_back(method);
			}
		}
		else if (key == "autoindex")
		{
			iss >> value;
			value = removeSemicolon(value);
			location.autoindex = (value == "on");
		}
		else if (key == "cgi_extension")
		{
			iss >> location.cgi.extension;
			location.cgi.extension = removeSemicolon(location.cgi.extension);
		}
		else if (key == "cgi_path")
		{
			iss >> location.cgi.path;
			location.cgi.path = removeSemicolon(location.cgi.path);
		}
	}
	return true;
}

bool parseCGI(std::istringstream& iss, CGIConfig& cgi)
{
	std::string key;
	std::string value;

	while (iss >> key >> value)
	{
		key = removeSemicolon(key);
		value = removeSemicolon(value);

		if (key == "cgi_extension")
			cgi.extension = value;
		else if (key == "cgi_path")
			cgi.path = value;
	}
	return true;
}