#ifndef PARSER_HPP
#define PARSER_HPP

#include <sstream>
#include <string>
#include <fstream>
#include <iostream>
#include "Data.hpp"

std::stringstream readConfigFile(const std::string& filePath);
bool parseConfigStream(std::stringstream& buffer, Data& data);

#endif