/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ecarlier <ecarlier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/08 14:54:59 by okrahl            #+#    #+#             */
/*   Updated: 2024/11/14 21:20:29 by ecarlier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ROUTER_HPP
#define ROUTER_HPP

#include "CommonHeader.hpp"

class HttpRequest;
class HttpResponse;

class Router
{
	private:

		ServerConfig& _serverConfig;

		std::string	readFile(const std::string& filepath);
		std::string	generateDirectoryListing(const std::string& dirPath, const std::string& requestPath);
		std::string	getCurrentTimestamp() const;
		std::string	extractFilenameFromUrl(const std::string& url);
		std::string	extractFilename(const std::string& contentDisposition);
		std::string	constructScriptPath(const HttpRequest& request, const Location& location);
		std::string	decodeChunkedBody(const std::string& body);

		void		ensureDirectoryExists(const std::string& path);
		void		executeCgi(const HttpRequest& request, int input_pipe[2], int output_pipe[2], const Location& location, const std::string& scriptPath);
		void		handleParentProcess(const HttpRequest& request, HttpResponse& response, int input_pipe[2], int output_pipe[2], pid_t pid);
		void		handleGET(const HttpRequest& request, HttpResponse& response, const Location& location);
		void		handlePOST(const HttpRequest& request, HttpResponse& response, const Location& location);
		void		handleDELETE(const HttpRequest& request, HttpResponse& response, const Location& location);
		void		handleCGI(const HttpRequest& request, HttpResponse& response, const Location& location);

		bool		isCgiEnabled(const Location& location);
		bool		createPipes(int input_pipe[2], int output_pipe[2]) ;

		pid_t		createFork(int input_pipe[2], int output_pipe[2], HttpResponse& response);

		std::vector<std::string> getFilesInDirectory(const std::string& directory);

	public:
		Router(ServerConfig& config);
		~Router();

		void		handleRequest(const HttpRequest& request, HttpResponse& response);
		void		setErrorResponse(HttpResponse& response, int errorCode);
};

#endif
