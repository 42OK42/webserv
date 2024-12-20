/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserver.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ecarlier <ecarlier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/16 15:06:19 by ecarlier          #+#    #+#             */
/*   Updated: 2024/11/25 18:01:53 by ecarlier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserver.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "Router.hpp"
#include <iostream>


Webserver::Webserver() {}

Webserver::Webserver(const std::vector<ServerConfig>& servers) : _servers(servers) {}

Webserver::~Webserver() {
	for (std::vector<struct pollfd>::iterator it = fds.begin(); it != fds.end(); ++it) {
		close(it->fd);
	}
}

/*
	Initializes server sockets for each unique port in the server list.

	The function iterates through the list of servers, checks if the port has been initialized already,
	and if not, attempts to initialize the server socket for that port. Successfully initialized sockets
	are added to the `fds` vector for event polling. If an error occurs during socket initialization,
	it is caught and reported, but the process continues for the other servers.

	@returns void
*/
void Webserver::initializeServers()
{
	std::set<int> initializedPorts;

	/*
		Iterates over the list of servers to initialize their sockets.

		For each server in the `_servers` list, the port is checked to ensure it hasn't been initialized yet.
		If the port is new, the server's socket is set up, and the port is added to the `initializedPorts` set
		to avoid duplicate initialization. If an error occurs during socket setup, it is caught and logged.
	*/

	for (size_t i = 0; i < _servers.size(); ++i) {
		ServerConfig& server = _servers[i];
		int port = server.getPort();
		/*
			Skips initializing the socket if the port has already been initialized.
			If the current port has already been added to the `initializedPorts` set, the function moves on
			to the next server without attempting to reinitialize the socket.
		*/
		if (initializedPorts.find(port) != initializedPorts.end()) {
			continue;
		}
		try {
		/*
			Attempts to set up the server socket for the current server.

			Calls the `setupServerSocket()` method of the server configuration object to create a socket.
			If successful, the socket file descriptor is added to the list of file descriptors `fds`
			for event polling. Any failure in this process throws an exception, which is handled in the
			`catch` block below.
		*/
			int server_socket = server.setupServerSocket();
			struct pollfd server_fd;
			server_fd.fd = server_socket;
			server_fd.events = POLLIN;
			fds.push_back(server_fd);
			initializedPorts.insert(port);
		}
		catch (const std::exception& e) {
			std::cerr << RED << "[ERROR]" << RESET << " Error initializing socket for port " << YELLOW << port << RESET << ": " << e.what() << std::endl;
		}
	}

	if (!fds.empty()) {
		std::cout << CYAN << "[INFO]" << RESET << " Available server configurations:" << std::endl;
		for (size_t i = 0; i < _servers.size(); ++i) {
			std::cout << GREEN << "[CONFIG]" << RESET << " Server configuration available: "
					  << _servers[i].getHost() << ":"
					  << _servers[i].getPort() << std::endl;

		}
	/*
		Starts the event loop to monitor server events.

		If there are valid server sockets in `fds`, the program enters an event loop to handle incoming
		events like connections and data requests.
	*/
		runEventLoop();
	} else {
		throw std::runtime_error("No server sockets could be initialized");
	}
}

/*
	Runs the event loop to monitor and handle I/O events for server sockets and client connections.

	The function uses `poll()` to check the file descriptors (representing server sockets and client connections)
	for any events. If an event is detected, the function processes it: for server sockets, it handles new incoming
	connections, and for client connections, it processes any data sent by the clients. The loop continues to run,
	waiting for events, with a timeout of 1 second.

	@returns void
*/
void Webserver::runEventLoop() {
	if (fds.empty()) {
		return;
	}

	std::vector<struct pollfd> current_fds = fds;

	int poll_count = poll(&current_fds[0], current_fds.size(), 1000);

	if (poll_count < 0) {
		if (errno == EINTR) {
			return;
		}
		std::cerr << "Poll error: " << strerror(errno) << std::endl;
		return;
	}

	if (poll_count == 0) {
		return;
	}

	for (size_t i = 0; i < current_fds.size() && !sigint_flag; ++i) {
		if (current_fds[i].revents & (POLLIN | POLLHUP)) {
			if (isServerSocket(current_fds[i].fd)) {
				handleNewConnection(current_fds[i].fd);
			} else {
				handleClientData(i);
			}
		}

		if (current_fds[i].revents & (POLLERR | POLLNVAL)) {
			if (!isServerSocket(current_fds[i].fd)) {
				closeConnection(i);
			}
		}
	}
}

