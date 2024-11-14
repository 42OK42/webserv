/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ecarlier <ecarlier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/16 17:44:54 by okrahl            #+#    #+#             */
/*   Updated: 2024/11/14 20:53:00 by ecarlier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Router.hpp"
#include "Helper.hpp"
#include <sstream>
#include <ctime>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cerrno>
#include <vector>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>

Router::Router(ServerConfig& config) : _serverConfig(config) {}

Router::~Router() {}

/*
    Reads the content of a file and returns it as a string.

    @param filepath The path to the file to be read.
    @returns A string containing the file content. Returns a "404 Not Found" HTML message if the file cannot be opened.
*/
std::string Router::readFile(const std::string& filepath) {
	std::ifstream file(filepath.c_str(), std::ios::binary);
	if (!file.is_open()) {
		return "<html><body><h1>404 Not Found</h1></body></html>";
	}
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

/*
    Ensures that the directories leading up to a specified path exist, creating them if necessary.

    @param path The path where directories should be ensured to exist.
    @returns void
*/
void Router::ensureDirectoryExists(const std::string& path) {
	size_t pos = 0;
	do {
		pos = path.find('/', pos + 1);
		std::string subpath = path.substr(0, pos);
		if (!subpath.empty()) {
			mkdir(subpath.c_str(), 0777);
		}
	} while (pos != std::string::npos);
}

/*
    Extracts the filename from a URL query string.

    @param url The URL containing the filename query parameter.
    @returns The extracted filename as a string, or an empty string if no filename is found.
*/
std::string Router::extractFilenameFromUrl(const std::string& url) {
	size_t pos = url.find("filename=");
	if (pos != std::string::npos) {
		return url.substr(pos + 9);
	}
	return "";
}

/*
    Extracts the filename from a Content-Disposition header.

    @param contentDisposition The Content-Disposition header containing the filename.
    @returns The extracted filename as a string, or an empty string if the filename is not found.
*/
std::string Router::extractFilename(const std::string& contentDisposition) {
	std::string filename;
	size_t pos = contentDisposition.find("filename=");
	if (pos != std::string::npos) {
		pos += 10;
		size_t endPos = contentDisposition.find("\"", pos);
		if (endPos != std::string::npos) {
			filename = contentDisposition.substr(pos, endPos - pos);
		}
	}
	return filename;
}

/*
    Retrieves a list of files in the upload directory for each location.

    @param The path of the directory (not used in this implementation).
    @returns A vector of strings containing the paths to the files in the upload directories.
*/
std::vector<std::string> Router::getFilesInDirectory(const std::string&) {
	std::vector<std::string> files;
	const std::map<std::string, Location>& locations = _serverConfig.getLocations();

	for (std::map<std::string, Location>::const_iterator it = locations.begin(); it != locations.end(); ++it) {
		std::string uploadDir = it->second.getRoot() + "/uploads/";
		std::string locationPath = it->first;
		DIR* dir = opendir(uploadDir.c_str());
		if (dir) {
			struct dirent* entry;
			while ((entry = readdir(dir)) != NULL) {
				if (entry->d_type == DT_REG) {
					files.push_back(locationPath + "|" + it->second.getRoot() + "|" + entry->d_name);
				}
			}
			closedir(dir);
		}
	}
	return files;
}

/*
    Handles an incoming HTTP request and processes it based on the request's method and URL.

    This function checks if the request is directed to a CGI location, if the request method is allowed,
    and processes the request accordingly. It handles GET, POST, DELETE methods and returns the appropriate
    response for each.

    @param request The HTTP request object containing the request details such as method, headers, and URL.
    @param response The HTTP response object where the status code, headers, and body will be set.

    @returns void
*/
void Router::handleRequest(const HttpRequest& request, HttpResponse& response) {
	#ifdef DEBUG_MODE
	std::cout << "\033[0;33m[DEBUG] Request URL: " << request.getUrl() << "\033[0m" << std::endl;
	std::cout << "\033[0;33m[DEBUG] Request Method: " << request.getMethod() << "\033[0m" << std::endl;
	#endif

	std::string requestHost = request.getHeader("Host");
	std::ostringstream expectedHost;
	expectedHost << _serverConfig.getHost() << ":" << _serverConfig.getPort();

	if (requestHost != expectedHost.str()) {
		setErrorResponse(response, 400);
		return;
	}

	std::string path = request.getUrl();

	if (path.find("/cgi-bin/") == 0) {
		#ifdef DEBUG_MODE
		std::cout << "\033[0;33m[DEBUG] CGI Request detected for path: " << path << "\033[0m" << std::endl;
		#endif
		try {
			const Location& location = _serverConfig.findLocation("/cgi-bin");
			handleCGI(request, response, location);
			return;
		} catch (const ServerConfig::LocationNotFound& e) {
			#ifdef DEBUG_MODE
			std::cout << "\033[0;31m[DEBUG] CGI Location not found\033[0m" << std::endl;
			#endif
			setErrorResponse(response, 404);
			return;
		}
	}

	size_t queryPos = path.find('?');
	if (queryPos != std::string::npos) {
		path = path.substr(0, queryPos);
	}

	try {
		const Location& location = _serverConfig.findLocation(path);

		if (!location.isMethodAllowed(request.getMethod())) {
			setErrorResponse(response, 405);
			return;
		}

		if (request.getMethod() == "GET") {
			handleGET(request, response, location);
		}
		else if (request.getMethod() == "POST") {
			handlePOST(request, response, location);
		}
		else if (request.getMethod() == "DELETE") {
			handleDELETE(request, response, location);
		}
		else {
			setErrorResponse(response, 405);
		}
	} catch (const ServerConfig::LocationNotFound& e) {
		setErrorResponse(response, 404);
	}
}

/*
    Handles an HTTP GET request by fetching the requested resource and generating the appropriate response.

    This function processes the GET request in different ways:
    - If a query string indicates a file, it attempts to find and return the requested file from the server.
    - If the request is for an image file, it tries to find and return the file from the `/uploads/` directory.
    - If the request is for a directory, it may return the directory listing or an index file, depending on the location configuration.

    @param request The HTTP request object containing the request details such as method, headers, and URL.
    @param response The HTTP response object where the status code, headers, and body will be set.
    @param location The location object that contains configuration details for the path being accessed.

    @returns void
*/
void Router::handleGET(const HttpRequest& request, HttpResponse& response, const Location& location) {
	if (!location.isMethodAllowed("GET")) {
		setErrorResponse(response, 405);
		return;
	}

	std::string path = request.getUrl();
	size_t queryPos = path.find("?file=");

	if (queryPos != std::string::npos) {
		std::string filename = path.substr(queryPos + 6);
		std::string uploadPath = location.getRoot() + "/uploads/" + filename;


		struct stat statbuf;
		if (stat(uploadPath.c_str(), &statbuf) == 0 && S_ISREG(statbuf.st_mode)) {
			std::ifstream file(uploadPath.c_str(), std::ios::binary);
			if (file) {
				std::vector<char> buffer((std::istreambuf_iterator<char>(file)),
									   std::istreambuf_iterator<char>());

				response.setStatusCode(200);
				response.setBody(std::string(buffer.begin(), buffer.end()));

				std::string extension = filename.substr(filename.find_last_of(".") + 1);
				if (extension == "jpg" || extension == "jpeg")
					response.setHeader("Content-Type", "image/jpeg");
				else if (extension == "png")
					response.setHeader("Content-Type", "image/png");
				else if (extension == "gif")
					response.setHeader("Content-Type", "image/gif");
				return;
			}
		}
		setErrorResponse(response, 404);
		return;
	}

	if (!location.get_redirectTo().empty()) {
		response.setStatusCode(301);
		response.setHeader("Location", location.get_redirectTo());
		return;
	}

	std::string fullPath = location.getRoot();

	if (path.find("/uploads/") != std::string::npos) {
		std::string filename = path.substr(path.find_last_of("/") + 1);
		std::string uploadPath = location.getRoot() + "/uploads/" + filename;

		struct stat statbuf;
		if (stat(uploadPath.c_str(), &statbuf) == 0 && S_ISREG(statbuf.st_mode)) {
			std::ifstream file(uploadPath.c_str(), std::ios::binary);
			if (file) {
				std::vector<char> buffer((std::istreambuf_iterator<char>(file)),
									   std::istreambuf_iterator<char>());

				response.setStatusCode(200);
				response.setBody(std::string(buffer.begin(), buffer.end()));

				std::string extension = filename.substr(filename.find_last_of(".") + 1);
				if (extension == "jpg" || extension == "jpeg")
					response.setHeader("Content-Type", "image/jpeg");
				else if (extension == "png")
					response.setHeader("Content-Type", "image/png");
				else if (extension == "gif")
					response.setHeader("Content-Type", "image/gif");
				return;
			}
		}
		setErrorResponse(response, 404);
		return;
	}

	struct stat statbuf;
	if (stat(fullPath.c_str(), &statbuf) == 0) {
		if (S_ISDIR(statbuf.st_mode)) {
			if (!location.getIndex().empty()) {
				std::string indexPath = fullPath + "/" + location.getIndex();
				if (stat(indexPath.c_str(), &statbuf) == 0 && S_ISREG(statbuf.st_mode)) {
					std::string content = readFile(indexPath);

					if (request.getUrl() == "/uploadSuccessful") {
						std::vector<std::string> files = getFilesInDirectory("");
						std::string fileListJS = "\n<script>\nconst fileList = [";
						for (size_t i = 0; i < files.size(); ++i) {
							if (i > 0) fileListJS += ",";
							fileListJS += "\n    \"" + files[i] + "\"";
						}
						fileListJS += "\n];\n</script>\n</head>";

						size_t pos = content.find("</head>");
						if (pos != std::string::npos) {
							content.insert(pos, fileListJS);
						}
					}

					response.setStatusCode(200);
					response.setBody(content);
					response.setHeader("Content-Type", "text/html");
					return;
				}
			}
			if (location.getAutoIndex()) {
				response.setStatusCode(200);
				response.setBody(generateDirectoryListing(fullPath, request.getUrl()));
				response.setHeader("Content-Type", "text/html");
				return;
			}
		}
		else if (S_ISREG(statbuf.st_mode)) {
			response.setStatusCode(200);
			response.setBody(readFile(fullPath));
			response.setHeader("Content-Type", "text/html");
			return;
		}
	}
	setErrorResponse(response, 404);
}

/*
	Handles an HTTP POST request, typically used for receiving form submissions or file uploads.

    This function processes the POST request in two main scenarios:
    - If the request content type is `multipart/form-data`, it handles file uploads by saving files to the server's `/uploads/` directory.
    - If the request body contains plain text or data, it simply echoes the received data back in the response.

    @param request The HTTP request object containing the request details such as method, headers, and body.
    @param response The HTTP response object where the status code, headers, and body will be set.
    @param location The location object that contains configuration details for the path being accessed.

    @returns void
*/
void Router::handlePOST(const HttpRequest& request, HttpResponse& response, const Location& location) {
	std::string contentType = request.getHeader("Content-Type");

	if (contentType.find("multipart/form-data") != std::string::npos) {
		std::string uploadDir = location.getRoot() + "/uploads/";
		ensureDirectoryExists(uploadDir);

		const std::vector<std::string>& filenames = request.getFilenames();
		const std::vector<std::string>& fileContents = request.getFileContents();

		if (!filenames.empty() && filenames.size() == fileContents.size()) {
			for (size_t i = 0; i < filenames.size(); ++i) {
				std::string fullPath = uploadDir + filenames[i];
				std::ofstream outFile(fullPath.c_str(), std::ios::binary);
				if (outFile.is_open()) {
					outFile.write(fileContents[i].c_str(), fileContents[i].length());
					outFile.close();
				}
			}

			response.setStatusCode(303);
			response.setHeader("Location", "/uploadSuccessful");
			response.setHeader("Content-Type", "text/html");
			response.setBody("<html><body>Redirecting to /uploadSuccessful</body></html>");
		} else {
			setErrorResponse(response, 400);
		}
	} else {
		std::string body = request.getBody();
		if (!body.empty()) {
			response.setStatusCode(200);
			response.setHeader("Content-Type", "text/plain");
			response.setBody("Received: " + body);
		} else {
			setErrorResponse(response, 400);
			response.setBody("No data received");
		}
	}
}

/*
	Handles an HTTP DELETE request by attempting to delete the specified file on the server.

    This function:
    - Checks if the DELETE method is allowed for the requested location.
    - Attempts to extract the filename from the URL and delete the corresponding file from the server's `/uploads/` directory.
    - Returns a success message if the file is deleted, or an error message if the file is not found or there is an issue with deletion.

    @param request The HTTP request object containing the request details such as method, headers, and URL.
    @param response The HTTP response object where the status code, headers, and body will be set.
    @param location The location object that contains configuration details for the path being accessed.

    @returns void
*/
void Router::handleDELETE(const HttpRequest& request, HttpResponse& response, const Location& location) {
	if (!location.isMethodAllowed("DELETE")) {
		setErrorResponse(response, 405);
		return;
	}

	std::string filename = extractFilenameFromUrl(request.getUrl());
	if (!filename.empty()) {
		size_t separatorPos = filename.find("|");
		if (separatorPos != std::string::npos) {
			std::string root = filename.substr(0, separatorPos);
			std::string file = filename.substr(separatorPos + 1);
			std::string fullPath = root + "/uploads/" + file;

			struct stat statbuf;
			if (stat(fullPath.c_str(), &statbuf) != 0) {
				response.setStatusCode(204);
				response.setBody("File not found");
				response.setHeader("Content-Type", "text/plain");
				return;
			}

			if (remove(fullPath.c_str()) == 0) {
				response.setStatusCode(200);
				response.setBody("File deleted successfully");
			} else {
				response.setStatusCode(500);
				response.setBody("Error deleting file");
			}
		}
	} else {
		response.setStatusCode(400);
		response.setBody("No filename provided");
	}
	response.setHeader("Content-Type", "text/plain");
}


/*
	Sets an error response with the given status code and an optional custom error page.

    This function:
    - Checks if a custom error page is configured for the given error code in the server's configuration.
    - If a custom error page exists, it reads the content and sets it as the response body.
    - If no custom error page exists, it sets a default empty body and the error status code.

    @param response The HTTP response object where the status code, headers, and body will be set.
    @param errorCode The error code to set in the response (e.g., 404 for "Not Found", 500 for "Internal Server Error").

    @returns void
*/
void Router::setErrorResponse(HttpResponse& response, int errorCode) {
	const std::map<int, std::string>& errorPages = _serverConfig.getErrorPages();
	std::map<int, std::string>::const_iterator errorPageIt = errorPages.find(errorCode);

	if (errorPageIt != errorPages.end()) {
		std::string errorPageContent = readFile(errorPageIt->second);
		response.setStatusCode(errorCode);
		response.setBody(errorPageContent);
		response.setHeader("Content-Type", "text/html");
		response.setHeader("Error-Page-Path", errorPageIt->second);
	} else {
		response.setStatusCode(errorCode);
		response.setBody("");
		response.setHeader("Content-Type", "text/html");
	}
}

/*
Generates an HTML directory listing for the specified directory.

    This function:
    - Scans the specified directory for files and subdirectories.
    - Sorts the entries alphabetically.
    - Generates an HTML page displaying the name, last modified time, and size of each entry.
    - Creates links to each entry (directories will have a trailing slash).
    - Returns the generated HTML as a string.

    @param dirPath The path to the directory whose contents are to be listed.
    @param requestPath The original URL path that was requested, used to create relative links.

    @returns A string containing the HTML for the directory listing.
*/
std::string Router::generateDirectoryListing(const std::string& dirPath, const std::string& requestPath) {
	std::stringstream html;
	html << "<!DOCTYPE html>\n<html>\n<head>\n"
		 << "<title>Index of " << requestPath << "</title>\n"
		 << "<style>\n"
		 << "body { font-family: Arial, sans-serif; margin: 20px; }\n"
		 << "table { border-collapse: collapse; width: 100%; }\n"
		 << "th, td { text-align: left; padding: 8px; border-bottom: 1px solid #ddd; }\n"
		 << "th { background-color: #f2f2f2; }\n"
		 << "tr:hover { background-color: #f5f5f5; }\n"
		 << "a { text-decoration: none; color: #0366d6; }\n"
		 << "a:hover { text-decoration: underline; }\n"
		 << "</style>\n"
		 << "</head>\n<body>\n"
		 << "<h1>Index of " << requestPath << "</h1>\n"
		 << "<table>\n"
		 << "<tr><th>Name</th><th>Last Modified</th><th>Size</th></tr>\n";

	DIR* dir = opendir(dirPath.c_str());
	if (dir) {
		struct dirent* entry;
		std::vector<std::string> entries;

		while ((entry = readdir(dir)) != NULL) {
			entries.push_back(entry->d_name);
		}

		std::sort(entries.begin(), entries.end());

		for (std::vector<std::string>::const_iterator it = entries.begin();
			 it != entries.end(); ++it) {
			std::string name = *it;
			struct stat statbuf;
			std::string fullPath = dirPath + "/" + name;

			if (stat(fullPath.c_str(), &statbuf) == 0) {
				bool isDir = S_ISDIR(statbuf.st_mode);

				std::ostringstream size;
				if (isDir)
					size << "-";
				else
					size << statbuf.st_size;

				char timeStr[80];
				strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S",
						localtime(&statbuf.st_mtime));

				html << "<tr><td><a href=\""
					 << (requestPath == "/" ? "" : requestPath) << "/" << name
					 << (isDir ? "/" : "") << "\">" << name
					 << (isDir ? "/" : "") << "</a></td>"
					 << "<td>" << timeStr << "</td>"
					 << "<td>" << size.str() << "</td></tr>\n";
			}
		}
		closedir(dir);
	} else {
		html << "<tr><td colspan='3'>Error reading directory</td></tr>\n";
	}

	html << "</table>\n</body>\n</html>";
	return html.str();
}

