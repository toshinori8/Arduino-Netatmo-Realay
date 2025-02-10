# Arduino-Netatmo-Relay

## Opis projektu

Arduino-Netatmo-Relay to aplikacja, która integruje dane z Netatmo z systemem grzewczym opartym na piecu i kotle gazowym. Aplikacja umożliwia ustawianie temperatur, pobieranie aktualnych danych z Netatmo oraz łączenie ich z systemem grzewczym, aby zapewnić optymalne warunki w pomieszczeniach.

## Funkcje

- Pobieranie danych z Netatmo API za pomocą serwera proxy.
- Ustawianie zadanej temperatury dla poszczególnych pomieszczeń.
- Aktualizacja danych o temperaturze w czasie rzeczywistym.
- Integracja z systemem grzewczym opartym na piecu i kotle gazowym.
- Wyświetlanie aktualnych danych o temperaturze w interfejsie użytkownika.

## Struktura projektu

### Pliki i foldery

- `src/roomManager.h`: Plik nagłówkowy zawierający definicję klasy `RoomManager` oraz struktury `RoomData`.
- `src/main.cpp`: Główny plik źródłowy aplikacji.
- `include/src/app.js`: Plik JavaScript odpowiedzialny za interakcję z WebSocket oraz aktualizację interfejsu użytkownika.
- `include/indexb.html`: Plik HTML zawierający strukturę interfejsu użytkownika.

### Klasa `RoomManager`

Klasa `RoomManager` zarządza danymi o pomieszczeniach oraz integracją z Netatmo API. Posiada następujące metody:

- `addRoom(const RoomData &room)`: Dodaje nowe pomieszczenie do listy.
- `updateOrAddRoom(const RoomData &room)`: Aktualizuje istniejące pomieszczenie lub dodaje nowe, jeśli nie istnieje.
- `updateRoomParams(RoomData &existingRoom, const RoomData &newRoom)`: Aktualizuje parametry istniejącego pomieszczenia na podstawie nowych danych.
- `getRoom(size_t index)`: Zwraca dane pomieszczenia na podstawie indeksu.
- `updateRoom(size_t index, const RoomData &room)`: Aktualizuje dane pomieszczenia na podstawie indeksu.
- `isRequestInProgress() const`: Sprawdza, czy żądanie jest w trakcie realizacji.
- `setRequestInProgress(bool inProgress)`: Ustawia stan żądania.
- `getRoomsAsJson()`: Zwraca dane pomieszczeń w formacie JSON.
- `fetchJsonData(const char *url)`: Pobiera dane JSON z Netatmo API.

### Struktura `RoomData`

Struktura `RoomData` przechowuje dane o pomieszczeniu:

- `std::string name`: Nazwa pomieszczenia.
- `int ID`: ID pomieszczenia.
- `int pinNumber`: Numer przyporządkowanego pinu.
- `float targetTemperature`: Temperatura zadana.
- `float currentTemperature`: Temperatura aktualna.
- `bool forced`: Czy jest to tryb wymuszony.

### Interfejs użytkownika

Interfejs użytkownika jest zbudowany przy użyciu HTML, CSS i JavaScript. Plik `indexb.html` zawiera strukturę interfejsu, `app.css` zawiera style, a `app.js` obsługuje interakcje z WebSocket oraz aktualizację danych w czasie rzeczywistym.

## Instalacja

## Instalacja

1. Sklonuj repozytorium:
    ```sh
    git clone https://github.com/yourusername/Arduino-Netatmo-Relay.git
    ```
2. Otwórz projekt w PlatformIO.

3. Wgraj projekt na swoje urządzenie Arduino.

## Użycie

1. Połącz się z nowym urządzeniem Wi-Fi i przejdź do strony urządzenia (portal przechwytujący). Skonfiguruj Wi-Fi, a następnie zrestartuj urządzenie.
2. Otwórz przeglądarkę i przejdź do interfejsu użytkownika pod adresem http://netatmo_relay.local/.
3. Skonfiguruj dane API Netatmo, takie jak **client_id**, **client_secret**, **username** oraz **password**, klikając pierwszą ikonę na dole po lewej stronie. Szczegółowe informacje na temat uzyskania tych danych znajdziesz na stronie [Netatmo API](https://dev.netatmo.com/apidocumentation/oauth). Upewnij się, że przy tworzeniu aplikacji Netatmo w serwisie, nadałeś odpowiednie uprawnienia dla **read_thermostat** oraz **write_thermostat**.
4. Ikony po prawej stronie pokażą, czy nawiązano połączenie z urządzeniem NodeMCU v2 i serwisem Netatmo API.

## Licencja

Ten projekt jest licencjonowany na warunkach licencji MIT. Szczegóły znajdują się w pliku LICENSE.

Ten projekt jest licencjonowany na warunkach licencji MIT. Szczegóły znajdują się w pliku LICENSE.