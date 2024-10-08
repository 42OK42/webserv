/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ecarlier <ecarlier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/02 17:50:39 by okrahl            #+#    #+#             */
/*   Updated: 2024/10/08 11:59:25 by ecarlier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Router.hpp"


void Router::addRoute(const std::string& path, RouteHandler handler)
{
	routes[path] = handler;
}

void Router::handleRequest(const HttpRequest& request, HttpResponse& response) const
{
	std::map<std::string, RouteHandler>::const_iterator it = routes.find(request.getUrl());
	if (it != routes.end())
		it->second(request, response);
	else
	{
		response.setStatusCode(404);
		response.setBody("<html><body><h1>404 Not Found</h1></body></html>");
		response.setHeader("Content-Type", "text/html");
	}
}
