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
      float targetTemperature = docInput["targetTemperature"];
      bool forced = docInput["forced"];
      manager.setTemperature(id, targetTemperature);
     

      if (docInput["usegaz"] == "false")
      {
        manager.setTemperature(id, targetTemperature);
      }
    }



    // }
    if (docInput["command"]=="forced"){

      int id = docInput["id"];
      
      // Znajdz Room ktorego ID == id 
      RoomData room = manager.getRoomByID(id);
      // get pinNumber from room 
      int pinNumber = room.pinNumber;
      String pin = "pin_" + String(pinNumber);

      float targetTemperature = docInput["targetTemperature"];
      bool forced = docInput["forced"];
     
      // if (forced ==0){forced="false";}else(forced="true");
      room.forced = forced;
      Serial.println("Forced in room.forced");
      Serial.println(room.forced);



      // room.forced =forced;

      Serial.println(forced);
      // Pobierz aktualne dane pokoju, aby nie stracić innych wartości
      RoomData updatedRoom = manager.getRoomByID(id);
      // Aktualizuj tylko forced i targetTemperature
      updatedRoom.forced = forced;
      if (targetTemperature > 0) {
        updatedRoom.targetTemperature = targetTemperature;
      }
      manager.updateOrAddRoom(updatedRoom);


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


void manifoldLogicNew()
{
    delay(400);

    // Pobierz wszystkie pokoje z RoomManager
    const std::vector<RoomData>& rooms = manager.getAllRooms();

    // Flagi do śledzenia stanu
    bool anyRoomNeedsHeating = false;
    bool anyRoomForced = false;

    // Zmienne do śledzenia pokoju z najniższą temperaturą wśród wymuszonych
    int lowestTempForcedRoomId = -1;
    float lowestTemp = 100.0; // Inicjalizacja wysoką wartością

    // Zmienne do śledzenia pokoju z najwyższym priorytetem (dla przypadku gdy nie ma wymuszonych)
    float maxPriority = -999.0;
    int maxPriorityRoomId = -1;

    // Przejdź przez wszystkie pokoje
    for (const auto& room : rooms) {
        // Oblicz priorytet (różnica między temperaturą docelową a aktualną)
        float priority = room.targetTemperature - room.currentTemperature;

        // Sprawdź, czy pokój wymaga ogrzewania (temperatura docelowa > aktualna)
        if (priority > 0) {
            anyRoomNeedsHeating = true;
        }

        // Sprawdź, czy pokój jest w trybie wymuszonym
        if (room.forced) {
            anyRoomForced = true;

            // Sprawdź, czy ten pokój ma najniższą temperaturę wśród wymuszonych
            if (room.currentTemperature < lowestTemp) {
                lowestTemp = room.currentTemperature;
                lowestTempForcedRoomId = room.ID;
            }
        }

        // Aktualizuj pokój z najwyższym priorytetem (dla przypadku gdy nie ma wymuszonych)
        if (priority > maxPriority) {
            maxPriority = priority;
            maxPriorityRoomId = room.ID;
        }
    }

    // Najpierw ustaw wszystkie piny na LOW (wyłączone)
   if(anyRoomForced){
    for (int i = 0; i < 6; i++) {
        ExpOutput.digitalWrite(i, LOW); // LOW = OFF (odwrócona logika)
        docPins["pins"]["pin_" + String(i)]["state"] = "OFF";
    }
}
    // Sprawdź, czy są pokoje w trybie wymuszonym
    if (anyRoomForced && lowestTempForcedRoomId != -1) {
        Serial.println("Tryb wymuszony aktywny - wybieranie pokoju z najniższą temperaturą");

        // Znajdź pokój z najniższą temperaturą wśród wymuszonych
        for (const auto& room : rooms) {
            if (room.ID == lowestTempForcedRoomId) {
                // Sprawdź, czy pokój potrzebuje ogrzewania (temperatura docelowa > aktualna)
                float priority = room.targetTemperature - room.currentTemperature;

                if (priority > 0) {  // Tylko jeśli pokój potrzebuje ogrzewania
                    // Włącz odpowiedni pin dla pokoju z najniższą temperaturą wśród wymuszonych
                    if (room.pinNumber >= 0 && room.pinNumber < 6) {
                        // informacje w serial o ogrzewaniu
                        Serial.print("Ogrzewanie w pokoju wymuszonym: ");
                        Serial.println(room.name.c_str());
                        Serial.print("Stan pinu: ");
                        Serial.println(room.pinNumber);
                        Serial.print("Temperatura aktualna: ");
                        Serial.println(room.currentTemperature);
                        Serial.print("Temperatura docelowa: ");
                        Serial.println(room.targetTemperature);
                        Serial.print("Priorytet (różnica): ");
                        Serial.println(priority);

                        ExpOutput.digitalWrite(room.pinNumber, HIGH); // HIGH = ON (odwrócona logika)
                        docPins["pins"]["pin_" + String(room.pinNumber)]["state"] = "ON";
                    }
                } else {
                    Serial.print("Pokój wymuszony nie potrzebuje ogrzewania: ");
                    Serial.println(room.name.c_str());
                    Serial.print("Temperatura aktualna: ");
                    Serial.println(room.currentTemperature);
                    Serial.print("Temperatura docelowa: ");
                    Serial.println(room.targetTemperature);
                }

                // Zawsze ustawiamy forced na true, niezależnie od tego czy grzejemy
                if (room.pinNumber >= 0 && room.pinNumber < 6) {
                    docPins["pins"]["pin_" + String(room.pinNumber)]["forced"] = "true";
                }

                break; // Znaleźliśmy pokój, więc możemy przerwać pętlę
            }
        }
    
        // Jeśli nie ma pokoi wymuszonych, ale są pokoje potrzebujące ogrzewania,
        // wybierz pokój z najwyższym priorytetem
        Serial.println("Brak pokoi wymuszonych - wybieranie pokoju z najwyższym priorytetem");

        for (const auto& room : rooms) {
            if (room.ID == maxPriorityRoomId) {
                if (room.pinNumber >= 0 && room.pinNumber < 6) {
                    Serial.print("Ogrzewanie w pokoju z najwyższym priorytetem: ");
                    Serial.println(room.name.c_str());
                    Serial.print("Stan pinu: ");
                    Serial.println(room.pinNumber);
                    Serial.print("Temperatura aktualna: ");
                    Serial.println(room.currentTemperature);
                    Serial.print("Temperatura docelowa: ");
                    Serial.println(room.targetTemperature);
                    Serial.print("Priorytet (różnica): ");
                    Serial.println(room.targetTemperature - room.currentTemperature);

                    ExpOutput.digitalWrite(room.pinNumber, HIGH); // HIGH = ON (odwrócona logika)
                    docPins["pins"]["pin_" + String(room.pinNumber)]["state"] = "ON";
                }
                break;
            }
        }
    } else {
        Serial.println("Brak pokoi wymagających ogrzewania lub brak pokoi wymuszonych z potrzebą ogrzewania");
    }

    // Aktualizuj stan forced dla wszystkich pokoi w docPins
    for (const auto& room : rooms) {
        if (room.pinNumber >= 0 && room.pinNumber < 6) {
            docPins["pins"]["pin_" + String(room.pinNumber)]["forced"] = room.forced ? "true" : "false";
        }
    }

    // Aktualizuj docPins z informacjami o pokojach
    docPins["roomsInfo"] = manager.getRoomsAsJson();
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
