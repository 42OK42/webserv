/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ecarlier <ecarlier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/08 16:28:34 by okrahl            #+#    #+#             */
/*   Updated: 2024/11/14 23:40:50 by ecarlier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <iostream>
#include "ServerConfig.hpp"

class HttpRequest {
public:
	HttpRequest(const char* buffer, int bytesRead, const ServerConfig& serverConfig);

	const std::string& getMethod() const;
	const std::string& getUrl() const;
	const std::string& getHttpVersion() const;
	const std::map<std::string, std::string>& getHeaders() const;
	const std::string& getBody() const;
	const std::vector<std::string>& getFilenames() const;
	const std::vector<std::string>& getFileContents() const;

	std::string getHeader(const std::string& name) const;
	const std::string& getHost() const;
	int getPort() const;


private:
	std::string method;
	std::string url;
	std::string httpVersion;
	std::map<std::string, std::string> headers;
	std::string body;
	std::vector<std::string> filenames;
	std::vector<std::string> fileContents;

	std::string host;
	int port;

	std::string _boundary;
	const ServerConfig& _serverConfig;

	void parse(const char* buffer, int bytesRead);
	void parseMultipartData(const std::string& boundary);
};

#endif
