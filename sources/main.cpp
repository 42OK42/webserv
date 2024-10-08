/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ecarlier <ecarlier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/02 14:18:42 by ecarlier          #+#    #+#             */
/*   Updated: 2024/10/24 18:53:21 by ecarlier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Location.hpp"
#include "../includes/Parser.hpp"
#include "../includes/ServerConfig.hpp"
#include "../includes/Webserver.hpp"

bool sigint_flag = false;

void handle_signal(int sig) {
    if (sig == SIGINT)
        sigint_flag = true;
    else if (sig == SIGTERM)
        sigint_flag = true;
}


void signalHandler(int signum) {
    std::cout << "Interrupt signal (" << signum << ") received.\n";
    exit(signum);
}

int main(int argc, char **argv)
{
    (void)argv;

    if (argc > 2)
    {
        std::cout << "Wrong number of arguments" << std::endl;
        return (0);
    }
    else if (argc == 1)
         std::cout << "Starting server with default config." << std::endl;


    ServerConfig server;
    Parser parser;
    std::stringstream configBuffer;

    if (!parser.readFile("server_conf/test.conf", configBuffer)) {
        return 1;
    }
    if (!parser.ParseConfigStream(configBuffer)) {
        std::cerr << "Error while parsing." << std::endl;
        return 1;
    }

    std::vector<ServerConfig> servers = parser.getServers();
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    try
    {
        while (!sigint_flag)
        {
            Webserver webserver(servers);
            webserver.initializeServers();
        }


        std::cout << "\033[0;38;5;9m" << "All servers are shut down" << "\033[0m" << std::endl;
    }
    catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}

	return 0;
}

