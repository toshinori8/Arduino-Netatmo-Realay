#include <Arduino.h>
#include <ArduinoOTA.h>
#include "PCF8574.h"
#include <Wire.h>
#include "Timers.h"
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <espnow.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <IotWebConf.h>
#include <EEPROM.h> // Add EEPROM library

#ifndef IOTWEBCONF_ENABLE_JSON
#error platformio.ini must contain "build_flags = -DIOTWEBCONF_ENABLE_JSON"
#endif

StaticJsonDocument<8192> docPins;
StaticJsonDocument<1124> docRooms;
StaticJsonDocument<200> docInput;

#include <functional>
#include <webPage.h>
#include <roomManager.h>
#include <now.h>

// --- EEPROM Settings ---
#define MAX_ROOMS_EEPROM 6 // Max rooms to save in EEPROM (Adjusted to 6 based on user feedback)

// Define EEPROM Addresses
const int ADDR_MAGIC = 0;
const int ADDR_USE_GAZ = ADDR_MAGIC + sizeof(byte);
const int ADDR_ROOM_COUNT = ADDR_USE_GAZ + sizeof(bool);
const int ADDR_ROOM_DATA_START = ADDR_ROOM_COUNT + sizeof(byte);

// Calculate size needed for one room's data
const int ROOM_DATA_SIZE = sizeof(int) + sizeof(int) + sizeof(bool) + sizeof(float); // id, pin, forced, temp (4+4+1+4 = 13 bytes)

// Calculate total EEPROM size needed
// const int EEPROM_SIZE = ADDR_ROOM_DATA_START + (MAX_ROOMS_EEPROM * ROOM_DATA_SIZE); // 3 + (6 * 13) = 81 bytes
const int EEPROM_SIZE = 62; // Use 62 bytes as requested by user

#define MAX_ROOMS_EEPROM 4 // Max rooms to save in EEPROM (Adjusted to 4 based on 62-byte limit)

const byte EEPROM_MAGIC_VALUE = 0xAB; // Choose a magic byte value

/* // Old struct definitions (replaced by individual reads/writes)
struct RoomSettings {
    int id;
    int pinNumber;
    bool forced;
    float targetTemperatureFireplace;
};
struct EepromSettings {
    byte magic;
    bool useGaz;
    byte roomCount;
    RoomSettings rooms[MAX_ROOMS_EEPROM];
};
*/
// --- End EEPROM Settings ---

const char thingName[] = "Netatmo_Relay";
const char wifiInitialApPassword[] = "pmgana921";

// Dodaj domyślną konfigurację WiFi
const char iwcWifiSsid[] = "oooooio";
const char iwcWifiPassword[] = "pmgana921";

// Dodaj na początku pliku po innych stałych
const int CONFIG_PIN = 0; // GPIO0 - przycisk FLASH na ESP8266
unsigned long buttonDownTime = 0;
const int CONFIG_RESET_TIMEOUT = 10000; // 10 sekund przytrzymania przycisku

void handleRoot();

String forced;
String woodStove;
String state;
String pin;
bool useGaz_ = false; // use gas? button on webPage - Will be loaded from EEPROM
unsigned long lastSendTime = 0;
unsigned long previousMillis = 0; // Variable to store the previous time
// const long interval = 480 * 60 * 1000; // Interval at which to reset the NodeMCU

WebSocketsServer webSocket = WebSocketsServer(81);
DNSServer dnsServer;
WebServer server(80);
IotWebConf iotWebConf(thingName, &dnsServer, &server, wifiInitialApPassword);

RoomManager manager;

void prepareDataForWebServer()
{

  docPins["pins"]["pin_0"]["state"] = "OFF";
  docPins["pins"]["pin_0"]["forced"] = "false";

  docPins["pins"]["pin_1"]["state"] = "OFF";
  docPins["pins"]["pin_1"]["forced"] = "false";

  docPins["pins"]["pin_2"]["state"] = "OFF";
  docPins["pins"]["pin_2"]["forced"] = "false";

  docPins["pins"]["pin_3"]["state"] = "OFF";
  docPins["pins"]["pin_3"]["forced"] = "false";

  docPins["WoodStove"] = "off";
  docPins["manifoldTemp"] = "";
  docPins["netatmoData"] = "";
  docPins["usegaz"] = useGaz_ ? "true" : "false"; // Initialize based on loaded/default useGaz_
}

