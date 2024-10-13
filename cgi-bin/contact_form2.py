#!/usr/bin/python3

import os
import sys

# Get content length from the environment variable
content_length = int(os.environ.get('Content-Length', 0))

# Read the POST data (request body) from stdin
post_data = sys.stdin.read(content_length)

# Process the data (e.g., parse form data, etc.)
print("Content-Type: text/html\n")
print(f"<html><body><h1>Received Data</h1><p>{post_data}</p></body></html>")