/*
	Checks if a given file descriptor corresponds to a server socket.

	The function iterates through the list of servers and compares each server's socket file descriptor
	with the given `fd`. If a match is found, it returns `true`, indicating that the file descriptor
	belongs to a server socket. Otherwise, it returns `false`.

	@param fd The file descriptor to check.
	@returns bool True if the file descriptor belongs to a server socket, false otherwise.
*/
bool Webserver::isServerSocket(int fd) {
	for (size_t i = 0; i < _servers.size(); ++i) {
		if (_servers[i].getSocket() == fd) {
			return true;
		}
	}
	return false;
}

/*
	Handles a new incoming connection from a client.

	The function accepts a new connection from the provided `server_socket`. After successfully accepting
	the connection, it sets the socket to non-blocking mode and applies a 60-second timeout for the connection.
	The new socket is added to the list of file descriptors (`fds`) for event monitoring, and the association
	between the client socket and server socket is stored in the `client_to_server` map. A message is logged to
	indicate that a new client has connected to the server.

	@param server_socket The server socket that accepted the new connection.
	@returns void
*/
void Webserver::handleNewConnection(int server_socket) {
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);

	int new_fd = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
	if (new_fd < 0) {
		std::cerr << "Accept error: " << strerror(errno) << std::endl;
		return;
	}

	setNonBlocking(new_fd);

	struct pollfd client_fd;
	client_fd.fd = new_fd;
	client_fd.events = POLLIN;
	fds.push_back(client_fd);

	client_to_server[new_fd] = server_socket;
	std::cout << "\033[0;35m[Connection]\033[0m New client " << new_fd
			  << " connected to server socket " << server_socket << std::endl;
}

/*
	Handles data from an existing client connection.

	This function processes incoming data from a client socket. First, it identifies the server socket associated
	with the client and attempts to read the client data. If the data is successfully read, it attempts to parse the
	HTTP request. If the request is valid, it determines which server should handle the request, and then processes
	the request accordingly. If any errors occur (such as exceeding the maximum request body size), an error response
	is sent back to the client. After processing, client data is erased, and the connection is closed if the client
	requested to close it or if an error occurred.

	@param index The index in the `fds` array corresponding to the client connection to handle.
	@returns void
*/
void Webserver::handleClientData(size_t index) {
	if (index >= fds.size()) {
		return;
	}

	int client_fd = fds[index].fd;
	bool shouldClose = false;

	try {
		std::map<int, int>::iterator server_it = client_to_server.find(client_fd);
		if (server_it == client_to_server.end()) {
			closeConnection(index);
			return;
		}

		int server_socket = server_it->second;

		ServerConfig* server = NULL;
		for (size_t i = 0; i < _servers.size(); ++i) {
			if (_servers[i].getSocket() == server_socket) {
				server = &_servers[i];
				break;
			}
		}

		if (!server) {
			#ifdef DEBUG_MODE
			std::cerr << "\033[0;31m[DEBUG] Webserver::handleClientData: No server found for socket " << server_socket << "\033[0m" << std::endl;
			#endif
			closeConnection(index);
			return;
		}

		bool requestComplete = server->readClientData(client_fd);
		if (!requestComplete) {
			return;
		}

		std::string& requestData = server->getClientData(client_fd);

		if (requestData.empty()) {
			closeConnection(index);
			return;
		}

		HttpRequest httpRequest(requestData.c_str(), requestData.length(), *server);
		if (httpRequest.getHeader("Connection") == "close") {
			shouldClose = true;
		}

		ServerConfig* matchingServer = findMatchingServer(httpRequest.getHost(), httpRequest.getPort());
		if (matchingServer) {
			processRequest(httpRequest, matchingServer, client_fd);
		} else {
			processRequest(httpRequest, server, client_fd);
		}

		server->eraseClientData(client_fd);

		if (shouldClose) {
			#ifdef DEBUG_MODE
			std::cout << "[DEBUG] Closing connection due to Connection: close header" << std::endl;
			#endif
			closeConnection(index);
		}

	} catch (const std::exception& e) {
		std::cerr << "[ERROR] Error in handleClientData: " << e.what() << std::endl;
		closeConnection(index);
	}
}

