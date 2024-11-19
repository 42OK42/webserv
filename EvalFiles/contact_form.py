#!/usr/bin/env python3

import socket
import time

# Create a connection to the server
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(('localhost', 8080))

# Wait indefinitely
try:
    while True:
        time.sleep(1)  # Simulate a long wait
except KeyboardInterrupt:
    s.close()
