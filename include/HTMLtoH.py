import os
import re


# Pobierz aktualny katalog roboczy (current working directory)
current_directory = os.getcwd()



input_file_path =  os.path.join(current_directory,  "include",  "index.html")
output_file_path = os.path.join(current_directory,  "include", "webPage.h")

# Odczytaj zawartość pliku
with open(input_file_path, "r") as input_file:
    content = input_file.read()

# Usuń białe znaki i nadmiarowe spacje
content = re.sub(r'\s+', ' ', content).strip()

# Dodaj raw string literal na początku i na końcu
content = "#include <Arduino.h>\n\n const char webpage[] PROGMEM = R\"rawliteral(\n\n" + content + "\n\n)rawliteral\";"

# Zapisz zmieniony plik
with open(output_file_path, "w") as output_file:
    output_file.write(content)