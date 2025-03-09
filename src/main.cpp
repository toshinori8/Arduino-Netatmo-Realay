#include <Arduino.h>

#include <ArduinoOTA.h>
#include "PCF8574.h"
#include <Wire.h>
#include "Timers.h"
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <webPage.h>
#include <WebSocketsServer.h>
#include <functional>
#include <EEPROM.h>
#include <Esp.h>

const int NUM_RELAYS = 6;  // liczba przekaźników
const int relayPins[NUM_RELAYS] = {0, 1, 2, 3, 4, 5};  // piny przekaźników na ekspanderze

unsigned long lastSendTime = 0;
unsigned long previousMillis = 0;      // Variable to store the previous time
const long interval = 480 * 60 * 1000; // Interval at which to reset the NodeMCU
WebSocketsServer webSocket(8080);

bool useGaz_ = true; // use gas? button on webPage

StaticJsonDocument<200> doc;

#include <IotWebConf.h>
const char thingName[] = "Netatmo_Relay";
const char wifiInitialApPassword[] = "12345678";
void handleRoot();
DNSServer dnsServer;
WebServer server(80);
IotWebConf iotWebConf(thingName, &dnsServer, &server, wifiInitialApPassword);

String forced;
String woodStove;
String state;
String pin;

// Struktura do przechowywania stanu przekaźnika
struct RelayState {
  bool isOn;
  bool isAuto;
  String linkedSensor;
};

RelayState relayStates[NUM_RELAYS];
unsigned long lastSaveTime = 0;
const unsigned long SAVE_INTERVAL = 600000; // 10 minut w milisekundach

// Dodaj na początku pliku po innych includach
#include <ESP8266WiFi.h>
rst_info *myResetInfo;

// Dodaj na początku pliku po innych stałych
const int CONFIG_PIN = 0;  // GPIO0 - przycisk FLASH na ESP8266
unsigned long buttonDownTime = 0;
const int CONFIG_RESET_TIMEOUT = 3000; // 3 sekundy przytrzymania przycisku

const int WATCHDOG_TIMEOUT = 8; // sekundy na watchdog timer

void prepareDataForWebServer()
{

  doc["pin_0"]["state"] = "OFF";
  doc["pin_0"]["forced"] = "false";

  doc["pin_1"]["state"] = "OFF";
  doc["pin_1"]["forced"] = "false";

  doc["pin_2"]["state"] = "OFF";
  doc["pin_2"]["forced"] = "false";

  doc["pin_3"]["state"] = "OFF";
  doc["pin_3"]["forced"] = "false";
  doc["WoodStove"] = "off";
}

void handleRoot()
{
  if (iotWebConf.handleCaptivePortal())
  {
    return;
  }
  server.send(200, "text/html", webpage);
}

// utworzenie obiektu klasy Timers z trzema odliczającymi
// Timers<3> timers;

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
  doc["piec_pompa"] = "ON";
  doc["led"] = "ON";
}
// Obsługa eventow Websocket
void onWsEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{
  switch (type)
  {
  case WStype_DISCONNECTED:
    // handle client disconnection
    Serial.println("Client disconnected from WebSocket ");
    break;
  case WStype_CONNECTED:
  {
    // handle client connection
    String message = "{\"response\":\"connected\"}";
    webSocket.sendTXT(num, message);
  }
  break;
  case WStype_TEXT:
  {
    StaticJsonDocument<200> docInput;
    // handle incoming text message from client
    String messageText = String((char *)payload).substring(0, length);
    // parse the JSON message
    DeserializationError error = deserializeJson(docInput, messageText);
    if (error)
    {
      Serial.println("Error parsing JSON");
      return;
    }

    if (docInput["usegaz"])
    {
      String gas = String(docInput["usegaz"]);
      Serial.println("usegaz - detected");
      if (gas == "true")
      {
        useGaz_ = true;
      }
      else
      {
        useGaz_ = false;
      }
    }

    // extract the pin number and state from the JSON message
    pin = String(docInput["pin"]);
    forced = String(docInput["forced"]);
    state = String(docInput["state"]);

    if (forced == "true")
    {
      doc[pin]["state"] = "ON";
      doc[pin]["forced"] = "true";
    }
    else
    {

      doc[pin]["state"] = "OFF";
      doc[pin]["forced"] = "false";
    }

    // char data[200];
    // size_t len = serializeJson(doc, data);
    // webSocket.sendTXT(len, data);
    // ExpOutput.digitalWrite(P0, HIGH);
    // set the state of the input pin on the PCF8574 input expander
  }
  break;
  }
}

