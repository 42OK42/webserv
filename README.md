# Webserv

A HTTP/1.1 server implementation in C++98, inspired by NGINX.

## Overview

This project involves the development of an HTTP server written in C++98, designed to handle HTTP/1.1 requests. The server is built from scratch, with no reliance on external server implementations like NGINX. Its primary purpose is to deliver static webpages, manage file uploads, and integrate CGI (e.g., PHP scripts). Using non-blocking I/O (poll() or alternatives), the server aims to be efficient, configurable, and reliable, providing functionality comparable to lightweight HTTP servers.

## Features

- **HTTP Methods**: Supports GET, POST, and DELETE requests
- **Static Content Delivery**: Serves static webpages efficiently
- **File Uploads**: Handles file uploads through HTTP requests
- **CGI Integration**: Executes external scripts (e.g., PHP) using Common Gateway Interface
- **Non-blocking I/O**: Implements non-blocking input/output using poll() (or alternatives like select, kqueue, or epoll)
- **Configuration File**:
  - Specify ports, routes, and error pages
  - Set client limits (e.g., maximum concurrent connections or request size)
- **Standards Compliance**: Adheres to the HTTP/1.1 specification
- **Robustness**: Designed for stability and scalability, taking inspiration from NGINX

## Usage

### Building the Server

```bash
# Clone the repository
git clone git@github.com:42OK42/webserv.git
cd webserv

# Compile the server
make

# Run the server
./webserv [config_file]
```

### Main Features

#### 1. Static Webpages
- Server can serve static HTML files from the `HTMLFiles/` directory
- Supports various error pages (400, 403, 404, 405, 408, 413, 415, 500, 504)

#### 2. File Upload
- Navigate to `/upload`
- Supports image and video files
- Uploads are stored in the `uploads/` directory
- Successfully uploaded files can be viewed and deleted

#### 3. Contact Form
- Accessible at `/form`
- Processes POST requests using Python CGI
- Collects user's name, email, and age

#### 4. CGI Support
- Python CGI scripts in `cgi-bin/` directory
- Processes dynamic requests
- Supports POST and GET methods

### Configuration

The server can be customized via a configuration file. Important settings include:
- Port number
- Server name
- Root directory
- Allowed methods
- Maximum body size
- CGI settings

### Debug Mode

```bash
# Compile the server with debug information
make DEBUG=1
```

### Stopping the Server

The server can be cleanly terminated using `Ctrl+C`

### Supported HTTP Methods
- GET
- POST
- DELETE

### Special Features
- Themed in "Tractor Worship" style
- Responsive web design
- File upload management
- Error handling with user-friendly error pages

## Key Learnings

- **Low-Level Network Programming**: Leveraged sockets and non-blocking I/O mechanisms to manage multiple connections efficiently
- **HTTP Protocol Handling**: Gained in-depth knowledge of HTTP/1.1, including request parsing, response formatting, and status codes
- **Concurrency**: Managed simultaneous client requests without blocking, using event-driven architectures (poll()/select)
- **C++98 Practices**: Designed the server with pre-modern C++ standards, emphasizing manual memory management and efficiency
- **File Management**: Implemented file handling for static delivery and file uploads
- **CGI Integration**: Learned how to integrate and execute external programs dynamically to extend server functionality
- **Configuration Management**: Built a flexible configuration parser to adjust server behavior without recompilation
- **Debugging and Stability**: Identified and resolved edge cases to ensure stability under heavy load or malformed requests
