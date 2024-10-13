#!/usr/bin/python3

import urllib.parse
import os
import sys

# Get content length from the environment variable
content_length = int(os.environ.get('Content-Length', 0))

# Read the POST data (request body) from stdin
post_data = sys.stdin.read(content_length)

# Parse the query string
parsed_data = urllib.parse.parse_qs(post_data)

# Fetch form data
name = parsed_data['name'][0]
email = parsed_data['email'][0]
age = parsed_data['age'][0]

# Generate response
print("Content-Type: text/html\n")  # CGI script needs to specify content type in the response
print("<html><head><title>Contact Form Submission</title></head><body>")
print("<h2>Thank you, {}!</h2>".format(name))
print("<p>You are {} years old".format(age))
if int(age) < 0:
	print("... are you sure?</p>")
elif int(age) < 10:
	print("; that's cool!</p>")
elif int(age) < 30:
	print("; well you are still young ;) </p>")
elif int(age) < 90:
	print("; time flies huh?</p>")
else:
	print("; I bow before your wisdom!</p>")
print("<p>We will reach out to you at {}</p>".format(email))
print("</body></html>")
