/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ecarlier <ecarlier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/02 14:18:42 by ecarlier          #+#    #+#             */
/*   Updated: 2024/11/11 18:41:59 by ecarlier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Location.hpp"
#include "../includes/Parser.hpp"
#include "../includes/ServerConfig.hpp"
#include "../includes/Webserver.hpp"


bool sigint_flag = false;

void    handle_sigint(int sig) {
    (void)sig;
    sigint_flag = true;
    return ;
}


int main(int argc, char **argv)
{
    (void)argv;
    ServerConfig server;
    Parser parser;
    std::stringstream configBuffer;


    if (argc > 2)
    {
        std::cout << "Wrong number of arguments" << std::endl;
        return (0);
    }
    if (argc == 1)
    {
        std::cout << "Starting server with default config." << std::endl;
        if (!parser.readFile("server_conf/default.conf", configBuffer)) {
            return 1;
        }
        if (!parser.ParseConfigStream(configBuffer)) {
            std::cerr << "Error while parsing." << std::endl;
            return 1;
        }
    }
    else if (argc == 2)
    {
        std::cout << "Starting server with config passed as argument." << std::endl;
        if (!parser.readFile(argv[1], configBuffer)) {
            return 1;
        }
        if (!parser.ParseConfigStream(configBuffer)) {
            std::cerr << "Error while parsing." << std::endl;
            return 1;
        }
    }







    std::vector<ServerConfig> servers = parser.getServers();
    signal(SIGINT, &handle_sigint);
    try
    {
        Webserver webserver(servers);
        webserver.initializeServers();
        while (!sigint_flag)
        {
            webserver.runEventLoop();
        }

        std::cout << "\033[0;38;5;9m" << "All servers are shut down" << "\033[0m" << std::endl;
    }
    catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}

	return 0;
}

