/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: okrahl <okrahl@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/08 14:54:59 by okrahl            #+#    #+#             */
/*   Updated: 2024/10/14 14:30:47 by okrahl           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ROUTER_HPP
#define ROUTER_HPP

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <cstring>

class Router
{
	public:

		typedef void (Router::*RouteHandler)(const HttpRequest&, HttpResponse&);

		void addRoute(const std::string& path, RouteHandler handler);
		void handleRequest(const HttpRequest& request, HttpResponse& response);

		void initializeRoutes();

	private:

		std::map<std::string, RouteHandler> routes;
		std::vector<std::string> uploadedFiles;

		void handleHomeRoute(const HttpRequest& req, HttpResponse& res);
		void handleUploadRoute(const HttpRequest& req, HttpResponse& res);
		void handleFormRoute(const HttpRequest& req, HttpResponse& res);
		void handleDeleteRoute(const HttpRequest& req, HttpResponse& res);
		void handleUploadSuccessfulRoute(const HttpRequest& req, HttpResponse& res);
};

#endif