#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <vector>
#include <string>
#include <iostream>
#include <map>

// API endpoints
const char *api_url = "http://netatmo.adamkarski.art/getdata";

struct RoomData
{
    std::string name;         // Nazwa pokoju
    int ID;                   // ID pokoju
    int pinNumber;            // Numer przyporzadkowanego pinu
    float targetTemperature;  // Temperatura zadana
    float currentTemperature; // Temperatura aktualna
    bool forced;              // Czy jest to tryb wymuszony
    String battery_state;     // Stan baterii
    int battery_level;        // Poziom baterii
    int rf_strength;          // Siła sygnału
    String type;              // Typ pokoju
    bool reachable;           // Czy pokój jest osiągalny
    String anticipating;      // Czy pokój jest w trybie oczekiwania
    float priority;           // Priorytet pokoju
    bool valve;               // Czy zawór jest otwarty (dla pokoju z najwyższym priorytetem)


    RoomData() : name(""), ID(-1), pinNumber(0), targetTemperature(0.0), currentTemperature(0.0), forced(false), battery_state(""), battery_level(0), rf_strength(0), reachable(false), anticipating(""), priority(0), valve(false) {}

    RoomData(const std::string& name, int ID, int pinNumber, float targetTemperature, float currentTemperature, bool forced, const String& battery_state, int battery_level, int rf_strength, bool reachable, const String& anticipating, float priority = 0.0, bool valve = false)
        : name(name), ID(ID), pinNumber(pinNumber), targetTemperature(targetTemperature), currentTemperature(currentTemperature), forced(forced), battery_state(battery_state), battery_level(battery_level), rf_strength(rf_strength), reachable(reachable), anticipating(anticipating), priority(priority), valve(valve) {}
};

class RoomManager
{
public:
    RoomManager() : requestInProgress(false) {
        // Inicjalizacja domyślnego mapowania ID na piny
        idToPinMap[1812451076] = 1;
        idToPinMap[206653929] = 2;
        idToPinMap[1868270675] = 3;
        idToPinMap[38038562] = 4;
    }

    void addRoom(const RoomData &room)
    {
        rooms.push_back(room);
        Serial.print("Added room: ");
        Serial.println(room.name.c_str());
    }

    void updateOrAddRoom(const RoomData &room)
    {
        bool roomExists = false;
        for (auto &existingRoom : rooms)
        {
            if (existingRoom.ID == room.ID)
            {
                updateRoomParams(existingRoom, room);
                roomExists = true;
                Serial.print("Updated room: ");
                Serial.println(room.name.c_str());
                break;
            }
        }
        if (!roomExists)
        {
            addRoom(room);
        }
    }

    void updateRoomParams(RoomData &existingRoom, const RoomData &newRoom)
    {
        if (newRoom.targetTemperature != 0.0)
            existingRoom.targetTemperature = newRoom.targetTemperature;
        if(docPins["usegaz"] == "true")
        {
            if (newRoom.targetTemperature != 0.0)
            existingRoom.targetTemperature = newRoom.targetTemperature;
        }

        // Aktualizacja nazwy pokoju
        if (!newRoom.name.empty())
            existingRoom.name = newRoom.name;
        if (newRoom.ID != -1)
            existingRoom.ID = newRoom.ID;
        if (newRoom.pinNumber != 0)
            existingRoom.pinNumber = newRoom.pinNumber;
        
        if (newRoom.currentTemperature != 0.0)
            existingRoom.currentTemperature = newRoom.currentTemperature;
        if(newRoom.battery_state != "")
            existingRoom.battery_state = newRoom.battery_state;
        if(newRoom.battery_level != 0)
            existingRoom.battery_level = newRoom.battery_level;
        if(newRoom.rf_strength != 0)
            existingRoom.rf_strength = newRoom.rf_strength;
        if(newRoom.reachable != false)
            existingRoom.reachable = newRoom.reachable;
        if(newRoom.anticipating != "")
            existingRoom.anticipating = newRoom.anticipating;
        existingRoom.forced = newRoom.forced;

        // Obliczanie priorytetu
        existingRoom.priority = existingRoom.targetTemperature - existingRoom.currentTemperature;

    }

    RoomData getRoom(size_t index)
    {
        if (index < rooms.size())
        {
            return rooms[index];
        }
        else
        {
            Serial.println("Index out of range");
            return RoomData(); // Zwraca pusty RoomData w przypadku błędu
        }
    }

// get room by ID 
    RoomData getRoomByID(int roomID)
    {
        for (const auto &room : rooms)
        {
            if (room.ID == roomID)
            {
                return room;
            }
        }
        Serial.println("Room ID not found");
        return RoomData(); // Zwraca pusty RoomData w przypadku błędu
    }

