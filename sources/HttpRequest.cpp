/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: okrahl <okrahl@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/08 15:49:27 by okrahl            #+#    #+#             */
/*   Updated: 2024/10/14 19:13:09 by okrahl           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequest.hpp"
#include "Helper.hpp" // Für die Funktionen extractFilename, generateUniqueFilename, ensureDirectoryExists

HttpRequest::HttpRequest() : filename("") {}

const std::string& HttpRequest::getMethod() const { return method; }
const std::string& HttpRequest::getUrl() const { return url; }
const std::string& HttpRequest::getHttpVersion() const { return httpVersion; }
const std::map<std::string, std::string>& HttpRequest::getHeaders() const { return headers; }
const std::string& HttpRequest::getBody() const { return body; }
const std::string& HttpRequest::getFilename() const { return filename; }
std::vector<std::string> HttpRequest::getFilenames() const { return filenames; }
std::vector<std::string> HttpRequest::getFileData() const { return fileData; }
std::string HttpRequest::getBoundary() const { return boundary; }

std::string HttpRequest::getHeader(const std::string& name) const {
    std::map<std::string, std::string>::const_iterator it = headers.find(name);
    if (it != headers.end()) {
        return it->second;
    }
    return "";
}

std::string HttpRequest::getHost() const {
    return getHeader("Host");
}

std::string HttpRequest::getUserAgent() const {
    return getHeader("User-Agent");
}

std::string HttpRequest::getConnection() const {
    return getHeader("Connection");
}

std::string HttpRequest::getAccept() const {
    return getHeader("Accept");
}

bool HttpRequest::isRequestComplete(const std::vector<char>& buffer, int total_bytes_read) {
    std::string headers(buffer.begin(), buffer.begin() + total_bytes_read);
    size_t pos = headers.find("\r\n\r\n");
    return pos != std::string::npos;
}

bool HttpRequest::isBodyComplete(const std::vector<char>& buffer, int total_bytes_read) {
    std::string headers(buffer.begin(), buffer.begin() + total_bytes_read);
    size_t content_length_pos = headers.find("Content-Length: ");
    if (content_length_pos != std::string::npos) {
        content_length_pos += 16;
        size_t end_pos = headers.find("\r\n", content_length_pos);
        std::string content_length_str = headers.substr(content_length_pos, end_pos - content_length_pos);

        std::stringstream ss(content_length_str);
        int content_length;
        if (!(ss >> content_length)) {
            std::cerr << "Error parsing Content-Length: " << content_length_str << std::endl;
            return false;
        }

        size_t pos = headers.find("\r\n\r\n");
        if (total_bytes_read >= static_cast<int>(pos + 4 + content_length)) {
            return true;
        }
    }
    return false;
}

int HttpRequest::readChunk(int client_socket, std::vector<char>& buffer, int total_bytes_read) {
    int bytes_read = recv(client_socket, &buffer[total_bytes_read], buffer.size() - total_bytes_read, 0);
    if (bytes_read < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return 0; // No data read, continue polling
        } else {
            throw std::runtime_error("Failed to read from client");
        }
    }
    if (bytes_read == 0) {
        return -1; // Connection closed by client
    }
    return bytes_read;
}

bool HttpRequest::readRequest(int client_socket) {
    std::vector<char> buffer(8192);
    int total_bytes_read = 0;

    // Ensure the uploads directory exists
    const std::string upload_dir = "uploads";
    ensureDirectoryExists(upload_dir);

    while (true) {
        // Perform polling before reading chunks
        struct pollfd pfd;
        pfd.fd = client_socket;
        pfd.events = POLLIN;

        int poll_result = poll(&pfd, 1, -1); // Wait indefinitely for an event
        if (poll_result < 0) {
            perror("poll");
            return false;
        }

        if (pfd.revents & POLLIN) {
            int bytes_read = readChunk(client_socket, buffer, total_bytes_read);
            if (bytes_read < 0) {
                std::cerr << "Connection closed by client." << std::endl;
                break; // Connection closed by client
            }
            total_bytes_read += bytes_read;

            std::cout << "Bytes read: " << bytes_read << ", Total bytes read: " << total_bytes_read << std::endl;

            // If the buffer is full, increase its size
            if (total_bytes_read == static_cast<int>(buffer.size())) {
                buffer.resize(buffer.size() + 8192); // Increase buffer size
                std::cout << "Buffer resized to: " << buffer.size() << std::endl;
            }

            // Check if the headers are complete
            if (isRequestComplete(buffer, total_bytes_read)) {
                std::string headers(buffer.begin(), buffer.begin() + total_bytes_read);
                std::cout << "Headers:\n" << headers << std::endl;

                std::istringstream requestStream(headers);
                std::string requestLine;
                std::getline(requestStream, requestLine);
                std::istringstream lineStream(requestLine);
                lineStream >> method >> url >> httpVersion;

                if (method == "GET" || method == "DELETE") {
                    std::cout << method << " request detected, no body expected." << std::endl;
                    std::cout << "Parsed Request Line - Method: " << method << ", URL: " << url << ", HTTP Version: " << httpVersion << std::endl;
                    return true;
                }

                // For POST requests, read and process the body
                if (method == "POST") {
                    size_t boundary_pos = headers.find("boundary=");
                    if (boundary_pos != std::string::npos) {
                        boundary = headers.substr(boundary_pos + 9);
                        std::cout << "Extracted boundary: " << boundary << std::endl;
                    } else {
                        std::cerr << "Boundary not found in headers" << std::endl;
                        return false;
                    }

                    while (true) {
                        int bytes_read = readChunk(client_socket, buffer, total_bytes_read);
                        if (bytes_read < 0) {
                            std::cerr << "Connection closed by client." << std::endl;
                            break; // Connection closed by client
                        }
                        total_bytes_read += bytes_read;

                        std::cout << "Bytes read: " << bytes_read << ", Total bytes read: " << total_bytes_read << std::endl;

                        // If the buffer is full, increase its size
                        if (total_bytes_read == static_cast<int>(buffer.size())) {
                            buffer.resize(buffer.size() + 8192); // Increase buffer size
                            std::cout << "Buffer resized to: " << buffer.size() << std::endl;
                        }

                        // Check if the body is complete
                        if (isBodyComplete(buffer, total_bytes_read)) {
                            std::cout << "Request body is complete." << std::endl;

                            // Process the entire body
                            processBody(upload_dir, buffer, total_bytes_read);

                            // Stop further processing
                            return true;
                        }
                    }
                }
            }
        }
    }

    // This part will not be reached due to the return statement above
    return false;
}

void HttpRequest::processBody(const std::string& upload_dir, const std::vector<char>& buffer, int total_bytes_read) {
    // Find and process parts in the body
    std::string delimiter = "--" + boundary;
    size_t pos = 0;
    size_t end = std::string(buffer.begin(), buffer.begin() + total_bytes_read).find(delimiter, pos);

    std::cout << "Processing body with size: " << total_bytes_read << std::endl;

    while (end != std::string::npos) {
        size_t start = pos + delimiter.length() + 2; // Skip the delimiter and CRLF
        pos = end + delimiter.length();
        end = std::string(buffer.begin(), buffer.begin() + total_bytes_read).find(delimiter, pos);

        std::string part(buffer.begin() + start, buffer.begin() + end);

        size_t headerEnd = part.find("\r\n\r\n");
        if (headerEnd != std::string::npos) {
            std::string headers(part.begin(), part.begin() + headerEnd);
            std::vector<char> content(part.begin() + headerEnd + 4, part.end());

            std::cout << "Part headers:\n" << headers << std::endl;
            std::cout << "Part content size: " << content.size() << std::endl;

            std::istringstream headerStream(headers);
            std::string line;
            std::string extractedFilename;
            while (std::getline(headerStream, line) && line != "\r") {
                if (!line.empty() && line[line.size() - 1] == '\r') {
                    line.erase(line.size() - 1);
                }
                std::string key, value;
                std::istringstream headerLine(line);
                if (std::getline(headerLine, key, ':') && std::getline(headerLine, value)) {
                    if (key == "Content-Disposition") {
                        std::cout << "Content-Disposition header: " << value << std::endl;
                        extractedFilename = extractFilename(value); // Speichern des Dateinamens
                    }
                }
            }

            std::cout << "Extracted filename: " << extractedFilename << std::endl;

            if (!extractedFilename.empty()) {
                std::string uniqueFilename = generateUniqueFilename(upload_dir, extractedFilename, filenames);
                std::cout << "Unique filename: " << uniqueFilename << std::endl;
                filenames.push_back(uniqueFilename);
                fileData.push_back(std::string(content.begin(), content.end()));

                // Save file to uploads directory
                std::ofstream outFile((upload_dir + "/" + uniqueFilename).c_str(), std::ios::binary);
                if (!outFile) {
                    std::cerr << "Failed to open file: " << uniqueFilename << std::endl;
                } else {
                    outFile.write(content.data(), content.size());
                    outFile.close();
                    std::cout << "File saved: " << uniqueFilename << std::endl;
                }
            } else {
                std::cerr << "Extracted filename is empty" << std::endl;
            }
        } else {
            std::cerr << "Header end not found in part" << std::endl;
            std::cerr << "Part content: " << std::string(part.begin(), part.end()) << std::endl; // Print the part content for debugging
        }

        // Move to the next part
        pos = end + delimiter.length();
        end = std::string(buffer.begin(), buffer.begin() + total_bytes_read).find(delimiter, pos);
    }
}

void HttpRequest::print() const {
    std::cout << "Method: " << method << "\n";
    std::cout << "URL: " << url << "\n";
    std::cout << "HTTP Version: " << httpVersion << "\n";
    std::cout << "Headers:\n";
    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
        std::cout << "  " << it->first << ": " << it->second << "\n";
    }
    if (!body.empty()) {
        // std::cout << "Body (first 100 chars): " << body.substr(0, 100) << "\n";
        if (body.size() > 100) {
            std::cout << "  (truncated, total size: " << body.size() << " bytes)\n";
        }
    }
    if (!filename.empty()) {
        std::cout << "Filename: " << filename << "\n";
    }
}