/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ecarlier <ecarlier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/02 14:18:42 by ecarlier          #+#    #+#             */
/*   Updated: 2024/10/21 18:49:34 by ecarlier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/TcpServer.hpp"

#include "../includes/TcpServer.hpp"

#include "../includes/TcpServer.hpp"


int main()
{
	//TcpServer server;
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

    if (!parser.readFile("server_conf/basic.conf", configBuffer)) {
        return 1;
    }
    if (!parser.ParseConfigStream(configBuffer)) {
        std::cerr << "Erreur lors de l'analyse de la configuration." << std::endl;
        return 1;
    }

    server = parser.getFirstServer();
	try
	{

		server.startServer();
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}

	return 0;
}