    void updateRoom(size_t index, const RoomData &room)
    {
        if (index < rooms.size())
        {
            rooms[index] = room;
        }
        else
        {
            Serial.println("Index out of range");
        }
    }

    bool isRequestInProgress() const
    {
        return requestInProgress;
    }

    void setRequestInProgress(bool inProgress)
    {
        requestInProgress = inProgress;
    }

    String getRoomsAsJson()
    {
        DynamicJsonDocument docx(2024);
        JsonArray roomsArray = docx.createNestedArray("rooms");

        // Zmienne do śledzenia pokoju z najniższą temperaturą wśród wymuszonych
        int lowestTempForcedRoomId = -1;
        float lowestTemp = 100.0; // Inicjalizacja wysoką wartością

        // Zmienne do śledzenia pokoju z najwyższym priorytetem (dla przypadku gdy nie ma wymuszonych)
        float maxPriority = -999.0;
        int maxPriorityRoomId = -1;

        // Flaga do śledzenia, czy jakikolwiek pokój jest w trybie wymuszonym
        bool anyRoomForced = false;

        // Najpierw sprawdź, czy są pokoje w trybie wymuszonym i znajdź ten z najniższą temperaturą
        for (const auto &room : rooms)
        {
            // Sprawdź, czy pokój jest w trybie wymuszonym
            if (room.forced) {
                anyRoomForced = true;

                // Sprawdź, czy ten pokój ma najniższą temperaturę wśród wymuszonych
                if (room.currentTemperature < lowestTemp) {
                    lowestTemp = room.currentTemperature;
                    lowestTempForcedRoomId = room.ID;
                }
            }

            // Oblicz priorytet dla przypadku, gdy nie ma pokoi wymuszonych
            float roomPriority = room.targetTemperature - room.currentTemperature;
            if (roomPriority > maxPriority)
            {
                maxPriority = roomPriority;
                maxPriorityRoomId = room.ID;
            }
        }

        for (auto &room : rooms)
        {
            // Ustaw valve na true dla pokoju z najniższą temperaturą wśród wymuszonych
            // lub dla pokoju z najwyższym priorytetem, jeśli nie ma pokoi wymuszonych
            if (anyRoomForced) {
                room.valve = (room.ID == lowestTempForcedRoomId);
            } else {
                room.valve = (room.ID == maxPriorityRoomId);
            }

            JsonObject roomObject = roomsArray.createNestedObject();
            roomObject["name"] = room.name;
            roomObject["id"] = room.ID;
            roomObject["pinNumber"] = room.pinNumber;
            roomObject["targetTemperature"] = room.targetTemperature;
            roomObject["currentTemperature"] = room.currentTemperature;
            roomObject["forced"] = room.forced;
            roomObject["battery_state"] = room.battery_state;
            roomObject["battery_level"] = room.battery_level;
            roomObject["rf_strength"] = room.rf_strength;
            roomObject["reachable"] = room.reachable;
            roomObject["anticipating"] = room.anticipating;
            roomObject["priority"] = room.targetTemperature - room.currentTemperature;
            roomObject["valve"] = room.valve;
        }
         
         // Kopiowanie docPins do meta w docx
        JsonObject meta = docx.createNestedObject("meta");
        meta.set(docPins.as<JsonObject>());


        String jsonString;
        
        serializeJson(docx, jsonString);
        //Serial.println(jsonString);
        return jsonString;
    }

    void setTemperature(int roomID, float temp)
    {
   
       

        Serial.printf("Setting temperature on \n", roomID, temp);
   
            if (WiFi.status() == WL_CONNECTED)
        {
            WiFiClient client;

            HTTPClient http;
   
            String url = "http://netatmo.adamkarski.art/setRoomTemperature?mode=manual&temperature="+String(temp)+"&room_id=" + String(roomID);
            http.begin(client, url);   
            int httpCode = http.GET();
       

            Serial.printf("HTTP GET request code: %d\n", httpCode);
            Serial.printf("%s", url.c_str());

   
            Serial.println(http.getString());

            http.end(); // Dodano http.end()

            // Pobierz aktualne dane po zmianie temperatury
            fetchJsonData(api_url); // Dodano wywołanie fetchJsonData
            
            // Wyslij dane przez websocket
           
        }
        else
        {
            Serial.println("WiFi not connected");
        }
    }

    

