#!/usr/bin/python3

import urllib.parse
import os
import sys

print("Content-Type: text/html\n")
print("<h2>Debug Info:</h2>")
print("<pre>")
print("Script started")
print(f"stdin isatty: {sys.stdin.isatty()}")
print(f"stdin encoding: {sys.stdin.encoding}")
print(f"stdin mode: {sys.stdin.mode}")
print("</pre>")

print("<h2>Environment Variables:</h2>")
print("<pre>")
for key, value in os.environ.items():
    print(f"{key}: {value}")
print("</pre>")

content_length = int(os.environ.get('CONTENT_LENGTH', 0))
print(f"<p>Reading {content_length} bytes from stdin...</p>")

# Debug: Print raw stdin info
print("<h2>Raw stdin data:</h2>")
print("<pre>")
print(f"stdin buffer info: {dir(sys.stdin.buffer)}")
print(f"stdin fileno: {sys.stdin.fileno()}")
try:
    print(f"stdin ready: {sys.stdin.ready()}")
except:
    print("stdin.ready() not available")
print("</pre>")

# Lese alle Daten auf einmal
post_data = sys.stdin.read(content_length)

print(f"<p>Read complete. Got {len(post_data)} bytes</p>")
print("<h2>Actual stdin content:</h2>")
print(f"<pre>'{post_data}'</pre>")

try:
    # Debug: Rohdaten
    print("<h2>Raw Data:</h2>")
    print(f"<pre>Length: {len(post_data)}\nData: {repr(post_data)}</pre>")
    
    # Parse the query string
    parsed_data = urllib.parse.parse_qs(post_data)
    
    print("<h2>Parsed Data:</h2>")
    print(f"<pre>{parsed_data}</pre>")
    
    # Fetch form data
    name = parsed_data.get('name', [''])[0]
    email = parsed_data.get('email', [''])[0]
    age = parsed_data.get('age', ['0'])[0]
    
    print("<h2>Form Data:</h2>")
    print(f"<p>Name: {name}</p>")
    print(f"<p>Email: {email}</p>")
    print(f"<p>Age: {age}</p>")
    
except Exception as e:
    print(f"<h1>Error</h1><p>An error occurred: {str(e)}</p>")