// --- EEPROM Save/Load Functions ---

// Saves current settings (useGaz and room data) to EEPROM using individual writes, single commit
void saveSettings(RoomManager &mgr, bool currentUseGaz)
{
  Serial.println("Saving settings to EEPROM (individual fields)...");

  // 1. Write Magic Byte
  EEPROM.put(ADDR_MAGIC, EEPROM_MAGIC_VALUE);
  Serial.printf("  Putting Magic Byte: 0x%X at Addr: %d\n", EEPROM_MAGIC_VALUE, ADDR_MAGIC);
  if (!EEPROM.commit())
  {
    Serial.println("ERROR! EEPROM commit failed after magic byte");
  }

  // 2. Write useGaz flag
  EEPROM.put(ADDR_USE_GAZ, currentUseGaz);
  Serial.printf("  Putting useGaz: %s at Addr: %d\n", currentUseGaz ? "true" : "false", ADDR_USE_GAZ);
  if (!EEPROM.commit())
  {
    Serial.println("ERROR! EEPROM commit failed after useGaz");
  }

  // 3. Write Room Count
  const std::vector<RoomData> &currentRooms = mgr.getAllRooms();
  byte roomCountToSave = min((byte)currentRooms.size(), (byte)MAX_ROOMS_EEPROM);
  EEPROM.put(ADDR_ROOM_COUNT, roomCountToSave);
  Serial.printf("  Putting roomCount: %d at Addr: %d\n", roomCountToSave, ADDR_ROOM_COUNT);
  if (!EEPROM.commit())
  {
    Serial.println("ERROR! EEPROM commit failed after room count");
  }
  Serial.printf("Saving %d rooms to EEPROM.\n", roomCountToSave);

  // 4. Write Room Data
  int currentAddr = ADDR_ROOM_DATA_START;
  for (byte i = 0; i < roomCountToSave; ++i)
  {
    int roomId = currentRooms[i].ID;
    int roomPin = currentRooms[i].pinNumber;
    bool roomForced = currentRooms[i].forced;
    float roomFireplaceTemp = currentRooms[i].targetTemperatureFireplace;

    Serial.printf("  Putting Room ID: %d, Pin: %d, Forced: %s, Fireplace Temp: %.1f at Addr: %d\n",
                  roomId, roomPin, roomForced ? "true" : "false", roomFireplaceTemp, currentAddr);

    EEPROM.put(currentAddr, roomId);
    Serial.printf("  Putting Room ID: %d at Addr: %d\n", roomId, currentAddr);
    currentAddr += sizeof(int);

    EEPROM.put(currentAddr, roomPin);
    Serial.printf("  Putting Room Pin: %d at Addr: %d\n", roomPin, currentAddr - sizeof(int));
    currentAddr += sizeof(int);

    EEPROM.put(currentAddr, roomForced);
    Serial.printf("  Putting Room Forced: %s at Addr: %d\n", roomForced ? "true" : "false", currentAddr - sizeof(int));
    currentAddr += sizeof(bool);

    EEPROM.put(currentAddr, roomFireplaceTemp);
    Serial.printf("  Putting Room FireplaceTemp: %.1f at Addr: %d\n", roomFireplaceTemp, currentAddr - sizeof(float));
    currentAddr += sizeof(float);
  }
  Serial.printf("Finished putting fields. Last write Addr: %d. Attempting commit...\n", currentAddr - sizeof(float)); // Debug last address used

  // 5. Commit all changes once at the end
  if (EEPROM.commit())
  {
    Serial.println("EEPROM successfully committed");
  }
  else
  {
    Serial.println("ERROR! EEPROM commit failed after writing all data.");
    // Note: currentAddr here is the address *after* the last write.
    // The actual failure point during commit isn't directly reported by the library.
  }
}

