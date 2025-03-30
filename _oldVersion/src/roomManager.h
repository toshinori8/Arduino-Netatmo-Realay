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

// Mapowanie ID na piny
std::map<int, int> idToPinMap = {
    {1812451076, 1},
    {206653929, 2},
    {1868270675, 3},
    {38038562, 4}
};

struct RoomData
{
   
  /*  {
      id: '38038562',
      reachable: true,
      anticipating: null,

      therm_measured_temperature: 10.8,
      therm_setpoint_temperature: 28,
      therm_setpoint_mode: 'manual',
      name: 'Waleria',
      type: 'custom',
      battery_state: 'high',
      battery_level: 3782,
      rf_strength: 40
    }
   
 */

<<<<<<< HEAD:src_old/roomManager.h
struct RoomData
{
=======
>>>>>>> main:src/roomManager.h
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



    RoomData() : name(""), ID(-1), pinNumber(0), targetTemperature(0.0), currentTemperature(0.0), forced(false), battery_state(""), battery_level(0), rf_strength(0), reachable(false), anticipating("") {}

    RoomData(const std::string& name, int ID, int pinNumber, float targetTemperature, float currentTemperature, bool forced, const String& battery_state, int battery_level, int rf_strength, bool reachable, const String& anticipating)
        : name(name), ID(ID), pinNumber(pinNumber), targetTemperature(targetTemperature), currentTemperature(currentTemperature), forced(forced), battery_state(battery_state), battery_level(battery_level), rf_strength(rf_strength), reachable(reachable), anticipating(anticipating) {}
};

class RoomManager
{
public:
<<<<<<< HEAD:src_old/roomManager.h
=======
    RoomManager() : requestInProgress(false) {}

>>>>>>> main:src/roomManager.h
    void addRoom(const RoomData &room)
    {
        rooms.push_back(room);
        Serial.print("Added room: ");
        Serial.println(room.name.c_str());
    }

<<<<<<< HEAD:src_old/roomManager.h
    RoomData getRoom(int index)
    {
        if (index >= 0 && index < rooms.size())
=======
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



        if(docPins["usegaz"] == "true")
        {
            if (newRoom.targetTemperature != 0.0)
            existingRoom.targetTemperature = newRoom.targetTemperature;
        }


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
    }

    RoomData getRoom(size_t index)
    {
        if (index < rooms.size())
>>>>>>> main:src/roomManager.h
        {
            return rooms[index];
        }
        else
        {
<<<<<<< HEAD:src_old/roomManager.h
            // Wyrzuć wyjątek lub zwróć pustą strukturę
            throw std::out_of_range("Index out of range");
        }
    }

    void updateRoom(int index, const RoomData &room)
    {
        if (index >= 0 && index < rooms.size())
=======
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
>>>>>>> main:src/roomManager.h
        {
            rooms[index] = room;
        }
        else
        {
<<<<<<< HEAD:src_old/roomManager.h
            // Wyrzuć wyjątek lub zwróć pustą strukturę
            throw std::out_of_range("Index out of range");
        }
    }

    void printRooms()
    {
        for (const auto &room : rooms)
        {
            std::cout << "Nazwa pokoju: " << room.name << std::endl;
            std::cout << "Numer pinu: " << room.pinNumber << std::endl;
            std::cout << "Temperatura zadana: " << room.targetTemperature << std::endl;
            std::cout << "Temperatura aktualna: " << room.currentTemperature << std::endl;
            std::cout << "Tryb wymuszony: " << (room.forced ? "Tak" : "Nie") << std::endl;
            std::cout << std::endl;
=======
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

        for (const auto &room : rooms)
        {
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
            
>>>>>>> main:src/roomManager.h
        }
         
         // Kopiowanie docPins do meta w docx
        JsonObject meta = docx.createNestedObject("meta");
        meta.set(docPins.as<JsonObject>());


        String jsonString;
        
        serializeJson(docx, jsonString);
        Serial.println(jsonString);
        return jsonString;
    }

    void setTemperature(int roomID, float setTemperature)
    {
   
       

        Serial.printf("Setting temperature on \n", roomID, setTemperature);
   
            if (WiFi.status() == WL_CONNECTED)
        {
            WiFiClient client;

            HTTPClient http;
   
            String url = "http://netatmo.adamkarski.art/setRoomTemperature?mode=manual&temperature=" + String(setTemperature) + "&room_id=" + String(roomID);
            http.begin(client, url);   
            int httpCode = http.GET();

            Serial.printf("HTTP GET request code: %d\n", httpCode);
            Serial.printf("%s", url.c_str());




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
                Serial.println("Received payload:");

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
                        bool forced = strcmp(room["therm_setpoint_mode"], "away") == 0;

                        String battery_state = room["battery_state"].as<const char *>();
                        int battery_level = room["battery_level"].as<int>();
                        int rf_strength = room["rf_strength"].as<int>();
                        String type = room["type"].as<const char *>();
                        bool reachable = room["reachable"].as<bool>();
                        String anticipating = room["anticipating"].as<const char *>();



                        // Sprawdź przypisany pin na podstawie ID
                        int pinNumber = idToPinMap[id];

                        updateOrAddRoom(RoomData(name, id, pinNumber, targetTemperature, currentTemperature, forced, battery_state, battery_level, rf_strength, reachable, anticipating));
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

    

private:
    std::vector<RoomData> rooms;
<<<<<<< HEAD:src_old/roomManager.h
};

// int main() {
//     RoomManager manager;

//     // Dodaj pokoje
//     manager.addRoom(RoomData("Pokój 1", 1, 20.0f, 22.0f, false));
//     manager.addRoom(RoomData("Pokój 2", 2, 22.0f, 24.0f, true));
//     manager.addRoom(RoomData("Pokój 3", 3, 20.0f, 22.0f, false));

//     // Wypisz pokoje
//     manager.printRooms();

//     // Zmień dane pokoju
//     RoomData room = manager.getRoom(1);
//     room.targetTemperature = 23.0f;
//     manager.updateRoom(1, room);

//     // Wypisz pokoje ponownie
//     manager.printRooms();

//     return 0;
// }
=======
    bool requestInProgress;
};
>>>>>>> main:src/roomManager.h
