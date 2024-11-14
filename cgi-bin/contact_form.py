#!/usr/bin/python3

import urllib.parse
import os
import sys

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

# Lire la longueur du contenu et récupérer les données du formulaire
content_length = int(os.environ.get('CONTENT_LENGTH', 0))  # Utiliser os.environ pour les variables d'environnement
post_data = sys.stdin.read(content_length)  # Utiliser sys.stdin pour lire les données envoyées

# Décoder les données POST
parsed_data = urllib.parse.parse_qs(post_data)

# Récupérer les valeurs des champs du formulaire
name = parsed_data.get('name', [''])[0]
email = parsed_data.get('email', [''])[0]
age = parsed_data.get('age', [''])[0]

# Affichage des données saisies par l'utilisateur dans un style similaire à l'exemple HTML
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
