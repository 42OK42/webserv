/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ecarlier <ecarlier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/08 14:54:59 by okrahl            #+#    #+#             */
/*   Updated: 2024/11/17 00:55:51 by ecarlier         ###   ########.fr       */
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
#include <unistd.h>
#include <sys/wait.h>

class Router {
private:
	ServerConfig& _serverConfig;


	std::string readFile(const std::string& filepath);
	std::string generateDirectoryListing(const std::string& dirPath, const std::string& requestPath);
	std::string getCurrentTimestamp() const;
	std::vector<std::string> getFilesInDirectory(const std::string& directory);
	void ensureDirectoryExists(const std::string& path);
	std::string extractFilenameFromUrl(const std::string& url);
	std::string extractFilename(const std::string& contentDisposition);


	void handleGET(const HttpRequest& request, HttpResponse& response, const Location& location);
	void handlePOST(const HttpRequest& request, HttpResponse& response, const Location& location);
	void handleDELETE(const HttpRequest& request, HttpResponse& response, const Location& location);
	void handleCGI(const HttpRequest& request, HttpResponse& response, const Location& location);
	bool isCgiEnabled(const Location& location);
	std::string constructScriptPath(const HttpRequest& request, const Location& location);
	bool createPipes(int input_pipe[2], int output_pipe[2]) ;
	pid_t createFork(int input_pipe[2], int output_pipe[2], HttpResponse& response);
	void executeCgi(const HttpRequest& request, int input_pipe[2], int output_pipe[2], const Location& location, const std::string& scriptPath);
	void handleParentProcess(const HttpRequest& request, HttpResponse& response, int input_pipe[2], int output_pipe[2], pid_t pid);
	std::string decodeChunkedBody(const std::string& body);
	std::string getPathInfo(const std::string& url, const std::string& scriptPath);


public:
	Router(ServerConfig& config);
	~Router();

	void handleRequest(const HttpRequest& request, HttpResponse& response);
	void setErrorResponse(HttpResponse& response, int errorCode);
};

#endif
