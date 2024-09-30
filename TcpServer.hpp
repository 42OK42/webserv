/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TcpServer.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ecarlier <ecarlier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/30 15:38:08 by ecarlier          #+#    #+#             */
/*   Updated: 2024/09/30 16:22:15 by ecarlier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TcpServer_HPP
#define TcpServer_HPP

#include <string>
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>

class TcpServer
{
	private:
		int	m_socket; //return a socket decriptor


	public:
		TcpServer();
		~TcpServer();

		int TcpServer::startServer();


		class SocketCreationFailed : public std::exception {
			public:
				virtual const char* what() const throw();
		};
};

std::ostream &operator<<(std::ostream &o, TcpServer const &i);

#endif // TcpServer_HPP