    void fetchJsonData(const char *url)
    {
        

        if (isRequestInProgress())
        {
            Serial.println("Request already in progress");
            return;
        }

        setRequestInProgress(true);
        Serial.println("Fetching JSON data from API");

        if (WiFi.status() == WL_CONNECTED)
        {
            WiFiClient client;

            HTTPClient http;
            http.begin(client, url);
            int httpCode = http.GET();

            Serial.printf("HTTP GET request code: %d\n", httpCode);

            if (httpCode > 0)
            {
                String payload = http.getString();
                //Serial.println("Received payload:_______________________");
                //Serial.println(payload);
                

                if (payload.length() > 0)
                {
                    DynamicJsonDocument doc(2048);
                    deserializeJson(doc, payload);
//  {
//       "id": "1812451076",
//       "reachable": true,
//       "anticipating": null,
//       "open_window": null,
//       "therm_measured_temperature": 15.4,
//       "therm_setpoint_temperature": 12.5,
//       "therm_setpoint_mode": "away",
//       "name": "Łazienka",
//       "type": "bathroom",
//       "battery_state": "full",
//       "battery_level": 4160,
//       "rf_strength": 71
//     },
                    JsonArray rooms = doc["rooms"];
                    for (JsonObject room : rooms)
                    {
                        std::string name = room["name"].as<const char *>();
                        int id = room["id"].as<int>();
                        float currentTemperature = room["therm_measured_temperature"].as<float>();
                        float targetTemperature = room["therm_setpoint_temperature"].as<float>();
                        // Zachowaj wartość forced z istniejącego pokoju, jeśli istnieje
                        bool forced = false;
                        // Sprawdź, czy pokój już istnieje w naszej kolekcji
                        for (const auto &existingRoom : this->rooms) {
                            if (existingRoom.ID == id) {
                                forced = existingRoom.forced;
                                break;
                            }
                        }

                        String battery_state = room["battery_state"].as<const char *>();
                        int battery_level = room["battery_level"].as<int>();
                        int rf_strength = room["rf_strength"].as<int>();
                        String type = room["type"].as<const char *>();
                        bool reachable = room["reachable"].as<bool>();
                        String anticipating = room["anticipating"].as<const char *>();
                        float priority = currentTemperature - targetTemperature;
                        


                        // Sprawdź przypisany pin na podstawie ID
                        int pinNumber = this->idToPinMap[id];
                        Serial.print("ID: ");
                        Serial.print(id);
                        Serial.print(", Pin: ");
                        Serial.println(pinNumber);

                        updateOrAddRoom(RoomData(name, id, pinNumber, targetTemperature, currentTemperature, forced, battery_state, battery_level, rf_strength, reachable, anticipating, priority));
                    }
                }
                else
                {
                    Serial.println("Payload is empty");
                }
            }
            else
            {
                Serial.printf("HTTP GET request failed, error: %s\n", http.errorToString(httpCode).c_str());
            }
            http.end();
        }
        else
        {
            Serial.println("WiFi not connected");
        }

        setRequestInProgress(false);
    }

    std::map<int, int> idToPinMap; // Przenieś mapowanie tutaj

    void updatePinMapping(int roomId, int newPin) {
        idToPinMap[roomId] = newPin;
        // Zaktualizuj też pin w odpowiednim pokoju
        for (auto &room : rooms) {
            if (room.ID == roomId) {
                room.pinNumber = newPin;
                break;
            }
        }
    }

    // Dodaj metodę do serializacji mapowania pinów
    String getPinMappingAsJson() {
        DynamicJsonDocument doc(1024);
        JsonArray mappings = doc.createNestedArray("pinMappings");

        for (const auto &room : rooms) {
            JsonObject mapping = mappings.createNestedObject();
            mapping["roomId"] = room.ID;
            mapping["name"] = room.name;
            mapping["pin"] = room.pinNumber;
        }

        String jsonString;
        serializeJson(doc, jsonString);
        return jsonString;
    }

    // Metoda zwracająca referencję do wektora pokoi
    const std::vector<RoomData>& getAllRooms() const {
        return rooms;
    }

    // Metoda zwracająca liczbę pokoi
    size_t getRoomCount() const {
        return rooms.size();
    }

private:
    std::vector<RoomData> rooms;
    bool requestInProgress;
};