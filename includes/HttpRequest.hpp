/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: okrahl <okrahl@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/08 16:28:34 by okrahl            #+#    #+#             */
/*   Updated: 2024/11/05 16:35:47 by okrahl           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <iostream>

class HttpRequest {
public:
	HttpRequest(const char* buffer, int bytesRead);

	const std::string& getMethod() const;
	const std::string& getUrl() const;
	const std::string& getHttpVersion() const;
	const std::map<std::string, std::string>& getHeaders() const;
	const std::string& getBody() const;
	const std::vector<std::string>& getFilenames() const;

	std::string getHeader(const std::string& name) const;
	const std::string& getHost() const;
	int getPort() const;
	std::string getUserAgent() const;
	std::string getConnection() const;
	std::string getAccept() const;

	void print() const;

private:
	std::string method;
	std::string url;
	std::string httpVersion;
	std::map<std::string, std::string> headers;
	std::string body;
	std::vector<std::string> filenames;

	std::string host;
	int port;

	std::string _boundary;

	void parse(const char* buffer, int bytesRead);
	void parseMultipartData(const std::string& boundary);
};

#endif