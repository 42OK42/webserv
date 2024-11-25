#!/usr/bin/python3

print("Content-Type: text/html\n")
print("<html><body>")
print("<h1>Crash Test</h1>")


x = 1/0

raise Exception("Boom!")

exit(1)

print("</body></html>")
