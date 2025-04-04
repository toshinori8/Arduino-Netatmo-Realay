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

#ifndef IOTWEBCONF_ENABLE_JSON
# error platformio.ini must contain "build_flags = -DIOTWEBCONF_ENABLE_JSON"
#endif

StaticJsonDocument<8192> docPins;
StaticJsonDocument<1124> docRooms;
StaticJsonDocument<200> docInput;

#include <functional>
#include <webPage.h>
#include <roomManager.h>
#include <now.h>


void readConfigFile();





const char thingName[] = "netatmo_relay";
const char wifiInitialApPassword[] = "12345678";
// Dodaj domyślną konfigurację WiFi
const char iwcWifiSsid[] = "oooooio";
const char iwcWifiPassword[] = "pmgana921";


void handleRoot();

String forced;
String woodStove;
String state;
String pin;
bool useGaz_ = true; // use gas? button on webPage
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
  docPins["usegaz"] = "false";
}

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
Timers<3> timers;

// obiekty ekspanderów PCF8574
PCF8574 ExpInput(0x20);  // utworzenie obiektu dla pierwszego ekspandera
PCF8574 ExpOutput(0x26); // utworzenie obiektu dla drugiego ekspandera

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

  ExpInput.pinMode(P6, INPUT_PULLUP); // input from stove
  ExpInput.pinMode(P7, INPUT_PULLUP); // unused input

  ExpInput.digitalWrite(P0, HIGH);
  ExpInput.digitalWrite(P1, HIGH);
  ExpInput.digitalWrite(P2, HIGH);
  ExpInput.digitalWrite(P3, HIGH);
  ExpInput.digitalWrite(P4, HIGH);
  ExpInput.digitalWrite(P5, HIGH);

  ExpInput.digitalWrite(P6, HIGH);
  ExpInput.digitalWrite(P7, HIGH);
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
void useGaz(void)
{

  ExpOutput.digitalWrite(P6, LOW); // LED ON
  ExpOutput.digitalWrite(P7, LOW); // Pompa ON
  docPins["piec_pompa"] = "ON";
  docPins["led"] = "ON";
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
      Serial.println("usegaz - detected : " + String(docInput["usegaz"].as<const char*>()));
      Serial.println(docInput["usegaz"].as<const char*>());

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
        manager.setFireplaceTemperature(id, targetTemperatureFireplace); // Update local fireplace target only
    }



    // }
    if (docInput["command"]=="forced"){

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
      if (updatedRoom.ID != -1) { // Check if room exists
          updatedRoom.forced = forced; // Update only the forced status
          manager.updateOrAddRoom(updatedRoom); // Use updateOrAddRoom to save the change
          Serial.printf("Forced status for room %d set to %s\n", id, forced ? "true" : "false");
      } else {
          Serial.printf("Error: Room %d not found when trying to set forced status.\n", id);
      }

      // Update docPins for immediate UI feedback (though manifoldLogicNew will also update it)
      // Ensure pinNumber is valid before accessing docPins
      if (pinNumber >= 0 && pinNumber < 6) { // Assuming pins 0-5 are for rooms
          String pinStr = "pin_" + String(pinNumber);
          if (forced) {
              docPins["pins"][pinStr]["state"] = "ON"; // Tentative state, logic will confirm
              docPins["pins"][pinStr]["forced"] = "true";
          } else {
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
    if (docInput["command"] == "getPinMappings") {
        String mappings = manager.getPinMappingAsJson();
        webSocket.sendTXT(num, mappings);
    }
    else if (docInput["command"] == "updatePin") {
        int roomId = docInput["roomId"];
        int newPin = docInput["pin"];
        manager.updatePinMapping(roomId, newPin);
        
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

  // add docPins to docRooms

  String data = manager.getRoomsAsJson();
  webSocket.broadcastTXT(data);
}


void manifoldLogicNew() {
    delay(400); // Keep delay?

    // --- Gas/Pump Control ---
    if (useGaz_) {
        ExpOutput.digitalWrite(P6, LOW); // Gas/Pump ON (assuming LOW is ON for these pins)
        ExpOutput.digitalWrite(P7, LOW);
        docPins["piec_pompa"] = "ON"; // Assuming P7 is pump
        docPins["led"] = "ON";        // Assuming P6 is LED/Gas valve indicator
        Serial.println("Gas mode ON - P6/P7 LOW");
    } else {
        ExpOutput.digitalWrite(P6, HIGH); // Gas/Pump OFF (assuming HIGH is OFF)
        ExpOutput.digitalWrite(P7, HIGH);
        docPins["piec_pompa"] = "OFF";
        docPins["led"] = "OFF";
        Serial.println("Gas mode OFF - P6/P7 HIGH");
    }

    // --- Room Heating Logic ---
    const std::vector<RoomData>& rooms = manager.getAllRooms();
    std::vector<int> forcedRoomsNeedingHeatIDs; // Store IDs of forced rooms needing heat
    int primaryRoomId = -1;
    float lowestTemp = 100.0;
    int secondaryRoomId = -1;
    float smallestPositiveDifference = 100.0;

    // --- First Pass: Identify forced rooms needing heat and find the primary candidate ---
    for (const auto& room : rooms) {
        if (!room.forced) continue; // Skip non-forced rooms

        // Determine effective target temperature
        float effectiveTargetTemperature;
        if (useGaz_) {
            effectiveTargetTemperature = max(room.targetTemperatureNetatmo, room.targetTemperatureFireplace);
        } else {
            effectiveTargetTemperature = room.targetTemperatureFireplace;
        }

        // Check if room needs heating
        if (room.currentTemperature < effectiveTargetTemperature) {
            forcedRoomsNeedingHeatIDs.push_back(room.ID); // Add to list

            // Check if this room has the lowest temperature so far
            if (room.currentTemperature < lowestTemp) {
                lowestTemp = room.currentTemperature;
                primaryRoomId = room.ID;
            }
        }
    }

    // --- Second Pass (if primary found): Find the secondary candidate ---
    if (primaryRoomId != -1) {
        for (int roomId : forcedRoomsNeedingHeatIDs) {
            if (roomId == primaryRoomId) continue; // Skip the primary room

            // Find the room data again (could optimize by storing pointers/references)
            for (const auto& room : rooms) {
                if (room.ID == roomId) {
                     // Determine effective target temperature again (could optimize)
                    float effectiveTargetTemperature;
                    if (useGaz_) {
                        effectiveTargetTemperature = max(room.targetTemperatureNetatmo, room.targetTemperatureFireplace);
                    } else {
                        effectiveTargetTemperature = room.targetTemperatureFireplace;
                    }

                    float difference = effectiveTargetTemperature - room.currentTemperature;
                    if (difference > 0 && difference < smallestPositiveDifference) { // Must need heat and be smallest diff
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
    for (int i = 0; i < 6; i++) { // Assuming pins 0-5 control rooms
        ExpOutput.digitalWrite(i, LOW); // LOW = OFF
        docPins["pins"]["pin_" + String(i)]["state"] = "OFF";
    }

    // Activate primary room relay
    if (primaryRoomId != -1) {
        for (const auto& room : rooms) {
            if (room.ID == primaryRoomId) {
                if (room.pinNumber >= 0 && room.pinNumber < 6) {
                    ExpOutput.digitalWrite(room.pinNumber, HIGH); // HIGH = ON
                    docPins["pins"]["pin_" + String(room.pinNumber)]["state"] = "ON";
                    Serial.printf("Primary heating ON: Room %s (Pin %d, Temp %.1f, Lowest Temp)\n",
                                  room.name.c_str(), room.pinNumber, room.currentTemperature);
                }
                break;
            }
        }
    } else {
         Serial.println("No primary forced room needs heating.");
    }

    // Activate secondary room relay
    if (secondaryRoomId != -1) {
         for (const auto& room : rooms) {
            if (room.ID == secondaryRoomId) {
                if (room.pinNumber >= 0 && room.pinNumber < 6) {
                    // Check if it's the same pin as primary - avoid double logging if so
                    if (primaryRoomId == -1 || room.pinNumber != manager.getRoomByID(primaryRoomId).pinNumber) {
                         ExpOutput.digitalWrite(room.pinNumber, HIGH); // HIGH = ON
                         docPins["pins"]["pin_" + String(room.pinNumber)]["state"] = "ON";
                         Serial.printf("Secondary heating ON: Room %s (Pin %d, Temp %.1f, Smallest Diff %.1f)\n",
                                      room.name.c_str(), room.pinNumber, room.currentTemperature, smallestPositiveDifference);
                    } else {
                         Serial.printf("Secondary room (%s) shares pin with primary. Already ON.\n", room.name.c_str());
                    }
                }
                break;
            }
        }
    } else if (primaryRoomId != -1) {
        Serial.println("No suitable secondary forced room found.");
    }


    // --- Update docPins forced status and final room info ---
    for (const auto& room : rooms) {
        if (room.pinNumber >= 0 && room.pinNumber < 6) {
            docPins["pins"]["pin_" + String(room.pinNumber)]["forced"] = room.forced ? "true" : "false";
        }
    }
    docPins["roomsInfo"] = manager.getRoomsAsJson(); // Send updated state including valve status
    Serial.println("--- End Heating Logic ---");
}



void setup()
{

  Serial.begin(115200);
  Serial.println("Starting up...");

  // -- Initializing the configuration.
  iotWebConf.init();
  iotWebConf.setApTimeoutMs(0);
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

  initInputExpander();

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

  // Inicjalizacja timera
  timers.attach(0, 35000, fetchNetatmo);
  timers.attach(1, 12000, broadcastWebsocket);
  timers.attach(2, 20000, manifoldLogicNew);
  
}

// void manifoldLogic()
// {

//   bool anyForcedON = 0;
//   bool anyInputON = 0;
//   docPins["manifoldTemp"] = manifoldTemp;
//   unsigned long currentMillis = millis(); // Get the current time

//   delay(400);

//   // Update JSON doc from six states on first expander

//   for (int i = 0; i < 6; i++)
//   {
//     String curr_pin = String(ExpInput.digitalRead(i));

//     if (docPins["pins"]["pin_" + String(i)]["forced"] == "true")
//     {
//       anyForcedON = 1;
//     }
//   }

//   // check woodStove init input

//   // Serial.println(String(ExpInput.digitalRead(P6)) + " ExpInput.digital P6");
//   // if (String(ExpInput.digitalRead(P6)) == "true")
//   // {

//   //   // Serial.println("WoodStove operating ");
//   //   docPins["WoodStove"] = "on";
//   // }
//   // if (String(ExpInput.digitalRead(P6)) == "false")
//   // {

//   //   // Serial.println("WoodStove off ");
//   //   docPins["WoodStove"] = "off";
//   // }

//   if (anyForcedON == 1)
//   {
//     for (int i = 0; i < 6; i++)
//     {

//       if (docPins["pins"]["pin_" + String(i)]["forced"] == "true")
//       {
//         ExpOutput.digitalWrite(i, HIGH);
//       }
//       else
//       {
//         ExpOutput.digitalWrite(i, LOW);
//       }
//     }
//   }

//   // if (anyForcedON && useGaz_)
//   // {
//   //   // useGaz();
//   // }

//   if (anyForcedON == 0)
//   {
//     // Serial.println("Pump OFF");
//     //   ExpOutput.digitalWrite(P6, HIGH); // LED OFF
//     ExpOutput.digitalWrite(P7, HIGH); // Pompa OFF
//     //   doc["piec_pompa"] = "OFF";
//     //   doc["led"] = "OFF";

//     // Serial.println("All valves OFF");
//     for (int i = 0; i < 6; i++)
//     {

//       ExpOutput.digitalWrite(i, HIGH); // Pinoutput OFF
//     }
//   }
// }




void loop()
{
  // Sprawdzenie, czy upłynęło 20 minut od ostatniego restartu
  // if (currentMillis - previousMillis >= interval) {
  //   ESP.restart();  // Restart the NodeMCU
  //   previousMillis = currentMillis;
  // }

  iotWebConf.doLoop();

  // jeśli nawiązano połączenie z siecią
  if (iotWebConf.getState() == 4)
  {

    webSocket.loop();
    ArduinoOTA.handle();



    timers.process();
  }
}
