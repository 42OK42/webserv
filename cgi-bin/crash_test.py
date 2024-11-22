#!/usr/bin/python3

print("Content-Type: text/html\n")
print("<html><body>")
print("<h1>Crash Test</h1>")

# Verschiedene Möglichkeiten zum Crashen:
x = 1/0                    # Division durch Null
# oder
raise Exception("Boom!")   # Exception werfen
# oder
exit(1)                    # Explizit mit Fehler beenden

print("</body></html>")
