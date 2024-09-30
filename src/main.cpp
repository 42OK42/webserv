#include "Server.hpp"
#include "Data.hpp"

int main(int argc, char *argv[])
{
	// Check for configuration file argument
	const char* config_file = (argc > 1) ? argv[1] : "config/default.conf";

	// Initialize and start the server
	Server server;
	if (!server.init(config_file))
	{
		std::cerr << "Failed to initialize server." << std::endl;
		return 1;
	}
	server.run();

	return 0;
}