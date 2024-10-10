/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Init_Routes.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ecarlier <ecarlier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/02 17:55:55 by okrahl            #+#    #+#             */
/*   Updated: 2024/10/08 12:37:00 by ecarlier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "Router.hpp"
#include <fstream>
#include <sstream>

std::string readFile(const char* filePath)
{
	std::ifstream file(filePath);
	if (!file.is_open())
		return "<html><body><h1>404 Not Found</h1></body></html>";
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

void handleRoot(const HttpRequest& req, HttpResponse& res)
{
	if (req.getMethod() == "GET")
	{
		std::string content = readFile("welcome.html");
		res.setStatusCode(200);
		res.setBody(content);
		res.setHeader("Content-Type", "text/html");
	}
	// else if (req.getMethod() == "POST")
	// {

	// }
	// else if (req.getMethod() == "DELETE")
	// {

	// }
	else
	{
		res.setStatusCode(405); //405 method not allowed
		res.setBody("<html><body><h1>405 Method Not Allowed</h1></body></html>");
		res.setHeader("Content-Type", "text/html");
	}
}

void initializeRoutes(Router& router)
{
	router.addRoute("/", handleRoot);
	// Weitere Routen können hier hinzugefügt werden
}
