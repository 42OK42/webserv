
#include "Server.hpp"

Server::Server()
{

}

Server::~Server()
{

}

bool Server::init(const std::string& config_file)
{
	std::cout << "Loading configuration from: " << config_file << std::endl;
	return true;
}

void Server::run()
{
	std::cout << "Server is running..." << std::endl;
}