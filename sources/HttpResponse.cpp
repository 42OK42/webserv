/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: okrahl <okrahl@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/02 18:05:09 by okrahl            #+#    #+#             */
/*   Updated: 2024/10/16 18:23:42 by okrahl           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpResponse.hpp"

HttpResponse::HttpResponse(const HttpRequest& request) 
	: version(request.getHttpVersion()), statusCode(200), statusMessage("OK") {
}

void HttpResponse::setStatusCode(int code)
{
	statusCode = code;
	setStatusMessage(code);
}

void HttpResponse::setStatusMessage(int code)
{
	switch (code)
	{
		case 200: statusMessage = "OK"; break;
		case 404: statusMessage = "Not Found"; break;
		case 500: statusMessage = "Internal Server Error"; break;
		default: statusMessage = "Unknown Status"; break;
	}
}

void HttpResponse::setHeader(const std::string& key, const std::string& value)
{
	headers[key] = value;
}

void HttpResponse::setBody(const std::string& body)
{
	this->body = body;

	std::stringstream ss;
	ss << body.size();
	setHeader("Content-Length", ss.str());
}

std::string HttpResponse::toString() const
{
	std::ostringstream response;
	response << version << " " << statusCode << " " << statusMessage << "\r\n";
	
	for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
		response << it->first << ": " << it->second << "\r\n";
	
	response << "\r\n" << body;
	return response.str();
}

void HttpResponse::printResponse() const
{
	std::cout << "HTTP Version: " << version << std::endl;
	std::cout << "Status Code: " << statusCode << std::endl;
	std::cout << "Status Message: " << statusMessage << std::endl;
	std::cout << "Headers: " << std::endl;
	for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
		std::cout << it->first << ": " << it->second << std::endl;
	}
	std::cout << "Body: " << std::endl << body << std::endl;
}