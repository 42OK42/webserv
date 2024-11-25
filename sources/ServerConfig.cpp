/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ecarlier <ecarlier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/18 17:04:09 by okrahl            #+#    #+#             */
/*   Updated: 2024/11/25 16:56:32 by ecarlier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerConfig.hpp"
#include "Router.hpp"
#include "HttpResponse.hpp"
#include "HttpRequest.hpp"


ServerConfig::ServerConfig() {}

/*
	Destructor for the ServerConfig class. Closes the socket (`m_socket`) if it is open
	(i.e., not equal to -1) to release system resources.

	@returns void
*/
ServerConfig::~ServerConfig() {
	if (m_socket != -1) {
		close(m_socket);
	}
}

/*
	Sets the specified socket to non-blocking mode.

	@param sockfd: The socket file descriptor to be set to non-blocking mode.
	@returns: 0 on success, -1 on failure.
*/
int set_nonblocking(int sockfd) {
	int flags = fcntl(sockfd, F_GETFL, 0);
	if (flags == -1) {
		return -1;
	}
	return fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
}

/*
	Sets the send and receive timeout for the specified socket.

	@param sockfd: The socket file descriptor to set the timeout for.
	@param timeout_seconds: The timeout value in seconds for both sending and receiving operations.
*/
void ServerConfig::set_socket_timeout(int sockfd, int timeout_seconds) {
	struct timeval timeout;
	timeout.tv_sec = timeout_seconds;
	timeout.tv_usec = 0;

	if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
		perror("setsockopt SO_RCVTIMEO");
	}
	if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0) {
		perror("setsockopt SO_SNDTIMEO");
	}
}

