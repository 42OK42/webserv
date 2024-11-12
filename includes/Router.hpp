/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: okrahl <okrahl@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/08 14:54:59 by okrahl            #+#    #+#             */
/*   Updated: 2024/11/12 17:50:49 by okrahl           ###   ########.fr       */
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
#include <ctime>
#include <fstream>

class Router;
typedef void (Router::*RouteHandler)(const HttpRequest&, HttpResponse&);

class Router {
private:
	ServerConfig& _serverConfig;
	std::map<std::string, RouteHandler> routes;

	// Private Hilfsmethoden
	std::vector<std::string> getFilesInDirectory(const std::string& directory);
	void saveUploadedFiles(const HttpRequest& req, const std::string& uploadDir);
	void addRoute(const std::string& path, RouteHandler handler);
	bool handleDirectoryRequest(const std::string& path, const HttpRequest& request, HttpResponse& response);
	std::string getCurrentTimestamp() const;

	// Route Handler
	void handleHomeRoute(const HttpRequest& request, HttpResponse& response);
	void handleUploadRoute(const HttpRequest& request, HttpResponse& response);
	void handleUploadSuccessRoute(const HttpRequest& request, HttpResponse& response);
	void handleFormRoute(const HttpRequest& request, HttpResponse& response);

	// Neue Hilfsmethode für die Verzeichnisauflistung
	std::string generateDirectoryListing(const std::string& dirPath, const std::string& requestPath);

public:
	Router(ServerConfig& config);
	~Router();

	void setErrorResponse(HttpResponse& response, int errorCode);
	
	void initializeRoutes();
	void handleRequest(const HttpRequest& request, HttpResponse& response);
};

#endif