/*
	Returns the current timestamp formatted as "YYYYMMDD_HHMMSS".

    @returns A string containing the current timestamp.
*/
std::string Router::getCurrentTimestamp() const {
	std::time_t now = std::time(NULL);
	char buffer[20];
	std::strftime(buffer, sizeof(buffer), "%Y%m%d_%H%M%S", std::localtime(&now));
	return std::string(buffer);
}


/*
	Handles an HTTP request for a CGI (Common Gateway Interface) script.

    @param request The HTTP request object containing the request details such as method, headers, and URL.
    @param response The HTTP response object where the status code, headers, and body will be set.
    @param location The location object that contains configuration details for the path being accessed.

    @returns void
*/
void Router::handleCGI(const HttpRequest& request, HttpResponse& response, const Location& location) {
    if (!isCgiEnabled(location)) {
        setErrorResponse(response, 403);
        return;
    }

    std::string scriptPath = constructScriptPath(request, location);

    int input_pipe[2];
    int output_pipe[2];

    if (!createPipes(input_pipe, output_pipe)) {
        setErrorResponse(response, 500);
        return;
    }

    pid_t pid = createFork(input_pipe, output_pipe, response);

    if (pid == 0) {
        executeCgi(request, input_pipe, output_pipe, location, scriptPath);
    } else {
        handleParentProcess(request, response, input_pipe, output_pipe, pid);
    }
}

