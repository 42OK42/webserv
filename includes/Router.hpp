/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: okrahl <okrahl@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/08 14:54:59 by okrahl            #+#    #+#             */
/*   Updated: 2024/11/13 17:36:08 by okrahl           ###   ########.fr       */
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
#include <sstream>
#include <ctime>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cerrno>
#include <dirent.h>
#include <sys/stat.h>

class Router {
private:
	ServerConfig& _serverConfig;
	
	// Hilfsfunktionen
	std::string readFile(const std::string& filepath);
	std::string generateDirectoryListing(const std::string& dirPath, const std::string& requestPath);
	std::string getCurrentTimestamp() const;
	std::vector<std::string> getFilesInDirectory(const std::string& directory);
	void ensureDirectoryExists(const std::string& path);
	std::string extractFilenameFromUrl(const std::string& url);
	std::string extractFilename(const std::string& contentDisposition);
	
	// HTTP-Methoden-Handler
	void handleGET(const HttpRequest& request, HttpResponse& response, const Location& location);
	void handlePOST(const HttpRequest& request, HttpResponse& response, const Location& location);
	void handleDELETE(const HttpRequest& request, HttpResponse& response, const Location& location);

public:
	Router(ServerConfig& config);
	~Router();

	void handleRequest(const HttpRequest& request, HttpResponse& response);
	void setErrorResponse(HttpResponse& response, int errorCode);
};

#endif
