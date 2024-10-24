/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: okrahl <okrahl@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/08 14:54:59 by okrahl            #+#    #+#             */
/*   Updated: 2024/10/22 18:32:38 by okrahl           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ROUTER_HPP
#define ROUTER_HPP

#include "ServerConfig.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>

class Router;

typedef void (Router::*RouteHandler)(const HttpRequest&, HttpResponse&);

class Router {
private:
	ServerConfig& _serverConfig;
	std::map<std::string, RouteHandler> routes;
	std::map<std::string, std::string> uploadDirs; // Map for upload directories
	std::vector<std::string> uploadedFiles;

public:
	Router(ServerConfig& serverConfig);
	~Router();

	void addRoute(const std::string& path, RouteHandler handler, const std::string& uploadDir = "");
	void handleRequest(const HttpRequest& request, HttpResponse& response);

	void handleHomeRoute(const HttpRequest& req, HttpResponse& res);
	void handleFormRoute(const HttpRequest& req, HttpResponse& res);
	void handleUploadRoute(const HttpRequest& req, HttpResponse& res);
	void initializeRoutes();

	void saveUploadedFiles(const HttpRequest& req, const std::string& uploadDir); // Updated to include uploadDir
};

#endif // ROUTER_HPP