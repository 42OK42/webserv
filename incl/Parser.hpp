#ifndef PARSER_HPP
#define PARSER_HPP

#include <sstream>
#include <string>
#include <fstream>
#include <iostream>
#include "Data.hpp"

std::string removeSemicolon(const std::string& str);
bool readConfigFile(const std::string& filePath, std::stringstream& buffer);
bool parseConfigStream(std::stringstream& buffer, Data& data);
bool parseLocation(std::stringstream& buffer, LocationConf& location);
bool parseCGI(std::istringstream& iss, CGIConfig& cgi);

#endif