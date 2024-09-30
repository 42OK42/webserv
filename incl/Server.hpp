
#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <iostream>

class Server
{
	public:

		Server();
		~Server();

		bool init(const std::string& config_file);
		void run();

	private:

	
};

#endif