/*
	Checks whether CGI is enabled for the specified location.

    @param location The location object that contains the configuration details for the path being accessed.

    @returns A boolean indicating whether CGI is enabled for the specified location.
*/
bool Router::isCgiEnabled(const Location& location) {
    #ifdef DEBUG_MODE
    std::cout << "[DEBUG] CGI Location settings:" << std::endl;
    std::cout << "[DEBUG] CGI Enabled: " << location.isCgiEnabled() << std::endl;
    std::cout << "[DEBUG] CGI Path: " << location.getCgiBin() << std::endl;
    #endif

    return location.isCgiEnabled();
}

/*
	Constructs the full script path for a CGI request.

    @param request The HTTP request object containing the request details such as method, headers, and URL.
    @param location The location object that contains configuration details for the path being accessed.

    @returns A string containing the full script path for the CGI request.
*/
std::string Router::constructScriptPath(const HttpRequest& request, const Location& location) {
    std::string scriptPath = request.getUrl();
    if (scriptPath.find("/cgi-bin/") == 0) {
        scriptPath = location.getRoot() + scriptPath;
    }
    return scriptPath;
}


/*
	Creates two pipes for inter-process communication.

    @param input_pipe
    @param output_pipe

    @returns A boolean indicating whether both pipes were successfully created.
*/
bool Router::createPipes(int input_pipe[2], int output_pipe[2]) {
    if (pipe(input_pipe) < 0 || pipe(output_pipe) < 0) {
        return false;
    }
    return true;
}