void saveState() {
  EEPROM.begin(512);
  int addr = 0;
  
  for(int i = 0; i < NUM_RELAYS; i++) {
    EEPROM.write(addr++, relayStates[i].isOn ? 1 : 0);
    EEPROM.write(addr++, relayStates[i].isAuto ? 1 : 0);
    // Zapisz długość nazwy czujnika
    byte sensorNameLength = relayStates[i].linkedSensor.length();
    EEPROM.write(addr++, sensorNameLength);
    // Zapisz nazwę czujnika
    for(int j = 0; j < sensorNameLength; j++) {
      EEPROM.write(addr++, relayStates[i].linkedSensor[j]);
    }
  }
  
  EEPROM.commit();
}

void loadState() {
  EEPROM.begin(512);
  int addr = 0;
  
  for(int i = 0; i < NUM_RELAYS; i++) {
    relayStates[i].isOn = EEPROM.read(addr++) == 1;
    relayStates[i].isAuto = EEPROM.read(addr++) == 1;
    // Odczytaj długość nazwy czujnika
    byte sensorNameLength = EEPROM.read(addr++);
    char sensorName[33] = {0}; // Max 32 znaki + null terminator
    // Odczytaj nazwę czujnika
    for(int j = 0; j < sensorNameLength; j++) {
      sensorName[j] = EEPROM.read(addr++);
    }
    relayStates[i].linkedSensor = String(sensorName);
    
    // Zastosuj odczytany stan
    digitalWrite(relayPins[i], relayStates[i].isOn ? HIGH : LOW);
  }
}

// Dodaj po deklaracji iotWebConf
void wifiConnected()
{
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  // Wyślij informację o IP przez WebSocket
  StaticJsonDocument<200> ipDoc;
  ipDoc["type"] = "wifi_info";
  ipDoc["ip"] = WiFi.localIP().toString();
  ipDoc["status"] = "connected";
  
  String ipJSON;
  serializeJson(ipDoc, ipJSON);
  webSocket.broadcastTXT(ipJSON);
}

void configSaved()
{
  Serial.println("Configuration was updated.");
}

void setup()
{
  Serial.begin(9600);
  
  // Dodaj na początku setup
  ESP.wdtDisable(); // Wyłącz watchdog na czas setupu
  ESP.wdtEnable(WATCHDOG_TIMEOUT * 1000); // Włącz watchdog z timeoutem w ms
  
  pinMode(CONFIG_PIN, INPUT_PULLUP);
  
  // Debugowanie przyczyny restartu
  myResetInfo = ESP.getResetInfoPtr();
  String resetReason = "Reset reason: " + String(myResetInfo->reason);
  
  // Przygotuj JSON z informacją o restarcie
  StaticJsonDocument<200> resetDoc;
  resetDoc["type"] = "reset_info";
  resetDoc["reason"] = String(myResetInfo->reason);
  resetDoc["heap"] = ESP.getFreeHeap();
  
  // Dodaj callback-i dla IotWebConf
  iotWebConf.setWifiConnectionCallback(&wifiConnected);
  iotWebConf.setConfigSavedCallback(&configSaved);
  
  // Zwiększ timeout na połączenie WiFi
  iotWebConf.setApTimeoutMs(120000); // 2 minuty na konfigurację
  
  // Inicjalizacja z większym debugowaniem
  Serial.println("Starting up...");
  Serial.println("Initializing IotWebConf...");
  iotWebConf.init();
  
  // Sprawdź czy mamy zapisaną konfigurację
  if (iotWebConf.getState() == IOTWEBCONF_STATE_BOOT)
  {
    Serial.println("No saved config, starting AP mode");
  }
  else if (iotWebConf.getState() == IOTWEBCONF_STATE_NOT_CONFIGURED)
  {
    Serial.println("Config invalid, starting AP mode");
  }
  else
  {
    Serial.println("Trying to connect to configured network...");
  }

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

  // blinkOutput(20);

  otaStart();
  initInputExpander();
  // timers.attach(0, 1000, functionName);

  loadState(); // Dodaj to na końcu setup()

  // Po inicjalizacji WebSocket wyślij informację
  String resetJSON;
  serializeJson(resetDoc, resetJSON);
  webSocket.broadcastTXT(resetJSON);
}