// Loads settings from EEPROM using individual reads and updates manager and useGaz_
// Returns true if successful (magic byte matched), false otherwise
bool loadSettings(RoomManager &mgr)
{
  Serial.println("Loading settings from EEPROM (individual fields)...");

  // 1. Read and Check Magic Byte
  byte magic = 0; // Initialize to prevent compiler warning
  EEPROM.get(ADDR_MAGIC, magic);
  if (magic != EEPROM_MAGIC_VALUE)
  {
    Serial.println("EEPROM magic byte mismatch! Using default settings.");
    return false; // Indicate failure to load valid settings
  }
  Serial.println("EEPROM magic byte matched. Applying settings.");

  // 2. Read useGaz flag
  EEPROM.get(ADDR_USE_GAZ, useGaz_);
  docPins["usegaz"] = useGaz_ ? "true" : "false"; // Update JSON doc
  Serial.printf("Loaded useGaz_ = %s\n", useGaz_ ? "true" : "false");

  // 3. Read Room Count
  byte roomCountToLoad = 0; // Initialize to prevent compiler warning
  EEPROM.get(ADDR_ROOM_COUNT, roomCountToLoad);
  Serial.printf("Loading %d rooms from EEPROM.\n", roomCountToLoad);

  // 4. Read Room Data
  int currentAddr = ADDR_ROOM_DATA_START;
  for (byte i = 0; i < roomCountToLoad; ++i)
  {
    int loadedId;
    int loadedPin;
    bool loadedForced;
    float loadedFireplaceTemp;

    EEPROM.get(currentAddr, loadedId);
    currentAddr += sizeof(int);
    EEPROM.get(currentAddr, loadedPin);
    currentAddr += sizeof(int);
    EEPROM.get(currentAddr, loadedForced);
    currentAddr += sizeof(bool);
    EEPROM.get(currentAddr, loadedFireplaceTemp);
    currentAddr += sizeof(float);

    Serial.printf("  Room ID: %d, Pin: %d, Forced: %s, Fireplace Temp: %.1f at Addr: %d\n",
                  loadedId, loadedPin, loadedForced ? "true" : "false", loadedFireplaceTemp, ADDR_ROOM_DATA_START + i * ROOM_DATA_SIZE);
    // Update pin mapping in the manager FIRST
    mgr.idToPinMap[loadedId] = loadedPin;

    // Now try to update the room data if it exists (it might not exist yet if fetch hasn't run)
    bool roomFound = false;
    // Get a mutable reference to the rooms vector
    std::vector<RoomData> &mgrRooms = const_cast<std::vector<RoomData> &>(mgr.getAllRooms());
    for (auto &room : mgrRooms)
    {
      if (room.ID == loadedId)
      {
        room.pinNumber = loadedPin; // Update pin number based on loaded data
        room.forced = loadedForced;
        room.targetTemperatureFireplace = loadedFireplaceTemp;
        Serial.printf("  Applied settings to existing room %d\n", loadedId);
        roomFound = true;
        break;
      }
    }
    if (!roomFound)
    {
      // If the room doesn't exist in the manager's list yet,
      // we still updated the pin map. The fetchJsonData function, when it runs,
      // should create the room. The updateOrAddRoom logic needs to be checked
      // to ensure it preserves the loaded fireplace temp and forced status
      // if the fetched data doesn't override them.
      // Let's create a placeholder room now to ensure the data is stored.
      // We'll use default values for fields not stored in EEPROM.
      Serial.printf("  Room %d not found in manager, creating placeholder with loaded settings.\n", loadedId);
      RoomData placeholderRoom;
      placeholderRoom.ID = loadedId;
      placeholderRoom.pinNumber = loadedPin;
      placeholderRoom.forced = loadedForced;
      placeholderRoom.targetTemperatureFireplace = loadedFireplaceTemp;
      // Set other fields to defaults or leave them as initialized by RoomData()
      placeholderRoom.name = std::string("Loaded Room ") + std::to_string(loadedId); // Placeholder name
      mgr.addRoom(placeholderRoom);                                                  // Add the placeholder
    }
  }

  return true; // Indicate successful load
}
// --- End EEPROM Save/Load Functions ---

void handleRoot()
{
  if (iotWebConf.handleCaptivePortal())
  {
    return;
  }
  server.send(200, "text/html", webpage);
}

