/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ecarlier <ecarlier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/02 14:18:42 by ecarlier          #+#    #+#             */
/*   Updated: 2024/10/10 17:02:52 by ecarlier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/TcpServer.hpp"


int main()
{
	TcpServer server;

    // Parser parser;
    // std::stringstream configBuffer;

    // if (!parser.readFile("server_conf/invalid.conf", configBuffer)) {
    //     return 1;
    // }
    // if (!parser.ParseConfigStream(configBuffer)) {
    //     std::cerr << "Erreur lors de l'analyse de la configuration." << std::endl;
    //     return 1;
    // }

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
