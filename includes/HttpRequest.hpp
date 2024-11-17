/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ecarlier <ecarlier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/08 16:28:34 by okrahl            #+#    #+#             */
/*   Updated: 2024/11/17 03:29:53 by ecarlier         ###   ########.fr       */
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

class HttpRequest
{
	private:
		std::string							method;
		std::string							url;
		std::string							httpVersion;
		std::string							host;
		std::string							body;
		std::string							_boundary;
		int									 port;
		std::vector<std::string>			filenames;
		std::vector<std::string>			fileContents;
		const ServerConfig&					_serverConfig;
		std::map<std::string, std::string>	headers;


		void	parse(const char* buffer, int bytesRead);
		void	parseMultipartData(const std::string& boundary);

	public:

		HttpRequest(const char* buffer, int bytesRead, const ServerConfig& serverConfig);

		const std::string&							getMethod() const;
		const std::string&							getUrl() const;
		const std::string&							getHttpVersion() const;
		const std::string&							getBody() const;
		const std::string&							getHost() const;
		const std::map<std::string, std::string>&	getHeaders() const;
		const std::vector<std::string>&				getFilenames() const;
		const std::vector<std::string>&				getFileContents() const;
		std::string									getHeader(const std::string& name) const;
		std::string									getQueryString() const;
		int											getPort() const;

		std::string extractFilename(const std::string& contentDisposition);

};

#endif
