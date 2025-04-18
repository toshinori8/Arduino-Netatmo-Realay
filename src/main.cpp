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
#error platformio.ini must contain "build_flags = -DIOTWEBCONF_ENABLE_JSON"
#endif

StaticJsonDocument<8192> docPins;
StaticJsonDocument<1124> docRooms;
StaticJsonDocument<200> docInput;

#include <functional>
#include <webPage.h>
#include <roomManager.h>

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

// AHT10 INIT;
// #include <Adafruit_AHTX0.h>
// Adafruit_AHTX0 aht;
// sensors_event_t humidity, temp;

#include <aht10sensor.h>
AHT10 sensor;

float manifoldTemp, manifoldHum;
float manifoldMinTemp = 18.0;
float manifoldMaxTemp = 60.0;

void readAHT()
{

 float temp_reading, hum_reading;
    // Serial.printf("Attempting AHT read. Last known Temp: %.1f, Errors: %d\n", manifoldTemp, aht_read_errors); // Log

    if (sensor.measure(&temp_reading, &hum_reading)) {
        manifoldTemp = temp_reading;
        manifoldHum = hum_reading;
        // Serial.printf("AHT Read OK: Temp=%.1f, Hum=%.1f\n", manifoldTemp, manifoldHum);
        aht_read_errors = 0; // Resetuj licznik błędów po sukcesie
    } else {
        Serial.println("AHT Measurement failed.");
        aht_read_errors++;
        if (aht_read_errors >= MAX_AHT_ERRORS_BEFORE_RESET) {
            Serial.printf("Reached %d AHT read errors. Attempting sensor reset and re-init...\n", aht_read_errors);
            if (sensor.softReset()) {
                delay(100); // Daj czas na ustabilizowanie
                if (sensor.begin()) {
                     Serial.println("AHT Sensor re-initialized successfully after reset.");
                     aht_read_errors = 0; // Resetuj licznik po udanej reinicjalizacji
                } else {
                     Serial.println("AHT Sensor re-initialization FAILED after reset.");
                     // Błąd nadal występuje, licznik nie jest resetowany
                }
            } else {
                 Serial.println("AHT Sensor soft reset command FAILED.");
                 // Błąd nadal występuje
            }
        }
   }
}

WebSocketsServer webSocket = WebSocketsServer(81);
DNSServer dnsServer;
WebServer server(80);
IotWebConf iotWebConf(thingName, &dnsServer, &server, wifiInitialApPassword);

RoomManager manager;
#include <romManager.h>
void prepareDataForWebServer()
{
  docPins["pins"]["ROOM_RELAY_1"]["state"] = "OFF";
  docPins["pins"]["ROOM_RELAY_1"]["forced"] = "false";

  docPins["pins"]["ROOM_RELAY_2"]["state"] = "OFF";
  docPins["pins"]["ROOM_RELAY_2"]["forced"] = "false";

  docPins["pins"]["ROOM_RELAY_3"]["state"] = "OFF";
  docPins["pins"]["ROOM_RELAY_3"]["forced"] = "false";

  docPins["pins"]["ROOM_RELAY_4"]["state"] = "OFF";
  docPins["pins"]["ROOM_RELAY_4"]["forced"] = "false";

  docPins["WoodStove"] = "off";
  docPins["manifoldTemp"] = "";
  docPins["netatmoData"] = "";
  docPins["usegaz"] = useGaz_ ? "true" : "false"; // Initialize based on loaded/default useGaz_
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
Timers<4> timers;

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
    if (docInput["command"] == "manifoldMinTemp")
    {

      // value to float

      manifoldMinTemp = docInput["value"].as<float>();
    }
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
      saveSettings(manager, useGaz_, manifoldTemp); // Save after change
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
      saveSettings(manager, useGaz_, manifoldTemp);                                            // Save after change
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
        saveSettings(manager, useGaz_, manifoldTemp); // Save after change
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
      saveSettings(manager, useGaz_, manifoldTemp); // Save after change

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
  docPins["manifoldMinTemp"] = String(manifoldMinTemp);
  docPins["manifoldTemp"] = String(manifoldTemp);

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

#include <manifoldLogicOld.h>

void setup()
{

  pinMode(CONFIG_PIN, INPUT_PULLUP);
  Serial.begin(115200);
  while (!Serial)
  {
    delay(10);
  } // Wait for serial connection
  Serial.println("\nStarting up...");

  if (!sensor.begin())
  {
    Serial.println("Failed to initialize AHT10");
    while (1);
  }

  // -- Initialize EEPROM --
  EEPROM.begin(EEPROM_SIZE); // Use the requested size (512)
  Serial.printf("EEPROM size requested: %d bytes\n", EEPROM_SIZE);

  // -- Load settings or initialize EEPROM --
  if (!loadSettings(manager,useGaz_, manifoldTemp))
  {
    // EEPROM not initialized or data corrupted, save defaults
    Serial.println("Initializing EEPROM with default settings...");
    useGaz_ = false; // Default value
    // Manager defaults (like initial pin map) are set in its constructor.
    // We save the current state which includes these defaults.
    saveSettings(manager, useGaz_, manifoldTemp);
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
  timers.attach(3, 150000, readAHT);
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
