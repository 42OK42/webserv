/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ecarlier <ecarlier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/02 18:05:09 by okrahl            #+#    #+#             */
/*   Updated: 2024/11/17 03:41:56 by ecarlier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpResponse.hpp"

HttpResponse::HttpResponse()
	: version("HTTP/1.1"), statusCode(200), statusMessage("OK") {
}

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
		case 201: statusMessage = "Created"; break;
		case 204: statusMessage = "No Content"; break;
		case 301: statusMessage = "Moved Permanently"; break;
		case 303: statusMessage = "See Other" ; break;
		case 400: statusMessage = "Bad Request"; break;
		case 403: statusMessage = "Forbidden"; break;
		case 404: statusMessage = "Not Found"; break;
		case 405: statusMessage = "Method Not Allowed"; break;
		case 413: statusMessage = "Payload Too Large"; break;
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

	// Convert body size to string for Content-Length header
	std::stringstream ss;
	ss << body.size();
	setHeader("Content-Length", ss.str());
}

std::string HttpResponse::toString() const
{
	std::ostringstream response;
	response << version << " " << statusCode << " " << statusMessage << "\r\n";

	for (std::map<std::string, std::string>::const_iterator it = headers.begin();
		 it != headers.end(); ++it) {
		response << it->first << ": " << it->second << "\r\n";
	}
	response << "\r\n" << body;
	return response.str();
}