/*
	Reads the contents of a file and returns it as a string.

	@param filepath: The path to the file to read.
	@return A string containing the content of the file. If the file cannot be opened, returns an empty string.
*/
std::string ServerConfig::readFile(const std::string& filepath) {
	std::ifstream file(filepath.c_str());
	if (!file) {
		std::cerr << "Error: Could not open file: " << filepath << std::endl;
		return "";
	}
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

/*
	Reads data from a client socket and stores it in a buffer.

	@param client_fd: The file descriptor of the client socket.
	@return true if the complete client data (including headers and content) is received, false otherwise.

	This function reads data from the client, appends it to a buffer, and checks for the end of the HTTP headers and the content length.
	If the full data has not been received, it returns false, indicating more data is expected.
*/
bool ServerConfig::readClientData(int client_fd) {
	char buffer[1024];
	int n = recv(client_fd, buffer, sizeof(buffer), 0);

	#ifdef DEBUG_MODE
	std::cout << "\033[0;36m[DEBUG] ServerConfig::readClientData: Received " << n << " bytes\033[0m" << std::endl;
	#endif

	if (n < 0)
		return false;

	if (n == 0)
		return true;

	client_data[client_fd].append(buffer, n);

	#ifdef DEBUG_MODE
	std::cout << "\033[0;36m[DEBUG] ServerConfig::readClientData: Total data size: "
			  << client_data[client_fd].size() << "\033[0m" << std::endl;
	#endif

	std::string& data = client_data[client_fd];
	size_t header_end = data.find("\r\n\r\n");
	if (header_end == std::string::npos) {
		return false;
	}

	size_t content_length_pos = data.find("Content-Length: ");
	if (content_length_pos != std::string::npos) {
		size_t end_pos = data.find("\r\n", content_length_pos);
		std::string length_str = data.substr(content_length_pos + 16, end_pos - (content_length_pos + 16));
		size_t content_length = std::atol(length_str.c_str());

		size_t expected_size = header_end + 4 + content_length;
		if (data.size() < expected_size) {
			#ifdef DEBUG_MODE
			std::cout << "\033[0;36m[DEBUG] ServerConfig::readClientData: Waiting for more data. "
					  << data.size() << "/" << expected_size << " bytes\033[0m" << std::endl;
			#endif
			return false;
		}
	}
	return true;
}

/*
	Sets up a server socket for accepting incoming client connections.

	@return The file descriptor of the server socket.

	This function creates a socket, binds it to the server's address and port, and listens for incoming client connections.
	It also sets the socket to non-blocking mode to handle asynchronous communication.
	If any step fails (socket creation, binding, listening, or non-blocking setup), the function throws an appropriate exception.
*/
int ServerConfig::setupServerSocket()
{
	m_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (m_socket == -1)
		throw ServerConfig::SocketCreationFailed();

	int opt = 1;
	if (setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
		throw ServerConfig::SocketCreationFailed();
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(_port);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	// Bind the socket to the address and port
	if (bind(m_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
		throw ServerConfig::SocketBindingFailed();

	//std::cout << GREEN << "[INFO]" << RESET << " Server bound to port: " << YELLOW << _port << RESET << std::endl;


	// Listen for incoming connections
	if (listen(m_socket, SOMAXCONN) < 0)
		throw ServerConfig::SocketListeningFailed();

	std::cout << GREEN << "[INFO]" << RESET << " Server is listening on port: " << YELLOW << _port << RESET << std::endl;

	// Set the server socket to non-blocking mode
	if (set_nonblocking(m_socket) == -1)
		throw ServerConfig::SocketCreationFailed();

	// Return the socket descriptor
	return m_socket;
}


/* ---------------------- Setters ---------------------- */


void ServerConfig::setHost(const std::string& host) {
	_host = host;
}

void ServerConfig::setPort(int port) {
	_port = port;
}

void ServerConfig::setServerName(const std::vector<std::string>& tokens) {
	if (!tokens.empty()) {
		_serverName = tokens[0];
	}
}

void ServerConfig::setRoot(const std::vector<std::string>& tokens) {
	if (!tokens.empty()) {
		_root = tokens[0];
	}
}

void ServerConfig::setErrorPage(const std::vector<std::string>& tokens) {
	if (tokens.size() >= 2) {
		int code = atoi(tokens[0].c_str());
		 _errorPages[code] = tokens[1];
	}
}

void ServerConfig::setClientMaxBodySize(size_t token) {

    std::stringstream ss;
    ss << token;
    std::string sizeString = ss.str();

    if (sizeString[sizeString.length() - 1] == 'M' || sizeString[sizeString.length() - 1] == 'm') {
        sizeString = sizeString.substr(0, sizeString.length() - 1);
        size_t sizeInMb = 0;
        std::istringstream(sizeString) >> sizeInMb;

        token = sizeInMb * 1024 * 1024;
    }

    _clientMaxBodySize = token;
}

void ServerConfig::setCgiEnabled(const std::vector<std::string>& tokens) {
	if (!tokens.empty()) {
		std::string val = tokens[0];
		_cgiEnabled = (val == "on" || val == "1" || val == "true");
	}
}

void ServerConfig::setCgiExtension(const std::vector<std::string>& tokens) {
	if (!tokens.empty()) {
		_cgiExtension = tokens[0];
	}
}

void ServerConfig::setCgiBin(const std::vector<std::string>& tokens) {
	if (!tokens.empty()) {
		_cgiBin = tokens[0];
	}
}

/* ---------------------- Getters ---------------------- */

std::string ServerConfig::getHost() const {
	return _host;
}

int ServerConfig::getPort() const {
	return _port;
}

std::string ServerConfig::getServerName() const {
	return _serverName;
}

std::string ServerConfig::getRoot() const {
	return _root;
}

int ServerConfig::getClientMaxBodySize() const {
	return static_cast<int>(_clientMaxBodySize);
}

bool ServerConfig::isCgiEnabled() const {
	return _cgiEnabled;
}

std::string ServerConfig::getCgiExtension() const {
	return _cgiExtension;
}

std::string ServerConfig::getCgiBin() const {
	return _cgiBin;
}

/* ---------------- Locations Accessors ---------------- */


const std::map<std::string, Location>& ServerConfig::getLocations() const {
	return _locations;
}


void ServerConfig::addLocation(const std::string& path, const Location& location) {
	_locations[path] = location;
}

/* ---------------- Error pages methods and accesors ---------------- */

void ServerConfig::addErrorPage(int code, const std::string& page)
{
		_errorPages[code] = page;
}

	const std::map<int, std::string>& ServerConfig::getErrorPages() const {
		return _errorPages;
	}

/*
	Checks and sets default error pages for common HTTP error codes (400, 403, 404, etc.) if no custom error page is defined.

	For each error code (400, 403, 404, 405, 413, 415, 500), the function checks if a custom error page path has been provided.
	If not, it assigns a default error page file path. If the custom error page exists, it remains in place;
	otherwise, it is replaced with the default error page. The error pages are stored in a map (_errorPages) indexed by error code.

	No return value.
*/
void  ServerConfig::checkErrorPage()
{
	std::vector<int> errorsToCheck;
	errorsToCheck.push_back(400);
	errorsToCheck.push_back(403);
	errorsToCheck.push_back(404);
	errorsToCheck.push_back(405);
	errorsToCheck.push_back(408);
	errorsToCheck.push_back(413);
	errorsToCheck.push_back(415);
	errorsToCheck.push_back(500);
	errorsToCheck.push_back(504);

	std::map<int, std::string> defaultErrorPaths;
	defaultErrorPaths[400] = "/HTMLFiles/error/400.html";
	defaultErrorPaths[403] = "/HTMLFiles/error/403.html";
	defaultErrorPaths[404] = "/HTMLFiles/error/404.html";
	defaultErrorPaths[405] = "/HTMLFiles/error/405.html";
	defaultErrorPaths[408] = "/HTMLFiles/error/408.html";
	defaultErrorPaths[413] = "/HTMLFiles/error/413.html";
	defaultErrorPaths[415] = "/HTMLFiles/error/415.html";
	defaultErrorPaths[500] = "/HTMLFiles/error/500.html";
	defaultErrorPaths[504] = "/HTMLFiles/error/504.html";

	for (size_t i = 0; i < errorsToCheck.size(); ++i)
	{
		int errorCode = errorsToCheck[i];
		if (_errorPages.find(errorCode) == _errorPages.end())
		{
			std::string defaultErrorPagePath = getErrorFilePath(errorCode);
			_errorPages[errorCode] = defaultErrorPagePath;
		}
		else
		{
			const std::string& filePath = _errorPages[errorCode];
			if (access(filePath.c_str(), F_OK) == -1)
			{
				std::string defaultErrorPagePath = getErrorFilePath(errorCode);
				_errorPages[errorCode] = defaultErrorPagePath;
				#ifdef PARSER_MODE
					std::cout << "Replaced with default error page: " << defaultErrorPagePath << std::endl;
				#endif
			}
			else
			{
				#ifdef PARSER_MODE
					std::cout << "Error page for code " << errorCode << " is valid: " << filePath << std::endl;
				#endif
			}
		}
	}
}

/* ---------------------- Getters ---------------------- */

/*
	Retrieves the path of the currently running executable.
	@return The absolute path of the executable as a string.
*/
std::string ServerConfig::getExecutablePath() {
	char result[PATH_MAX];
	ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
	return std::string(result, (count > 0) ? count : 0);
}

int ServerConfig::getSocket() const {
	return m_socket;
}

std::string& ServerConfig::getClientData(int client_fd) {
	return client_data[client_fd];
}

void ServerConfig::eraseClientData(int client_fd) {
	client_data.erase(client_fd);
}

/*
    Generates the file path for an error page based on the error code.

    @param errorCode The HTTP error code (e.g., 404, 500).
    @return The file path to the corresponding error page HTML file.
*/
std::string ServerConfig::getErrorFilePath(int errorCode)
{
	std::string execPath = getExecutablePath();
	std::string execDir = execPath.substr(0, execPath.find_last_of("/"));

	std::ostringstream oss;
	oss << errorCode;

	std::string errorFilePath = execDir + "/HTMLFiles/error/" + oss.str() + ".html";

	return errorFilePath;
}

/* ---------------------- Methods ---------------------- */

/*
    Finds a location configuration by its path.

    This function searches for a location configuration in the `_locations` map using the provided `locationPath`.
    If the location is found, it returns the corresponding `Location` object. If not, it throws a `LocationNotFound` exception.

    @param locationPath The path of the location to search for.

    @return The `Location` object associated with the given path.

    @throws ServerConfig::LocationNotFound If the location path is not found in the `_locations` map.
*/

Location ServerConfig::findLocation(std::string locationPath)
{
	 std::map<std::string, Location>::iterator it = _locations.find(locationPath);

	if (it != _locations.end())
		return (it->second);
	else
		throw  ServerConfig::LocationNotFound();

}

/*
	Checks if the given content length is within the allowed maximum size for the body.

	This function compares the provided `contentLength` with the server's configured maximum allowed body size (_clientMaxBodySize).
	It returns `true` if the content length is within the allowed limit; otherwise, it returns `false`.

	@param contentLength The size of the body content to check, in bytes.

	@return True if the content length is less than or equal to the allowed size; false otherwise.

	@note In DEBUG_MODE, the function will log the content length being checked and the maximum allowed body size to the console.
*/

bool ServerConfig::isBodySizeAllowed(size_t contentLength) const {
	#ifdef DEBUG_MODE
	std::cout << "\033[0;36m[DEBUG] ServerConfig::isBodySizeAllowed: Checking size " << contentLength
			  << " against maximum " << _clientMaxBodySize << "\033[0m" << std::endl;
	#endif
	return contentLength <= _clientMaxBodySize;
}


/* ---------------------- Overload Operator ---------------------- */

std::ostream& operator<<(std::ostream& os, const ServerConfig& server) {

	os << "\n### Server ###" << std::endl;

	os << "\033[1m\033[31mPort: " << server.getPort() << "\033[0m" << std::endl;
	os << "\033[1m\033[32mHost: " << server.getHost() << "\033[0m" << std::endl;
	os << "\033[1m\033[34mMaxbodysize: " << server.getClientMaxBodySize() << "\033[0m" << std::endl;

	os << std::endl;

	os << "Root: " << server.getRoot() << std::endl;

	os << "Error Pages:\n";
	for (std::map<int, std::string>::const_iterator it = server.getErrorPages().begin(); it != server.getErrorPages().end(); ++it) {
		os << "Error Code " << it->first << " -> " << it->second << std::endl;
	}

	os << "Locations: " << std::endl;
	for (std::map<std::string, Location>::const_iterator it = server.getLocations().begin(); it != server.getLocations().end(); ++it) {
		os << it->first << ":";
		os << it->second;
	}

	return os;
}

/* ---------------------- Exceptions ---------------------- */

const char* ServerConfig::SocketCreationFailed::what() const throw () {
	return "Throwing exception: creating server socket";
}

const char* ServerConfig::SocketBindingFailed::what() const throw () {
	return "Throwing exception: socket binding failed";
}

const char* ServerConfig::SocketAcceptFailed::what() const throw () {
	return "Throwing exception: Failed to accept connection";
}

const char* ServerConfig::SocketReadFailed::what() const throw () {
	return "Throwing exception: Failed to read from client";
}

const char* ServerConfig::LocationNotFound::what() const throw () {
	return "Throwing exception: Location not found";
}

const char* ServerConfig::SocketListeningFailed::what() const throw () {
	return "Throwing exception: socket listening failed";
}
