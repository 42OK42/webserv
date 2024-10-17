/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: okrahl <okrahl@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/02 14:18:42 by ecarlier          #+#    #+#             */
/*   Updated: 2024/10/17 17:34:14 by okrahl           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/TcpServer.hpp"

int main(int argc, char **argv)
{
	(void)argv; // Markieren Sie argv als bewusst ungenutzt

	if (argc > 2)
	{
		std::cout << "Wrong number of arguments" << std::endl;
		return 0;
	}
	else if (argc == 1)
	{
		std::cout << "Starting server with default config." << std::endl;
	}

	TcpServer server;

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