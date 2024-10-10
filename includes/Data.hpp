/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Data.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: okrahl <okrahl@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/02 15:09:10 by okrahl            #+#    #+#             */
/*   Updated: 2024/10/02 15:09:11 by okrahl           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef DATA_HPP
#define DATA_HPP

#include <string>
#include <vector>
#include <map>

struct CGIConfig
{
	std::string extension;
	std::string path;
};

struct LocationConf
{
	std::string path;
	std::string root;
	std::string index;
	std::vector<std::string> allowedMethods;
	bool autoindex;
	CGIConfig cgi;
};

struct ServerConf
{
	int listenPort;
	std::string serverName;
	std::map<int, std::string> errorPages;
	size_t clientMaxBodySize;
	std::vector<LocationConf> locations;
};

struct Data
{
	ServerConf serverConfig;
};

#endif