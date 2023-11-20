import os
import re

# Pobierz aktualny katalog roboczy (current working directory)
current_directory = os.getcwd()

input_file_path = os.path.join(current_directory, "include", "index.html")
output_file_path = os.path.join(current_directory, "include", "webPage.h")

css_files = ["style.css", "weather.css", "qr_code.css"]
js_files = ["app.js", "weather.js", "qr_code.js"]  # Dodaj nazwy plików JS


# Odczytaj zawartość pliku HTML
with open(input_file_path, "r") as input_file:
    content = input_file.read()



# Zastąp linki do plików CSS ich zawartością
for css_file in css_files:
    css_file_path = os.path.join(current_directory, "include", "css", css_file)
    with open(css_file_path, "r") as css_file_content:
        css_content = css_file_content.read()

    # Zastąp link do pliku CSS w pliku HTML zawartością CSS
    content = content.replace(f'<link rel="stylesheet" href="css/{css_file}" />', f'<style type="text/css">{css_content}</style>')

# Zastąp linki do plików JS ich zawartością
for js_file in js_files:
    js_file_path = os.path.join(current_directory, "include", "app", js_file)
    with open(js_file_path, "r") as js_file_content:
        js_content = js_file_content.read()

    # Zastąp link do pliku JS w pliku HTML zawartością JS
    content = content.replace(f'<script type="text/javascript" src="app/{js_file}"></script>', f'<script type="text/javascript">{js_content}</script>')



# Usuń białe znaki i nadmiarowe spacje
# content = re.sub(r'\s+', ' ', content).strip()


# Dodaj #include <Arduino.h> na początku
content = "#include <Arduino.h>\n\n const char webpage[] PROGMEM = R\"rawliteral(\n\n" + content + "\n\n)rawliteral\";"

# Zapisz zmieniony plik
with open(output_file_path, "w") as output_file:
    output_file.write(content)





