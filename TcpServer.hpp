/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TcpServer.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ecarlier <ecarlier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/30 15:38:08 by ecarlier          #+#    #+#             */
/*   Updated: 2024/09/30 15:38:59 by ecarlier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TcpServer_HPP
#define TcpServer_HPP

#include <string>
#include <iostream>

class TcpServer
{
	private:

	public:
		TcpServer();
		TcpServer(const TcpServer &copy);
		TcpServer &operator=(const TcpServer &copy);
		~TcpServer();

		class CustomException : public std::exception {
			public:
				virtual const char* what() const throw();
		};
};

std::ostream &operator<<(std::ostream &o, TcpServer const &i);

#endif // TcpServer_HPP