/*
	Creates a new process by forking the current process.

    @param input_pipe .
    @param output_pipe
    @param response The HTTP response object to handle error responses in case of failure.

    @returns The PID of the child process, or -1 if forking fails.
*/
pid_t Router::createFork(int input_pipe[2], int output_pipe[2], HttpResponse& response) {
    pid_t pid = fork();
    if (pid < 0) {
        close(input_pipe[0]);
        close(input_pipe[1]);
        close(output_pipe[0]);
        close(output_pipe[1]);
        setErrorResponse(response, 500);
    }
    return pid;
}

/*
	Executes a CGI script by redirecting input/output through pipes.

    @param request The HTTP request object
    @param input_pipe
    @param output_pipe .
    @param location The location object containing the path to the CGI binary.
    @param scriptPath The full path to the CGI script that needs to be executed.

    @returns void
*/
void Router::executeCgi(const HttpRequest& request, int input_pipe[2], int output_pipe[2], const Location& location, const std::string& scriptPath) {
    #ifdef DEBUG_MODE
    std::cout << "[DEBUG-CHILD] Executing CGI script: " << location.getCgiBin() << std::endl;
    std::cout << "[DEBUG-CHILD] Script path: " << scriptPath << std::endl;
    #endif

    std::ostringstream oss;
    oss << request.getBody().length();
    setenv("CONTENT_LENGTH", oss.str().c_str(), 1);
    setenv("CONTENT_TYPE", request.getHeader("Content-Type").c_str(), 1);
    setenv("REQUEST_METHOD", request.getMethod().c_str(), 1);
    setenv("QUERY_STRING", "", 1);

    close(input_pipe[1]);
    close(output_pipe[0]);

    if (dup2(input_pipe[0], STDIN_FILENO) == -1) {
        perror("[ERROR-CHILD] dup2 input failed");
        exit(1);
    }

    if (dup2(output_pipe[1], STDOUT_FILENO) == -1) {
        perror("[ERROR-CHILD] dup2 output failed");
        exit(1);
    }

    close(input_pipe[0]);
    close(output_pipe[1]);

    char* const args[] = {
        const_cast<char*>(location.getCgiBin().c_str()),
        const_cast<char*>(scriptPath.c_str()),
        NULL
    };

    execv(location.getCgiBin().c_str(), args);
    perror("[ERROR-CHILD] execv failed");
    exit(1);
}