void fetchNetatmo()
{
  manager.fetchJsonData(api_url);
}

// utworzenie obiektu klasy Timers z trzema odliczającymi
Timers<4> timers;

// obiekty ekspanderów PCF8574
PCF8574 ExpInput(0x20);  // utworzenie obiektu dla pierwszego ekspandera
PCF8574 ExpOutput(0x26); // utworzenie obiektu dla drugiego ekspandera

// AHT10 INIT;
#include <Adafruit_AHTX0.h>
Adafruit_AHTX0 aht;
sensors_event_t humidity, temp;
  
void readAHT()
{
  aht.getEvent(&humidity, &temp);
  // acces temperature and humidity
//manifoldTemp
  
 docPins["manifoldTemp"]=String(temp.temperature);
   
}

void otaStart();
void initInputExpander()
{
  // ustawienie pinów jako wejścia i włączenie wbudowanych rezystorów podciągających

  ExpInput.pinMode(P0, INPUT_PULLUP); // netatmo relays input pin
  ExpInput.pinMode(P1, INPUT_PULLUP); // netatmo relays input pin
  ExpInput.pinMode(P2, INPUT_PULLUP); // netatmo relays input pin
  ExpInput.pinMode(P3, INPUT_PULLUP); // netatmo relays input pin
  ExpInput.pinMode(P4, INPUT_PULLUP); // netatmo relays input pin
  ExpInput.pinMode(P5, INPUT_PULLUP); // netatmo relays input pin

  ExpInput.pinMode(P6, INPUT_PULLUP); // Gaz boiler
  ExpInput.pinMode(P7, INPUT_PULLUP); // Wodstove pump

  ExpInput.digitalWrite(P0, HIGH); // Relay Valve
  ExpInput.digitalWrite(P1, HIGH); // Relay Valve
  ExpInput.digitalWrite(P2, HIGH); // Relay Valve
  ExpInput.digitalWrite(P3, HIGH); // Relay Valve
  ExpInput.digitalWrite(P4, HIGH); // Relay Valve
  ExpInput.digitalWrite(P5, HIGH); // Relay Valve

  ExpInput.digitalWrite(P6, HIGH); // Gaz boiler
  ExpInput.digitalWrite(P7, HIGH); // Wodstove pump
};
void initOutputExpander()
{
  for (int i = 0; i < 7; i++)
  {

    ExpOutput.pinMode(i, OUTPUT);
    ExpOutput.digitalWrite(i, HIGH);
  }
};
void blinkOutput(int timer)
{
  for (int i = 0; i < 7; i++)
  {
    ExpOutput.digitalWrite(i, LOW);
    delay(timer);
    ExpOutput.digitalWrite(i, HIGH);
  }
}

