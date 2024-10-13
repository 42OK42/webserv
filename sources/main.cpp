/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ggiertzu <ggiertzu@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/02 14:18:42 by ecarlier          #+#    #+#             */
/*   Updated: 2024/10/13 15:59:29 by ggiertzu         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/TcpServer.hpp"
#include "../includes/Location.hpp"
#include "../includes/Parser.hpp"
#include "../includes/ServerConfig.hpp"

int main()
{
	TcpServer server;

    Parser parser;
    std::stringstream configBuffer;

    if (!parser.readFile("server_conf/basic.conf", configBuffer)) {
        return 1;
    }
    if (!parser.ParseConfigStream(configBuffer)) {
        std::cerr << "Erreur lors de l'analyse de la configuration." << std::endl;
        return 1;
    }

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
