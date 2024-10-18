/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: okrahl <okrahl@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/08 14:54:59 by okrahl            #+#    #+#             */
/*   Updated: 2024/10/18 19:01:53 by okrahl           ###   ########.fr       */
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

class Router;

typedef void (Router::*RouteHandler)(const HttpRequest&, HttpResponse&);

class Router {
private:
	ServerConfig& _serverConfig;
	std::map<std::string, RouteHandler> routes;
	std::vector<std::string> uploadedFiles;

	std::string readFile(const std::string& filepath);
	void saveUploadedFiles(const HttpRequest& req);
	std::string extractFilenameFromUrl(const std::string& url);
	void ensureDirectoryExists(const std::string& directory);

public:
	Router(ServerConfig& serverConfig);
	~Router();

	void addRoute(const std::string& path, RouteHandler handler);
	void handleRequest(const HttpRequest& request, HttpResponse& response);

	void handleHomeRoute(const HttpRequest& req, HttpResponse& res);
	void handleFormRoute(const HttpRequest& req, HttpResponse& res);
	void handleUploadRoute(const HttpRequest& req, HttpResponse& res);
	void initializeRoutes();
};

#endif