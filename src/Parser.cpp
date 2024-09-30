#include "Parser.hpp"

std::stringstream readConfigFile(const std::string& filePath)
{
	std::ifstream configFile(filePath);
	if (!configFile.is_open())
	{
		std::cerr << "Error: File not readable" << std::endl;
		return std::stringstream();
	}
	std::stringstream buffer;
	buffer << configFile.rdbuf();
	configFile.close();
	return buffer;
}

bool parseConfigStream(std::stringstream& buffer, Data& data)
{
	ServerConf server;
	std::string line;
	std::string key;

	while (std::getline(buffer, line))
	{
		std::istringstream iss(line);
		if (!(iss >> key))
			continue;

		if (key == "listen")
			iss >> server.listenPort;
		else if (key == "server_name")
			iss >> server.serverName;
		else if (key == "error_page")
		{
			int errorCode;
			std::string errorPage;
			while (iss >> errorCode >> errorPage)
				server.errorPages[errorCode] = errorPage;
		}
		else if (key == "client_max_body_size")
		{
			std::string sizeStr;
			iss >> sizeStr;
			server.clientMaxBodySize = std::stoul(sizeStr) * 1024 * 1024;
		}
		else if (key == "location")
		{
			LocationConf location;
			iss >> location.path;
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

	while (std::getline(buffer, line) && line.find('}') == std::string::npos)
	{
		std::istringstream iss(line);
		if (!(iss >> key))
			continue;

		if (key == "root")
			iss >> location.root;
		else if (key == "index")
			iss >> location.index;
		else if (key == "allow_methods")
		{
			std::string method;
			while (iss >> method)
				location.allowedMethods.push_back(method);
		}
		else if (key == "autoindex")
		{
			std::string value;
			iss >> value;
			location.autoindex = (value == "on");
		}
		else if (key == "cgi_extension" || key == "cgi_path")
			parseCGI(iss, location.cgi);
	}
	return true;
}

bool parseCGI(std::istringstream& iss, CGIConfig& cgi)
{
	std::string key;
	while (iss >> key)
	{
		if (key == "cgi_extension")
			iss >> cgi.extension;
		else if (key == "cgi_path")
			iss >> cgi.path;
	}
	return true;
}