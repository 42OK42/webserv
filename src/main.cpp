#include "Server.hpp"
#include "Data.hpp"
#include "Parser.hpp"

int main(int argc, char *argv[])
{

	if (argc <= 1)
	{
		std::cerr << "Error: No configuration file provided." << std::endl;
		return 1;
	}

	const char* config_file = argv[1];

	std::stringstream buffer;
	if (!readConfigFile(config_file, buffer))
	{
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