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
    std::string name;         // Nazwa pokoju
    int ID;                   // ID pokoju
    int pinNumber;            // Numer przyporzadkowanego pinu
    float targetTemperature;  // Temperatura zadana
    float currentTemperature; // Temperatura aktualna
    bool forced;              // Czy jest to tryb wymuszony

    RoomData() : name(""), ID(-1), pinNumber(0), targetTemperature(0.0), currentTemperature(0.0), forced(false) {}

    RoomData(const std::string& name, int ID, int pinNumber, float targetTemperature, float currentTemperature, bool forced)
        : name(name), ID(ID), pinNumber(pinNumber), targetTemperature(targetTemperature), currentTemperature(currentTemperature), forced(forced) {}
};

class RoomManager
{
public:
    RoomManager() : requestInProgress(false) {}

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
        if (!newRoom.name.empty())
            existingRoom.name = newRoom.name;
        if (newRoom.ID != -1)
            existingRoom.ID = newRoom.ID;
        if (newRoom.pinNumber != 0)
            existingRoom.pinNumber = newRoom.pinNumber;
        if (newRoom.targetTemperature != 0.0)
            existingRoom.targetTemperature = newRoom.targetTemperature;
        if (newRoom.currentTemperature != 0.0)
            existingRoom.currentTemperature = newRoom.currentTemperature;
        existingRoom.forced = newRoom.forced;
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

        for (const auto &room : rooms)
        {
            JsonObject roomObject = roomsArray.createNestedObject();
            roomObject["name"] = room.name;
            roomObject["id"] = room.ID;
            roomObject["pinNumber"] = room.pinNumber;
            roomObject["targetTemperature"] = room.targetTemperature;
            roomObject["currentTemperature"] = room.currentTemperature;
            roomObject["forced"] = room.forced;
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

                    JsonArray rooms = doc["rooms"];
                    for (JsonObject room : rooms)
                    {
                        std::string name = room["name"].as<const char *>();
                        int id = room["id"].as<int>();
                        float currentTemperature = room["therm_measured_temperature"].as<float>();
                        float targetTemperature = room["therm_setpoint_temperature"].as<float>();
                        bool forced = strcmp(room["therm_setpoint_mode"], "away") == 0;

                        // Sprawdź przypisany pin na podstawie ID
                        int pinNumber = idToPinMap[id];

                        updateOrAddRoom(RoomData(name, id, pinNumber, targetTemperature, currentTemperature, forced));
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

private:
    std::vector<RoomData> rooms;
    bool requestInProgress;
};