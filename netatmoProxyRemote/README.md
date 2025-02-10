# Node.js Thermostat API

## Opis
Ten projekt to serwer Node.js, który integruje się z API Netatmo, umożliwiając zarządzanie termostatami. Pozwala na ustawianie temperatury w pokojach, zmianę trybu pracy oraz pobieranie danych o stanie termostatów.

## Struktura projektu
```
nodejs-server
├── src
│   ├── config.js
│   ├── tokenize.js
│   ├── setData.js
│   ├── getData.js
│   └── server.js
├── package.json
└── README.md
```

## Wymagania
- Node.js (wersja 20 lub nowsza)
- NPM (Node Package Manager)

## Instalacja
1. Sklonuj repozytorium:
  ```
  git clone <URL_REPOZYTORIUM>
  ```
2. Przejdź do katalogu projektu:
  ```
  cd nodejs-server
  ```
3. Zainstaluj zależności:
  ```
  npm install
  ```

## Użycie
1. Uruchom serwer:
  ```
  npm start
  ```
2. Użyj poniższych endpointów do interakcji z API:

  - **Ustaw temperaturę w pokoju**
    - **Endpoint:** `POST /set-data`
    - **Parametry:**
     - `room_id` (string) - ID pokoju
     - `temperature` (number) - Żądana temperatura

  - **Ustaw tryb domu**
    - **Endpoint:** `POST /set-mode`
    - **Parametry:**
     - `mode` (string) - Tryb pracy (np. `away`, `home`)

  - **Pobierz dane o termostatach**
    - **Endpoint:** `GET /get-data`
    - **Parametry:** Brak

## Przykłady
- Ustawienie temperatury w pokoju:
  ```
  POST /set-data
  Body: {
   "room_id": "<ROOM_ID>",
   "temperature": <TEMPERATURE>
  }
  ```

- Ustawienie trybu domu na "away":
  ```
  POST /set-mode
  Body: {
   "mode": "away"
  }
  ```

- Pobranie danych o termostatach:
  ```
  GET /get-data
  ```

## Licencja
Ten projekt jest objęty licencją MIT.