/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: okrahl <okrahl@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/02 18:07:22 by okrahl            #+#    #+#             */
/*   Updated: 2024/10/02 18:14:07 by okrahl           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef ROUTER_HPP
#define ROUTER_HPP

#include <string>
#include <map>
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

class Router
{
public:
	typedef void (*RouteHandler)(const HttpRequest&, HttpResponse&);

	void addRoute(const std::string& path, RouteHandler handler);
	void handleRequest(const HttpRequest& request, HttpResponse& response) const;

private:
	std::map<std::string, RouteHandler> routes;
};

void initializeRoutes(Router& router);

#endif // ROUTER_HPP