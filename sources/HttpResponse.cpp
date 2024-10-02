/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: okrahl <okrahl@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/02 17:17:40 by okrahl            #+#    #+#             */
/*   Updated: 2024/10/02 17:18:00 by okrahl           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpResponse.hpp"

HttpResponse::HttpResponse(const HttpRequest& request) 
	: version(request.getHttpVersion()), statusCode(200), statusMessage("OK")
{
	
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
	setHeader("Content-Length", std::to_string(body.size()));
}

std::string HttpResponse::toString() const
{
	std::ostringstream response;
	response << version << " " << statusCode << " " << statusMessage << "\r\n";
	for (const auto& header : headers)
		response << header.first << ": " << header.second << "\r\n";
	response << "\r\n" << body;
	return response.str();
}