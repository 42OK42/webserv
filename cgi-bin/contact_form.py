#!/usr/bin/python3

import urllib.parse
import os
import sys

# Function to parse GET or POST data
def get_form_data():
    if os.environ.get('REQUEST_METHOD') == 'POST':
        content_length = int(os.environ.get('CONTENT_LENGTH', 0))  # Get the length of the content
        post_data = sys.stdin.read(content_length)  # Read the POST data
        return urllib.parse.parse_qs(post_data)  # Parse the POST data
    else:
        # For GET requests, get the query string from the URL
        query_string = os.environ.get('QUERY_STRING', '')  # Get the query string from the environment
        return urllib.parse.parse_qs(query_string)  # Parse the GET data

# Get the form data from either GET or POST
form_data = get_form_data()

# Retrieve values for each parameter from the parsed data
name = form_data.get('name', [''])[0]
email = form_data.get('email', [''])[0]
age = form_data.get('age', [''])[0]

# Generate the HTML response
print("""
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Vroom Vroom Family - Tractor Worship Form</title>
    <style>
        body {
            font-family: 'Arial', sans-serif;
            background-color: #f0e4d7;
            color: #5f4b3b;
            text-align: center;
            padding: 40px;
        }
        h1 {
            color: #2c6d2f;
            font-size: 4em;
            margin-bottom: 20px;
            text-transform: uppercase;
            font-weight: bold;
            text-shadow: 2px 2px 5px rgba(0, 0, 0, 0.2);
        }
        p {
            font-size: 1.5em;
            color: #3b5d33;
        }
        .form-data {
            font-size: 1.5em;
            color: #3b5d33;
            margin-top: 20px;
        }
        .vroom-button {
            font-size: 1.5em;
            background-color: #2c6d2f;
            color: white;
            border: none;
            padding: 10px 20px;
            cursor: pointer;
            border-radius: 8px;
        }
        .vroom-button:hover {
            background-color: #4c7b1f;
        }
        .button-container {
            margin-top: 30px;
        }
    </style>
</head>
<body>
    <h1>Vroom Vroom Family!</h1>
    <p>Here are the details you've submitted for the Tractor Worship Crew:</p>
""")

# Display the form data in the HTML
print(f"""
    <div class="form-data">
        <p><strong>Tractor Lover's name:</strong> {name}</p>
        <p><strong>Email:</strong> {email}</p>
        <p><strong>Vroom Age:</strong> {age}</p>
    </div>

    <div class="button-container">
        <button onclick="location.href='/'" class="vroom-button">Back to the Tractor Zone!</button>
        <button onclick="location.href='/upload'" class="vroom-button">Upload Your Tractor Pics!</button>
    </div>
</body>
</html>
""")
