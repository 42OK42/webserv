/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: okrahl <okrahl@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/08 14:54:59 by okrahl            #+#    #+#             */
/*   Updated: 2024/10/16 15:50:28 by okrahl           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ROUTER_HPP
#define ROUTER_HPP

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <ctime>
#include <sys/stat.h>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cerrno>
#include <cstring>

class Router {

	public:

		Router();
		~Router();

		void initializeRoutes();
		void handleRequest(const HttpRequest& request, HttpResponse& response);
		void saveUploadedFiles(const HttpRequest& request);

		typedef void (Router::*RouteHandler)(const HttpRequest&, HttpResponse&);
		void addRoute(const std::string& path, RouteHandler handler);

	private:

		std::map<std::string, RouteHandler> routes;
		std::vector<std::string> uploadedFiles;

		void handleHomeRoute(const HttpRequest& req, HttpResponse& res);
		void handleFormRoute(const HttpRequest& req, HttpResponse& res);
		void handleUploadRoute(const HttpRequest& req, HttpResponse& res);
};

#endif