/*
	Closes a client connection and removes it from monitoring.

	The function closes the specified client connection by shutting down the socket and removing it from the
	list of file descriptors (`fds`). It also removes the mapping between the client socket and the server socket
	from the `client_to_server` map. A debug message is logged when the connection is closed.

	@param index The index in the `fds` array of the client connection to close.
	@returns void
*/
void Webserver::closeConnection(size_t index) {
	if (index >= fds.size()) {
		std::cerr << "[ERROR] Invalid index in closeConnection: " << index << std::endl;
		return;
	}

	int client_fd = fds[index].fd;
	#ifdef DEBUG_MODE
	std::cout << "[DEBUG] Closing connection for client_fd: " << client_fd << std::endl;
	#endif

	std::map<int, int>::iterator server_it = client_to_server.find(client_fd);
	if (server_it != client_to_server.end()) {
		client_to_server.erase(server_it);
	}

	if (client_fd >= 0) {
		close(client_fd);
	}

	try {
		fds.erase(fds.begin() + index);
	} catch (const std::exception& e) {
		std::cerr << "[ERROR] Failed to erase fd: " << e.what() << std::endl;
	}
}

/*
	Sets the socket to non-blocking mode.

	@param sockfd The socket file descriptor to modify.
	@returns void
*/
void Webserver::setNonBlocking(int sockfd) {
	int flags = fcntl(sockfd, 3, 0);
	if (flags == -1) {
		std::cerr << "Error getting socket flags: " << strerror(errno) << std::endl;
		return;
	}
	if (fcntl(sockfd, 4, flags | O_NONBLOCK) == -1) {
		std::cerr << "Error setting socket to non-blocking: " << strerror(errno) << std::endl;
	}
}

/*
	Searches for a matching server based on the host and port.

	@param host The host to match.
	@param port The port to match.
	@returns ServerConfig* Pointer to the matching server configuration, or NULL if no match is found.
*/
ServerConfig* Webserver::findMatchingServer(const std::string& host, int port) {
	std::cout << "\033[0;33m[Router]\033[0m Searching server for " << host << ":" << port << std::endl;  // Gelb

	for (size_t i = 0; i < _servers.size(); ++i) {
		const ServerConfig& server = _servers[i];

		if (server.getPort() == port) {
			if (server.getHost() == host) {
				std::cout << "\033[0;32m[Router]\033[0m Found matching server: "
						  << server.getHost() << ":" << server.getPort() << std::endl;  // Grün
				return &_servers[i];
			}
		}
	}

	std::cout << "\033[0;31m[Router]\033[0m No matching server found!" << std::endl;  // Rot
	return NULL;
}


/*
	Processes an HTTP request from a client.

	The function processes the given HTTP request by passing it to the router for handling the necessary routes.
	After handling the request, it generates the corresponding HTTP response and sends it back to the client.
	Debug messages are logged during each stage of processing, including the sending of the response.

	@param httpRequest The HTTP request to be processed.
	@param server The server configuration that will handle the request.
	@param client_fd The client socket file descriptor to send the response to.
	@returns void
*/
void Webserver::processRequest(HttpRequest& httpRequest, ServerConfig* server, int client_fd) {
	if (!server) {
		return;
	}

	#ifdef DEBUG_MODE
	std::cout << "\033[0;36m[DEBUG] Webserver::processRequest: Processing request for client "
			  << client_fd << "\033[0m" << std::endl;
	#endif

	HttpResponse httpResponse(httpRequest);
	Router router(*server);

	try {
		router.handleRequest(httpRequest, httpResponse);

		std::string responseStr = httpResponse.toString();
		ssize_t total_sent = 0;

		while (total_sent < static_cast<ssize_t>(responseStr.length())) {
			ssize_t sent = send(client_fd, responseStr.c_str() + total_sent,
							  responseStr.length() - total_sent, MSG_NOSIGNAL);
			if (sent <= 0) {
				break;
			}
			total_sent += sent;
		}

		if (httpResponse.getHeader("Connection") == "close") {
			for (size_t i = 0; i < fds.size(); ++i) {
				if (fds[i].fd == client_fd) {
					closeConnection(i);
					break;
				}
			}
		}
	} catch (const std::exception& e) {
		std::cerr << "Error in processRequest: " << e.what() << std::endl;
		for (size_t i = 0; i < fds.size(); ++i) {
			if (fds[i].fd == client_fd) {
				closeConnection(i);
				break;
			}
		}
	}
}

void Webserver::cleanup() {
	for (size_t i = 0; i < fds.size(); ++i) {
		if (!isServerSocket(fds[i].fd)) {
			closeConnection(i);
			i--;
		}
	}

	for (size_t i = 0; i < fds.size(); ++i) {
		if (isServerSocket(fds[i].fd)) {
			close(fds[i].fd);
		}
	}

	fds.clear();
	client_to_server.clear();
}
