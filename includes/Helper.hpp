/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Helper.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: okrahl <okrahl@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/08 13:34:41 by okrahl            #+#    #+#             */
/*   Updated: 2024/10/10 18:01:13 by okrahl           ###   ########.fr       */
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
#include <vector>
#include <algorithm>
#include <pthread.h>

void ensureDirectoryExists(const std::string& path);
std::string readFile(const std::string& filePath);
std::string extractFilename(const std::string& contentDisposition);
std::string extractFilenameFromUrl(const std::string& url);
std::string generateUniqueFilename(const std::string& uploadDir, const std::string& baseName, std::vector<std::string>& uploadedFiles);

#endif