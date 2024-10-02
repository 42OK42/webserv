/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: okrahl <okrahl@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/02 18:05:09 by okrahl            #+#    #+#             */
/*   Updated: 2024/10/02 18:06:08 by okrahl           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpResponse.hpp"
#include <sstream> // Für stringstream

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
	
	// Verwende stringstream, um die Länge in einen String zu konvertieren
	std::stringstream ss;
	ss << body.size();
	setHeader("Content-Length", ss.str());
}

std::string HttpResponse::toString() const
{
	std::ostringstream response;
	response << version << " " << statusCode << " " << statusMessage << "\r\n";
	
	// Verwende herkömmliche Schleife
	for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
		response << it->first << ": " << it->second << "\r\n";
	
	response << "\r\n" << body;
	return response.str();
}