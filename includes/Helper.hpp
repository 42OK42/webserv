/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Helper.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: okrahl <okrahl@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/08 13:34:41 by okrahl            #+#    #+#             */
/*   Updated: 2024/10/16 17:27:17 by okrahl           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HELPER_HPP
#define HELPER_HPP

#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <sys/statvfs.h>

void ensureDirectoryExists(const std::string& path);
std::string readFile(const std::string& filePath);
std::string extractFilename(const std::string& contentDisposition);
std::string extractFilenameFromUrl(const std::string& url);
unsigned long long getAvailableSpace(const std::string& path);


#endif