/*
	Handles the parent process after forking for CGI execution.

    This function:
    - Writes the request body to the input pipe (either chunked or non-chunked).
    - Reads the output from the CGI script via the output pipe.
    - Waits for the child process to finish and sets the HTTP response based on the CGI output.

    @param request The HTTP request object containing the request details.
    @param response The HTTP response object to set the response headers, status, and body.
    @param input_pipe
    @param output_pipe
    @param pid The PID of the child process to wait for.

    @returns void
*/
void Router::handleParentProcess(const HttpRequest& request, HttpResponse& response, int input_pipe[2], int output_pipe[2], pid_t pid) {
    close(input_pipe[0]);
    close(output_pipe[1]);

    if (request.getHeader("Transfer-Encoding") == "chunked") {
        std::string decoded_body = decodeChunkedBody(request.getBody());

        ssize_t written = write(input_pipe[1], decoded_body.c_str(), decoded_body.length());
        if (written != static_cast<ssize_t>(decoded_body.length())) {
            perror("[ERROR-PARENT] Write failed or incomplete");
            close(input_pipe[1]);
            setErrorResponse(response, 500);
            return;
        }
        fsync(input_pipe[1]);
    } else {
        if (!request.getBody().empty()) {
            ssize_t written = write(input_pipe[1], request.getBody().c_str(), request.getBody().length());
            if (written != static_cast<ssize_t>(request.getBody().length())) {
                perror("[ERROR-PARENT] Write failed or incomplete");
                close(input_pipe[1]);
                setErrorResponse(response, 500);
                return;
            }
            fsync(input_pipe[1]);
        }
    }

    close(input_pipe[1]);

    std::string cgi_output;
    char buffer[4096];
    ssize_t bytes_read;

    while ((bytes_read = read(output_pipe[0], buffer, sizeof(buffer))) > 0) {
        cgi_output.append(buffer, bytes_read);
    }
    close(output_pipe[0]);

    int status;
    waitpid(pid, &status, 0);

    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        response.setStatusCode(200);
        response.setBody(cgi_output);
        response.setHeader("Content-Type", "text/html");
    } else {
        setErrorResponse(response, 500);
    }
}

/* Decodes a chunked transfer-encoded HTTP body.

    @param body The chunked HTTP body to decode.

    @returns The decoded body as a string.
*/
std::string Router::decodeChunkedBody(const std::string& body)
{
    std::string decoded_body;
    size_t pos = 0;
    while (pos < body.size()) {
        size_t chunk_size_end = body.find("\r\n", pos);
        if (chunk_size_end == std::string::npos) {
            break;
        }

        std::string chunk_size_str = body.substr(pos, chunk_size_end - pos);
        long chunk_size = strtol(chunk_size_str.c_str(), NULL, 16);

        pos = chunk_size_end + 2;

        if (pos + chunk_size > body.size()) {
            break;
        }

        decoded_body.append(body.substr(pos, chunk_size));
        pos += chunk_size + 2;
    }

    return decoded_body;
}
