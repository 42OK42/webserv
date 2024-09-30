#include "Server.hpp"

Server::Server(const ServerConf& config) : config(config)
{
}

Server::~Server()
{
}

bool Server::init()
{
	std::cout << "Initializing server with the following configuration:" << std::endl;
	std::cout << "Listen Port: " << config.listenPort << std::endl;
	std::cout << "Server Name: " << config.serverName << std::endl;
	std::cout << "Max Body Size: " << config.clientMaxBodySize << " bytes" << std::endl;

	for (std::vector<LocationConf>::const_iterator it = config.locations.begin(); it != config.locations.end(); ++it) {
		const LocationConf& location = *it;
		std::cout << "Location Path: " << location.path << std::endl;
		std::cout << "  Root: " << location.root << std::endl;
		std::cout << "  Index: " << location.index << std::endl;
		std::cout << "  Autoindex: " << (location.autoindex ? "on" : "off") << std::endl;
		std::cout << "  CGI Extension: " << location.cgi.extension << std::endl;
		std::cout << "  CGI Path: " << location.cgi.path << std::endl;
		std::cout << "  Allowed Methods: ";
		for (std::vector<std::string>::const_iterator mit = location.allowedMethods.begin(); mit != location.allowedMethods.end(); ++mit)
			std::cout << *mit << " ";
		std::cout << std::endl;
	}

	return true;
}

void Server::run()
{
	std::cout << "Server is running..." << std::endl;
}