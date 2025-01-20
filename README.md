Overview

This project involves the development of an HTTP server written in C++98, designed to handle HTTP/1.1 requests. The server is built from scratch, with no reliance on external server implementations like NGINX. Its primary purpose is to deliver static webpages, manage file uploads, and integrate CGI (e.g., PHP scripts). Using non-blocking I/O (poll() or alternatives), the server aims to be efficient, configurable, and reliable, providing functionality comparable to lightweight HTTP servers.
Features

    HTTP Methods: Supports GET, POST, and DELETE requests.
    Static Content Delivery: Serves static webpages efficiently.
    File Uploads: Handles file uploads through HTTP requests.
    CGI Integration: Executes external scripts (e.g., PHP) using Common Gateway Interface.
    Non-blocking I/O: Implements non-blocking input/output using poll() (or alternatives like select, kqueue, or epoll).
    Configuration File:
        Specify ports, routes, and error pages.
        Set client limits (e.g., maximum concurrent connections or request size).
    Standards Compliance: Adheres to the HTTP/1.1 specification.
    Robustness: Designed for stability and scalability, taking inspiration from NGINX.

Key Learnings

    Low-Level Network Programming: Leveraged sockets and non-blocking I/O mechanisms to manage multiple connections efficiently.
    HTTP Protocol Handling: Gained in-depth knowledge of HTTP/1.1, including request parsing, response formatting, and status codes.
    Concurrency: Managed simultaneous client requests without blocking, using event-driven architectures (poll()/select).
    C++98 Practices: Designed the server with pre-modern C++ standards, emphasizing manual memory management and efficiency.
    File Management: Implemented file handling for static delivery and file uploads.
    CGI Integration: Learned how to integrate and execute external programs dynamically to extend server functionality.
    Configuration Management: Built a flexible configuration parser to adjust server behavior without recompilation.
    Debugging and Stability: Identified and resolved edge cases to ensure stability under heavy load or malformed requests.

Usage

    ## Usage

# Clone the Repository
git clone <repository-url>
cd webserv

# Compile the Server
make

# To enable debug mode
make DEBUG=1

# Run the Server
./webserv [config_file]

# Stopping the Server
# The server can be cleanly terminated using Ctrl+C.

# Supported HTTP Methods
# - GET: Retrieve resources from the server.
# - POST: Submit data to the server (e.g., for file uploads).
# - DELETE: Remove resources from the server.

# Special Features
# - Themed in "Tractor Worship" style: Unique and creative aesthetic.
# - Responsive Web Design: Optimized for various devices.
# - File Upload Management: Handles user file uploads seamlessly.


# - Error Handling: Provides user-friendly custom error pages.
