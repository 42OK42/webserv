/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: okrahl <okrahl@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/14 16:10:56 by okrahl            #+#    #+#             */
/*   Updated: 2024/10/14 19:07:11 by okrahl           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <poll.h>
#include <errno.h>
#include <stdexcept>
#include <sys/stat.h>
#include <unistd.h>


class HttpRequest {
public:
    HttpRequest();

    const std::string& getMethod() const;
    const std::string& getUrl() const;
    const std::string& getHttpVersion() const;
    const std::map<std::string, std::string>& getHeaders() const; // Korrektur hier
    const std::string& getBody() const;
    const std::string& getFilename() const;
    std::vector<std::string> getFilenames() const;
    std::vector<std::string> getFileData() const;
    std::string getBoundary() const;

    std::string getHeader(const std::string& name) const;
    std::string getHost() const;
    std::string getUserAgent() const;
    std::string getConnection() const;
    std::string getAccept() const;

    bool isRequestComplete(const std::vector<char>& buffer, int total_bytes_read);
    bool isBodyComplete(const std::vector<char>& buffer, int total_bytes_read);
    int readChunk(int client_socket, std::vector<char>& buffer, int total_bytes_read);
    bool readRequest(int client_socket);
    void processBody(const std::string& upload_dir, const std::vector<char>& buffer, int total_bytes_read);
    void print() const;

private:
    std::string method;
    std::string url;
    std::string httpVersion;
    std::map<std::string, std::string> headers;
    std::string body;
    std::string filename;
    std::vector<std::string> filenames;
    std::vector<std::string> fileData;
    std::string boundary;
};

#endif // HTTPREQUEST_HPP