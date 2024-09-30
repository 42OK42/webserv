#ifndef SERVER_HPP
#define SERVER_HPP

#include "Data.hpp"
#include <string>
#include <iostream>

class Server {
public:
	Server(const ServerConf& config);
	~Server();

	bool init();
	void run();

private:
	ServerConf config;
};

#endif // SERVER_HPP