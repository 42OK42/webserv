#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <vector>
#include <map>
#include "HttpRequest.h"
#include "ServerConfig.h"

class Webserver {
public:
    Webserver();
    void handleClientData(size_t index);
    void closeConnection(size_t index);

private:
    std::vector<ServerConfig> _servers;
    std::map<int, int> client_to_server;
    std::vector<struct pollfd> fds;

    ServerConfig* findMatchingServer(const std::string& host, int port);
    void processRequest(const HttpRequest& request, ServerConfig* server, int client_fd);
};

Webserver::Webserver() {
    // Initialize your servers and other necessary components here
}

void Webserver::handleClientData(size_t index) {
    int client_fd = fds[index].fd;
    int server_socket = client_to_server[client_fd];
    
    ServerConfig* server = NULL;
    for (size_t i = 0; i < _servers.size(); ++i) {
        if (_servers[i].getSocket() == server_socket) {
            server = &_servers[i];
            break;
        }
    }
    
    if (!server) {
        std::cerr << "Error: No server configuration found for socket " << server_socket << std::endl;
        closeConnection(index);
        return;
    }
    
    try {
        bool requestComplete = server->readClientData(client_fd);
        if (!requestComplete) {
            return;  // Warte auf mehr Daten
        }

        std::string& requestData = server->getClientData(client_fd);
        HttpRequest httpRequest(requestData.c_str(), requestData.length(), *server);
        
        // Prüfe Connection Header
        bool shouldClose = (httpRequest.getHeader("Connection") == "close");
        
        ServerConfig* matchingServer = findMatchingServer(httpRequest.getHost(), httpRequest.getPort());
        if (matchingServer) {
            processRequest(httpRequest, matchingServer, client_fd);
        } else {
            processRequest(httpRequest, server, client_fd);
        }

        // Lösche nur die Request-Daten, nicht die Verbindung
        server->eraseClientData(client_fd);
        
        // Schließe nur wenn nötig
        if (shouldClose) {
            closeConnection(index);
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error processing request: " << e.what() << std::endl;
        closeConnection(index);
    }
}

void Webserver::closeConnection(size_t index) {
    int client_fd = fds[index].fd;
    close(client_fd);
    fds.erase(fds.begin() + index);
    client_to_server.erase(client_fd);
}

ServerConfig* Webserver::findMatchingServer(const std::string& host, int port) {
    // Implement your logic to find a matching server here
    return NULL;
}

void Webserver::processRequest(const HttpRequest& request, ServerConfig* server, int client_fd) {
    // Implement your logic to process the request here
}

int main() {
    Webserver webserver;
    // Add your code here to handle incoming connections and requests
    return 0;
} 