// Obsługa Websocket
void onWsEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{
  switch (type)
  {
  case WStype_DISCONNECTED:
    // handle client disconnection
    Serial.printf("Client %u disconnected from WebSocket\n", num);
    break;
  case WStype_CONNECTED:
  {
    // handle client connection
    IPAddress ip = webSocket.remoteIP(num);
    Serial.printf("Client %u connected from %s\n", num, ip.toString().c_str());
    String message = "{\"response\":\"connected\"}";
    webSocket.sendTXT(num, message);
  }
  break;
  case WStype_TEXT:
  {

    // handle incoming text message from client
    String messageText = String((char *)payload).substring(0, length);
    Serial.println("Message from client: " + messageText);
    // parse the JSON message
    DeserializationError error = deserializeJson(docInput, messageText);
    if (error)
    {
      Serial.println("Error parsing JSON");
      return;
    }
    else
    {
      Serial.println("JSON parsed");
    }
    //  {"id":206653929,"command":"act_temperature","targetTemperature":"15.5","forced":false}
    // {"id":1812451076,"command":"act_temperature","targetTemperature":"26.5","forced":false}

    Serial.println(messageText);
    //{"command":"usegaz","value":"true"}
    if (docInput["command"] == "usegaz")
    {
      Serial.println("usegaz - detected : " + String(docInput["usegaz"].as<const char *>()));
      Serial.println(docInput["usegaz"].as<const char *>());

      if (docInput["command"] == "usegaz" && docInput["value"] == "true")
      {
        useGaz_ = true;
        docPins["usegaz"] = "true";
      }
      else
      {
        useGaz_ = false;
        docPins["usegaz"] = "false";
      }
      saveSettings(manager, useGaz_); // Save after change
    }

    if (docInput["command"] == "act_temperature")
    {
      int id = docInput["id"];
      float targetTemperatureNetatmo = docInput["targetTemperature"]; // Value from Slider 1
      // This command always updates the Netatmo target
      manager.setTemperature(id, targetTemperatureNetatmo);
      // No need to check usegaz here, setTemperature handles the Netatmo update always.
    }

    // New command handler for Slider 2 (Fireplace Target)
    if (docInput["command"] == "set_fireplace_target")
    {
      int id = docInput["id"];
      float targetTemperatureFireplace = docInput["targetTemperatureFireplace"]; // Value from Slider 2
      manager.setFireplaceTemperature(id, targetTemperatureFireplace);           // Update local fireplace target only
      saveSettings(manager, useGaz_);                                            // Save after change
    }

    // }
    if (docInput["command"] == "forced")
    {

      int id = docInput["id"];

      // Znajdz Room ktorego ID == id
      RoomData room = manager.getRoomByID(id);
      // get pinNumber from room
      int pinNumber = room.pinNumber;
      String pin = "pin_" + String(pinNumber);

      // float targetTemperature = docInput["targetTemperature"]; // This might be ambiguous now
      bool forced = docInput["forced"];

      // if (forced ==0){forced="false";}else(forced="true");
      // Update the forced status directly in the manager
      // We don't need to update temperature here as that's handled by separate commands
      RoomData updatedRoom = manager.getRoomByID(id); // Get existing data
      if (updatedRoom.ID != -1)
      {                                       // Check if room exists
        updatedRoom.forced = forced;          // Update only the forced status
        manager.updateOrAddRoom(updatedRoom); // Use updateOrAddRoom to save the change
        Serial.printf("Forced status for room %d set to %s\n", id, forced ? "true" : "false");
        saveSettings(manager, useGaz_); // Save after change
      }
      else
      {
        Serial.printf("Error: Room %d not found when trying to set forced status.\n", id);
      }

      // Update docPins for immediate UI feedback (though manifoldLogicNew will also update it)
      // Ensure pinNumber is valid before accessing docPins
      if (pinNumber >= 0 && pinNumber < 6)
      { // Assuming pins 0-5 are for rooms
        String pinStr = "pin_" + String(pinNumber);
        if (forced)
        {
          docPins["pins"][pinStr]["state"] = "ON"; // Tentative state, logic will confirm
          docPins["pins"][pinStr]["forced"] = "true";
        }
        else
        {
          docPins["pins"][pinStr]["state"] = "OFF";
          docPins["pins"][pinStr]["forced"] = "false";
        }
      }

      if (forced)
      {
        docPins["pins"][pin]["state"] = "ON";
        docPins["pins"][pin]["forced"] = "true";
      }
      else
      {
        docPins["pins"][pin]["state"] = "OFF";
        docPins["pins"][pin]["forced"] = "false";
      }
    }

    // Dodaj obsługę nowych komend
    if (docInput["command"] == "getPinMappings")
    {
      String mappings = manager.getPinMappingAsJson();
      webSocket.sendTXT(num, mappings);
    }
    else if (docInput["command"] == "updatePin")
    {
      int roomId = docInput["roomId"];
      int newPin = docInput["pin"];
      manager.updatePinMapping(roomId, newPin);
      saveSettings(manager, useGaz_); // Save after change

      // Potwierdź aktualizację
      String response = "{\"response\":\"pinUpdated\",\"roomId\":" + String(roomId) +
                        ",\"pin\":" + String(newPin) + "}";
      webSocket.sendTXT(num, response);
    }

    // Send acknowledgment back to client
    String ackMessage = "{\"response\":\"acknowledged\"}";
    webSocket.sendTXT(num, ackMessage);
  }
  break;
  }
}

