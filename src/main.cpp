#include "Server.hpp"
#include "Data.hpp"
#include "Parser.hpp"
#include <iostream>
#include <sstream>

int main(int argc, char *argv[])
{
	// Check for configuration file argument
	const char* config_file = (argc > 1) ? argv[1] : "config/default.conf";

	std::stringstream buffer = readConfigFile(config_file);
	if (buffer.str().empty()) {
		std::cerr << "Failed to read configuration file." << std::endl;
		return 1;
	}

	Data data;
	if (!parseConfigStream(buffer, data))
	{
		std::cerr << "Failed to parse configuration." << std::endl;
		return 1;
	}

	Server server(data.serverConfig);
	if (!server.init())
	{
		std::cerr << "Failed to initialize server." << std::endl;
		return 1;
	}
	server.run();

	return 0;
}