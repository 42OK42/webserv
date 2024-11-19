/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: okrahl <okrahl@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/08 14:54:59 by okrahl            #+#    #+#             */
/*   Updated: 2024/11/19 21:05:46 by okrahl           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ROUTER_HPP
#define ROUTER_HPP

#include "Common.hpp"
#include "ServerConfig.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "Webserver.hpp"
#include <string>


class Router
{
	private:

		ServerConfig& _serverConfig;

		void	handleGET(const HttpRequest& request, HttpResponse& response, const Location& location);
		void	handlePOST(const HttpRequest& request, HttpResponse& response, const Location& location);
		void	handleDELETE(const HttpRequest& request, HttpResponse& response, const Location& location);
		void	handleCGI(const HttpRequest& request, HttpResponse& response, const Location& location);
		void	ensureDirectoryExists(const std::string& path);
		void	executeCgi(const HttpRequest& request, int input_pipe[2], int output_pipe[2], const Location& location, const std::string& scriptPath);
		void	handleParentProcess(const HttpRequest& request, HttpResponse& response, int input_pipe[2], int output_pipe[2], pid_t pid);

		bool	isCgiEnabled(const Location& location);
		bool	createPipes(int input_pipe[2], int output_pipe[2]) ;

		std::string	readFile(const std::string& filepath);
		std::string	generateDirectoryListing(const std::string& dirPath, const std::string& requestPath);
		std::string	getCurrentTimestamp() const;
		std::string	extractFilenameFromUrl(const std::string& url);
		std::string	extractFilename(const std::string& contentDisposition);
		std::string	decodeChunkedBody(const std::string& body);
		std::string	getPathInfo(const std::string& url, const std::string& scriptPath);
		std::string	constructScriptPath(const HttpRequest& request, const Location& location);

		std::vector<std::string>	getFilesInDirectory(const std::string& directory);

		pid_t	createFork(int input_pipe[2], int output_pipe[2], HttpResponse& response);

	public:
		static const int SOCKET_TIMEOUT_SECONDS = 10;
		static const int READ_TIMEOUT_SECONDS = 30;

		Router(ServerConfig& config);
		~Router();

		void	handleRequest(const HttpRequest& request, HttpResponse& response);
		void	setErrorResponse(HttpResponse& response, int errorCode);
};

#endif
