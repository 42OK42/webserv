/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ecarlier <ecarlier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/02 14:18:42 by ecarlier          #+#    #+#             */
/*   Updated: 2024/11/13 19:08:47 by ecarlier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Location.hpp"
#include "../includes/Parser.hpp"
#include "../includes/ServerConfig.hpp"
#include "../includes/Webserver.hpp"


#define RESET       "\033[0m"
#define BOLD        "\033[1m"
#define UNDERLINE   "\033[4m"
#define RED         "\033[31m"
#define GREEN       "\033[32m"
#define YELLOW      "\033[33m"
#define BLUE        "\033[34m"
#define CYAN        "\033[36m"

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
        std::cout << RED << "[ERROR]" << RESET << " Wrong number of arguments. Only one argument allowed for the config file." << std::endl;
        return (0);
    }
    if (argc == 1)
    {
        std::cout << GREEN << "[INFO]" << RESET << " Starting server(s) with default config." << std::endl;
        if (!parser.readFile("server_conf/default.conf", configBuffer)) {
            return 1;
        }
        if (!parser.ParseConfigStream(configBuffer)) {
            std::cerr << RED << "[ERROR]" << RESET << " Error while parsing configuration file." << std::endl;
            return 1;
        }
    }
    else if (argc == 2)
    {
        std::cout << GREEN << "[INFO]" << RESET << " Starting server with config passed as argument: " << YELLOW << argv[1] << RESET << std::endl;
        if (!parser.readFile(argv[1], configBuffer)) {
            return 1;
        }
        if (!parser.ParseConfigStream(configBuffer)) {
            std::cerr << RED << "[ERROR]" << RESET << " Error while parsing configuration file." << std::endl;
            return 1;
        }
    }

    std::vector<ServerConfig> servers = parser.getServers();
    signal(SIGINT, &handle_sigint);
    try
    {
        Webserver webserver(servers);
        webserver.initializeServers();
        std::cout << CYAN << "[INFO]" << RESET << " Servers initialized successfully. Entering event loop..." << std::endl;
        while (!sigint_flag)
        {
            webserver.runEventLoop();
        }

        std::cout << CYAN << "[INFO]" << RESET << " All servers are shut down gracefully." << std::endl;
    }
    catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}

	return 0;
}