// WYSYŁANIE ROOMS PRZEZ WSSOCKET
void broadcastWebsocket()
{
  String data = manager.getRoomsAsJson();
  webSocket.broadcastTXT(data);
}

// relayMode LOW/HIGH = ON/OFF function
void relayMode(uint8_t state)
{
  for (int i = 0; i < 6; i++)
  {
    ExpOutput.digitalWrite(i, state); // LOW = ON, HIGH = OFF
  }
}

void manifoldLogicNew()
{

  // --- Room Heating Logic ---
  const std::vector<RoomData> &rooms = manager.getAllRooms();
  std::vector<int> forcedRoomsNeedingHeatIDs; // Store IDs of forced rooms needing heat
  int primaryRoomId = -1;
  float lowestTemp = 100.0;
  int secondaryRoomId = -1;
  float smallestPositiveDifference = 100.0;

  // --- First Pass: Identify forced rooms needing heat and find the primary candidate ---
  for (const auto &room : rooms)
  {
    if (!room.forced)
      continue; // Skip non-forced rooms

    // Determine effective target temperature
    float effectiveTargetTemperature;
    if (useGaz_)
    {
      effectiveTargetTemperature = max(room.targetTemperatureNetatmo, room.targetTemperatureFireplace);
    }
    else
    {
      effectiveTargetTemperature = room.targetTemperatureFireplace;
    }

    // Check if room needs heating
    if (room.currentTemperature < effectiveTargetTemperature)
    {
      forcedRoomsNeedingHeatIDs.push_back(room.ID); // Add to list

      // Check if this room has the lowest temperature so far
      if (room.currentTemperature < lowestTemp)
      {
        lowestTemp = room.currentTemperature;
        primaryRoomId = room.ID;
      }
    }
  }

  // --- Second Pass (if primary found): Find the secondary candidate ---
  if (primaryRoomId != -1)
  {
    for (int roomId : forcedRoomsNeedingHeatIDs)
    {
      if (roomId == primaryRoomId)
        continue; // Skip the primary room

      // Find the room data again (could optimize by storing pointers/references)
      for (const auto &room : rooms)
      {
        if (room.ID == roomId)
        {
          // Determine effective target temperature again (could optimize)
          float effectiveTargetTemperature;
          if (useGaz_)
          {
            effectiveTargetTemperature = max(room.targetTemperatureNetatmo, room.targetTemperatureFireplace);
          }
          else
          {
            effectiveTargetTemperature = room.targetTemperatureFireplace;
          }

          float difference = effectiveTargetTemperature - room.currentTemperature;
          if (difference > 0 && difference < smallestPositiveDifference)
          { // Must need heat and be smallest diff
            smallestPositiveDifference = difference;
            secondaryRoomId = roomId;
          }
          break; // Found the room data
        }
      }
    }
  }

  // --- Control Relays ---
  Serial.println("--- Heating Logic ---");
  // Turn OFF all room relays initially
  for (int i = 0; i < 6; i++)
  {
    ExpOutput.digitalWrite(i, HIGH); // LOW = OFF
    docPins["pins"]["pin_" + String(i)]["state"] = "OFF";
  }

  // Activate primary room relay
  if (primaryRoomId != -1)
  {
    for (const auto &room : rooms)
    {
      if (room.ID == primaryRoomId)
      {
        if (room.pinNumber >= 0 && room.pinNumber < 6)
        {
          relayMode(LOW);
          ExpOutput.digitalWrite(room.pinNumber, HIGH); // HIGH = ON
          docPins["pins"]["pin_" + String(room.pinNumber)]["state"] = "ON";
          Serial.printf("Primary heating ON: Room %s (Pin %d, Temp %.1f, Lowest Temp)\n",
                        room.name.c_str(), room.pinNumber, room.currentTemperature);
        }
        break;
      }
    }
  }
  else
  {
    Serial.println("No primary forced room needs heating.");
  }

  // Activate secondary room relay
  if (secondaryRoomId != -1)
  {
    for (const auto &room : rooms)
    {
      if (room.ID == secondaryRoomId)
      {
        if (room.pinNumber >= 0 && room.pinNumber < 6)
        {
          // Check if it's the same pin as primary - avoid double logging if so
          if (primaryRoomId == -1 || room.pinNumber != manager.getRoomByID(primaryRoomId).pinNumber)
          {
            //  relayMode(LOW);
            ExpOutput.digitalWrite(room.pinNumber, HIGH); // HIGH = OFF
                                                          //  docPins["pins"]["pin_" + String(room.pinNumber)]["state"] = "ON";
            Serial.printf("Secondary heating ON: Room %s (Pin %d, Temp %.1f, Smallest Diff %.1f)\n",
                          room.name.c_str(), room.pinNumber, room.currentTemperature, smallestPositiveDifference);
          }
          else
          {
            Serial.printf("Secondary room (%s) shares pin with primary. Already ON.\n", room.name.c_str());
          }
        }
        break;
      }
    }
  }
  else if (primaryRoomId != -1)
  {
    Serial.println("No suitable secondary forced room found.");
    relayMode(HIGH);
  }

  // --- Gas/Pump Control ---

  // if there is forced room, is primary room
  if (primaryRoomId != -1 && useGaz_ == true)
  {

    ExpOutput.digitalWrite(P6, LOW);
    ExpOutput.digitalWrite(P7, LOW);

    Serial.println("Gas mode ON - P6/P7 ON");
  }
  // ONLY KOMINEK
  else if (primaryRoomId != -1 && useGaz_ == false)
  {
    ExpOutput.digitalWrite(P6, HIGH);
    ExpOutput.digitalWrite(P7, LOW);

    Serial.println("Gas mode OFF - P6 OFF / P7 ON ");
  }
  // else if no primary room, turn off gas/pump
  else if (primaryRoomId == -1 && useGaz_ == false)
  {
    ExpOutput.digitalWrite(P6, HIGH);
    ExpOutput.digitalWrite(P7, HIGH);

    Serial.println("Gas mode OFF NO HEATING - P6/P7 OFF");
  }

  // --- Update docPins forced status and final room info ---
  for (const auto &room : rooms)
  {
    if (room.pinNumber >= 0 && room.pinNumber < 6)
    {
      docPins["pins"]["pin_" + String(room.pinNumber)]["forced"] = room.forced ? "true" : "false";
    }
  }
  docPins["roomsInfo"] = manager.getRoomsAsJson(); // Send updated state including valve status
  Serial.println("--- End Heating Logic ---");
}