void loop()
{
  // Dodaj na początku loop
  ESP.wdtFeed(); // Reset watchdog timer
  
  // Sprawdź czy przycisk jest wciśnięty (stan niski bo INPUT_PULLUP)
  if (digitalRead(CONFIG_PIN) == LOW)
  {
    if (buttonDownTime == 0) // Przycisk został właśnie wciśnięty
    {
      buttonDownTime = millis();
    }
    else if ((millis() - buttonDownTime) > CONFIG_RESET_TIMEOUT)
    {
      Serial.println("Resetting configuration...");
      iotWebConf.getSystemParameterGroup()->applyDefaultValue(); // Reset do wartości domyślnych
      iotWebConf.saveConfig(); // Zapisz pustą konfigurację
      ESP.restart(); // Zrestartuj urządzenie
    }
  }
  else
  {
    buttonDownTime = 0; // Reset licznika gdy przycisk puszczony
  }

  if (iotWebConf.getState() == IOTWEBCONF_STATE_ONLINE)
  {
    if(ESP.getFreeHeap() < 4096) { // Ostrzeżenie przy małej ilości pamięci
      Serial.println("Low memory: " + String(ESP.getFreeHeap()));
    }
    
    // Dodaj yield() w długich operacjach
    yield();  // Pozwól watchdogowi się zresetować

    bool anyForcedON = 0;
    bool anyInputON = 0;
    unsigned long currentMillis = millis(); // Get the current time

    delay(400);

    // Upfdate JSON doc from six states on first expander

    for (int i = 0; i < 6; i++)
    {
      String curr_pin = String(ExpInput.digitalRead(i));

      if (doc["pin_" + String(i)]["forced"] == "true")
      {
        anyForcedON = 1;
      }
    }

    // check woodStove init input

    Serial.println(String(ExpInput.digitalRead(P6)) + " ExpInput.digital P6");
    if (String(ExpInput.digitalRead(P6)) == "true")
    {

      Serial.println("WoodStove operating ");
      doc["WoodStove"] = "on";
    }  
    if (String(ExpInput.digitalRead(P6)) == "false")
    {

      Serial.println("WoodStove off ");
      doc["WoodStove"] = "off";
    }
   

    if (anyForcedON == 1)
    {
      for (int i = 0; i < 6; i++)
      {

        if (doc["pin_" + String(i)]["forced"] == "true")
        {
          ExpOutput.digitalWrite(i, HIGH);
        }
        else
        {
          ExpOutput.digitalWrite(i, LOW);
        }
      }
    }

    // if (anyForcedON && useGaz_)
    // {
    //   // useGaz();
    // }

    if (anyForcedON == 0)
    {
      Serial.println("Pump OFF");
      //   ExpOutput.digitalWrite(P6, HIGH); // LED OFF
      ExpOutput.digitalWrite(P7, HIGH); // Pompa OFF
      //   doc["piec_pompa"] = "OFF";
      //   doc["led"] = "OFF";

      Serial.println("All valves OFF");
      for (int i = 0; i < 6; i++)
      {

        ExpOutput.digitalWrite(i, HIGH); // Pinoutput OFF
      }
    }

    // WYSYŁANIE JSON PRZEZ WSSOCKET
    // czy upłynęło co najmniej 20 sekund od ostatniego wysłania

    if (millis() - lastSendTime > 9000)
    {
      String outputJSON;
      serializeJson(doc, outputJSON);
      serializeJsonPretty(doc, Serial);

      // wysyłanie info do klientów
      webSocket.broadcastTXT(outputJSON);
      lastSendTime = millis();

      Serial.println(String(anyInputON) + ": anyInputON");
      Serial.println(String(anyForcedON) + ": anyForcedON");
    }

    // Dodaj to w głównej pętli
    if (millis() - lastSaveTime >= SAVE_INTERVAL) {
      saveState();
      lastSaveTime = millis();
    }

    delay(10); // Małe opóźnienie zamiast 400ms
    yield();
  }
  else
  {
    // Jeśli nie jesteśmy połączeni, skupiamy się tylko na obsłudze konfiguracji
    iotWebConf.doLoop();
    delay(1000);
    return;
  }
  
  webSocket.loop();
  ArduinoOTA.handle();
}

// Zmodyfikuj funkcję obsługującą zmianę stanu przekaźnika
void handleRelay(int relayIndex, bool state) {
  digitalWrite(relayPins[relayIndex], state ? HIGH : LOW);
  relayStates[relayIndex].isOn = state;
  // Opcjonalnie możesz tu dodać natychmiastowy zapis stanu
  // saveState();
}
