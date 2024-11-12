/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ecarlier <ecarlier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/08 14:54:59 by okrahl            #+#    #+#             */
/*   Updated: 2024/11/12 21:09:11 by ecarlier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ROUTER_HPP
#define ROUTER_HPP

#include "ServerConfig.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include <cstring>
#include <map>
#include <string>
#include <vector>

class Router;
typedef void (Router::*RouteHandler)(const HttpRequest&, HttpResponse&);

class Router {
private:
	ServerConfig& _serverConfig;
	std::map<std::string, RouteHandler> routes;


	std::vector<std::string> getFilesInDirectory(const std::string& directory);
	void addRoute(const std::string& path, RouteHandler handler);
	bool handleDirectoryRequest(const std::string& path, const HttpRequest& request, HttpResponse& response);

	// Route Handler
	void handleHomeRoute(const HttpRequest& request, HttpResponse& response);
	void handleUploadRoute(const HttpRequest& request, HttpResponse& response);
	void handleUploadSuccessRoute(const HttpRequest& request, HttpResponse& response);
	void handleFormRoute(const HttpRequest& request, HttpResponse& response);
	void handleRedirectRoute(const HttpRequest& request, HttpResponse& response);
	void handleNewpage(const HttpRequest& request, HttpResponse& response);

	std::string generateDirectoryListing(const std::string& dirPath, const std::string& requestPath);

public:
	Router(ServerConfig& config);
	~Router();

	void setErrorResponse(HttpResponse& response, int errorCode);

	void initializeRoutes();
	void handleRequest(const HttpRequest& request, HttpResponse& response);
};

#endif