void readInitWifiConfig()
{
  // Tworzymy JSON w pamięci
  StaticJsonDocument<1024> doc;

  // Tworzymy strukturę JSON
  JsonObject iwcAll = doc.createNestedObject("iwcAll");
  JsonObject iwcSys = iwcAll.createNestedObject("iwcSys");

  // Ustawiamy wartości
  iwcSys["iwcThingName"] = thingName;
  iwcSys["iwcApPassword"] = wifiInitialApPassword;
  iwcSys["iwcApTimeout"] = "0";

  // Konfiguracja WiFi
  JsonObject iwcWifi0 = iwcSys.createNestedObject("iwcWifi0");
  iwcWifi0["iwcWifiSsid"] = iwcWifiSsid;
  iwcWifi0["iwcWifiPassword"] = iwcWifiPassword;
  JsonObject documentRoot = doc.as<JsonObject>();

  // Aplikujemy konfigurację
  iotWebConf.getRootParameterGroup()->loadFromJson(documentRoot);
  iotWebConf.saveConfig();
}

void setup()
{

  pinMode(CONFIG_PIN, INPUT_PULLUP);
  Serial.begin(115200);
  while (!Serial)
  {
    delay(10);
  } // Wait for serial connection
  Serial.println("\nStarting up...");


if (!aht.begin()) {
    Serial.println("Nie wykryto AHT10!");
    while (1) delay(10);
  }


  // -- Initialize EEPROM --
  EEPROM.begin(EEPROM_SIZE); // Use the requested size (512)
  Serial.printf("EEPROM size requested: %d bytes\n", EEPROM_SIZE);

  // -- Load settings or initialize EEPROM --
  if (!loadSettings(manager))
  {
    // EEPROM not initialized or data corrupted, save defaults
    Serial.println("Initializing EEPROM with default settings...");
    useGaz_ = false; // Default value
    // Manager defaults (like initial pin map) are set in its constructor.
    // We save the current state which includes these defaults.
    saveSettings(manager, useGaz_);
  }
  // Update docPins based on potentially loaded useGaz_ value AFTER loading attempt
  docPins["usegaz"] = useGaz_ ? "true" : "false";

  // -- Initializing the network configuration --
  readInitWifiConfig(); // Reads default WiFi creds, might be overwritten by saved config
  // Dodaj callback-i dla IotWebConf
  /*  iotWebConf.setWifiConnectionCallback(&wifiConnected);
   iotWebConf.setConfigSavedCallback(&configSaved); */

  iotWebConf.setConfigPin(CONFIG_PIN);
  iotWebConf.setStatusPin(LED_BUILTIN);

  delay(3000);

  iotWebConf.setApTimeoutMs(0);
  iotWebConf.init();
  // -- Set up required URL handlers on the web server.
  server.on("/", handleRoot);
  server.on("/config", []
            { iotWebConf.handleConfig(); });

  server.onNotFound([]()
                    { iotWebConf.handleNotFound(); });
  Serial.println("Ready.");

  // uruchomienie serwera HTTP
  server.begin();
  Serial.println("HTTP server started");
  // przygotowanie zmienych do odbioru z webpage
  prepareDataForWebServer();

  // uruchomienie serwera WebSocket
  webSocket.begin();
  webSocket.onEvent(onWsEvent);
  Serial.println("WebSocket server started");

  //  ustawienie pinów jako wejścia i włączenie wbudowanych rezystorów podciągających
  initInputExpander();
  initOutputExpander();

  // INICJALIZACJA PCF
  Serial.print("Init input Expander...");
  if (ExpInput.begin())
  {
    Serial.println("OK");
  }
  else
  {
    Serial.println("error");
  }

  Serial.print("Init output Expander...");
  if (ExpOutput.begin())
  {
    Serial.println("OK");
  }
  else
  {
    Serial.println("error");
  }

  otaStart();
  initInputExpander();
  blinkOutput(20);
  // Inicjalizacja ESP-NOW
  if (esp_now_init() != ERR_OK)
  {
    Serial.println("Błąd inicjalizacji ESP-NOW");
    return;
  }
  esp_now_register_recv_cb(onDataRecv);

#if defined(ESP8266) || defined(ESP32)
  int eepromSize = 512; // Change based on board
  EEPROM.begin(eepromSize);
#endif

  Serial.println("EEPROM content:");
  for (int i = 0; i < 64; i++)
  {
    Serial.print("Address ");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(EEPROM.read(i));
  }

  // Inicjalizacja timera
  timers.attach(0, 65000, fetchNetatmo);
  timers.attach(1, 12000, broadcastWebsocket);
  timers.attach(2, 20000, manifoldLogicNew);
  timers.attach(4, 50000, readAHT);
}

void loop()
{
  // Sprawdzenie, czy upłynęło 20 minut od ostatniego restartu
  // if (currentMillis - previousMillis >= interval) {
  //   ESP.restart();  // Restart the NodeMCU
  //   previousMillis = currentMillis;
  // }
  // Sprawdź czy przycisk jest wciśnięty (stan niski bo INPUT_PULLUP)
  if (digitalRead(CONFIG_PIN) == LOW)
  {
    if (buttonDownTime == 0) // Przycisk został właśnie wciśnięty
    {

      buttonDownTime = millis();
    }
    else if ((millis() - buttonDownTime) > CONFIG_RESET_TIMEOUT)
    {

      iotWebConf.getSystemParameterGroup()->applyDefaultValue(); // Reset do wartości domyślnych
      iotWebConf.saveConfig();                                   // Zapisz pustą konfigurację
      ESP.restart();                                             // Zrestartuj urządzenie */
    }
  }
  else
  {
    buttonDownTime = 0; // Reset licznika gdy przycisk puszczony
  }

  iotWebConf.doLoop();

  // jeśli nawiązano połączenie z siecią
  if (iotWebConf.getState() == 4)
  {

    webSocket.loop();
    ArduinoOTA.handle();

    timers.process();
  }
}
