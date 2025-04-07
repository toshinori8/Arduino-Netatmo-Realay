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
#include "manifoldLogic.h"



#include <ArduinoJson.h>

#ifndef IOTWEBCONF_ENABLE_JSON
# error platformio.ini must contain "build_flags = -DIOTWEBCONF_ENABLE_JSON"
#endif

void readConfigFile();



// #include <Wire.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_SSD1306.h>

// Adafruit_SSD1306 display(128, 64, &Wire, -1); // Użyj -1, jeśli nie używasz resetu

const int NUM_RELAYS = 8;                             // liczba przekaźników
const int relayPins[NUM_RELAYS] = {0, 1, 2, 3, 4, 5,6 ,7}; // piny przekaźników na ekspanderze
const char* relayNames[NUM_RELAYS] = {
  "Przedpokój",
  "Sypialnia",
  "Kuchnia",
  "Łazienka",
  "_Klatka schodowa",
  "_Gościnny",
  "_Salon", 
  "_Nawetnia"

};

unsigned long lastSendTime = 0;
unsigned long previousMillis = 0;      // Variable to store the previous time
const long interval = 480 * 60 * 1000; // Interval at which to reset the NodeMCU
WebSocketsServer webSocket(8080);

bool useGaz_ = true; // use gas? button on webPage

StaticJsonDocument<200> docPins;

#include <IotWebConf.h>
const char thingName[] = "Netatmo_Relay";
const char wifiInitialApPassword[] = "pmgana921";

// Dodaj domyślną konfigurację WiFi
const char iwcWifiSsid[] = "oooooio";
const char iwcWifiPassword[] = "pmgana921";





void handleRoot();
DNSServer dnsServer;
WebServer server(80);
IotWebConf iotWebConf(thingName, &dnsServer, &server, wifiInitialApPassword);


String forced;
String woodStove;
String state;
String pin;

// Deklaracje funkcji
void saveState();
void onWsEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length);

RelayState relayStates[NUM_RELAYS];
unsigned long lastSaveTime = 0;
const unsigned long SAVE_INTERVAL = 600000; // 10 minut w milisekundach

// Dodaj na początku pliku po innych includach
#include <ESP8266WiFi.h>
rst_info *myResetInfo;

// Dodaj na początku pliku po innych stałych
const int CONFIG_PIN = 0; // GPIO0 - przycisk FLASH na ESP8266
unsigned long buttonDownTime = 0;
const int CONFIG_RESET_TIMEOUT = 10000; // 10 sekund przytrzymania przycisku

const int WATCHDOG_TIMEOUT = 8; // sekundy na watchdog timer

// Dodaj na początku pliku po innych stałych
String debugLogs = ""; // Bufor na logi
const int MAX_LOG_LENGTH = 2000; // Maksymalna długość bufora logów

// Funkcja do dodawania logów
void addDebugLog(const String& message) {
  debugLogs += message + "\n";
  // Ogranicz długość bufora
  if (debugLogs.length() > MAX_LOG_LENGTH) {
    debugLogs = debugLogs.substring(debugLogs.length() - MAX_LOG_LENGTH);
  }
}

// Funkcja do wysyłania logów przez WebSocket
void sendDebugLogs() {
  if (debugLogs.length() > 0) {
    StaticJsonDocument<2048> doc;
    doc["type"] = "debug_logs";
    doc["logs"] = debugLogs;
    String outputJSON;
    serializeJson(doc, outputJSON);
    webSocket.broadcastTXT(outputJSON);
    debugLogs = ""; // Wyczyść bufor po wysłaniu
  }
}

void prepareDataForWebServer()
{

  docPins["pin_0"]["state"] = "OFF";
  docPins["pin_0"]["forced"] = "false";

  docPins["pin_1"]["state"] = "OFF";
  docPins["pin_1"]["forced"] = "false";

  docPins["pin_2"]["state"] = "OFF";
  docPins["pin_2"]["forced"] = "false";

  docPins["pin_3"]["state"] = "OFF";
  docPins["pin_3"]["forced"] = "false";
  docPins["WoodStove"] = "off";
   docPins["manifoldTemp"] = "";
}

void handleRoot()
{
  if (iotWebConf.handleCaptivePortal())
  {
    return;
  }
  else
  {
    server.sendHeader("Cache-Control", "no-cache");
    server.send(200, "text/html", webpage);
  }
}

// Dodajemy obsługę OPTIONS dla CORS
void handleOptions() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
  server.send(200);
}

// utworzenie obiektu klasy Timers z trzema odliczającymi
// Timers<3> timers;

// obiekty ekspanderów PCF8574
PCF8574 ExpInput(0x20);  // utworzenie obiektu dla pierwszego ekspandera
PCF8574 ExpOutput(0x26); // utworzenie obiektu dla drugiego ekspandera

float manifoldTemp;

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

// void useGaz(void)
// {

//   ExpOutput.digitalWrite(P6, LOW); // LED ON
//   ExpOutput.digitalWrite(P7, LOW); // Pompa ON
//   docPins["gaz"] = "ON";
//   docPins["piec_pompa"] = "ON";
// }
// Funkcja do aktualizacji stanu przekaźnika
 StaticJsonDocument<500> stateDoc;
void updateRelayState(int relayIndex, bool newState) {
  if (relayIndex >= 0 && relayIndex < NUM_RELAYS) {
    relayStates[relayIndex].isOn = newState;
    relayStates[relayIndex].isAuto = false; // Przy ręcznym przełączeniu wyłączamy tryb auto
    
    // Aktualizuj stan fizyczny przekaźnika
    ExpOutput.digitalWrite(relayPins[relayIndex], newState ? LOW : HIGH);
    
    // Zapisz stan do EEPROM
    // saveState();
    
    // Wyślij aktualizację przez WebSocket
   
    stateDoc["type"] = "state";
    stateDoc["timestamp"] = millis();
    stateDoc["manifoldTemp"] = manifoldTemp;
    stateDoc["woodStove"] = (String(ExpInput.digitalRead(P6)) == "true");
    
    // Dodaj stany przekaźników
    JsonArray relays = stateDoc.createNestedArray("relays");
    for (int i = 0; i < NUM_RELAYS; i++) {
      JsonObject relay = relays.createNestedObject();
      relay["id"] = i;
      relay["name"] = relayStates[i].name;
      relay["isOn"] = relayStates[i].isOn;
      relay["isAuto"] = relayStates[i].isAuto;
      relay["linkedSensor"] = relayStates[i].linkedSensor;
    }

    String outputJSON;
    serializeJson(stateDoc, outputJSON);
    webSocket.broadcastTXT(outputJSON);
    
    addDebugLog("Zaktualizowano stan przekaźnika " + relayStates[relayIndex].name + 
                " na " + (newState ? "ON" : "OFF"));
  }
}

void checkThermostatInputs() {
  for (int i = 0; i < 6; i++) {
    bool inputState = ExpInput.digitalRead(i);
    if (inputState == LOW) { // Pin zwarty (termostat wymusza)
      relayStates[i].isForcedByThermostat = true;
      relayStates[i].isOn = true;
      ExpOutput.digitalWrite(relayPins[i], LOW);
    } else {
      relayStates[i].isForcedByThermostat = false;
    }
  }
}

// Funkcja do wysyłania pełnego stanu przekaźników
void sendRelayStates() {
  StaticJsonDocument<2048> stateDoc;
  stateDoc["type"] = "state";
  stateDoc["timestamp"] = millis();
  stateDoc["manifoldTemp"] = manifoldTemp;
  stateDoc["woodStove"] = (String(ExpInput.digitalRead(P6)) == "true");
  
  // Dodaj stany przekaźników
  JsonArray relays = stateDoc.createNestedArray("relays");
  for (int i = 0; i < NUM_RELAYS; i++) {
    JsonObject relay = relays.createNestedObject();
    relay["id"] = i;
    relay["name"] = relayStates[i].name;
    relay["isOn"] = relayStates[i].isOn;
    relay["isAuto"] = relayStates[i].isAuto;
    relay["linkedSensor"] = relayStates[i].linkedSensor;
    relay["isForcedByThermostat"] = relayStates[i].isForcedByThermostat;
  }

  String outputJSON;
  serializeJson(stateDoc, outputJSON);
  webSocket.broadcastTXT(outputJSON);
  
  addDebugLog("Wysłano pełny stan: " + outputJSON);
}


void printEPROM (){
   // Dla wygody w serialu:
    Serial.println("EEPROM: Relays:");
    for (int i = 0; i < NUM_RELAYS; i++) {
      Serial.print(i);
      Serial.print(": ");
      Serial.print(relayStates[i].name);
      Serial.print(", isOn: ");
      Serial.print(relayStates[i].isOn);
      Serial.print(", isAuto: ");
      Serial.print(relayStates[i].isAuto);
      Serial.print(", linkedSensor: ");
      Serial.print(relayStates[i].linkedSensor);
      Serial.println();
    }
};

void saveState()
{
  const int EEPROM_SIZE = 512;
  EEPROM.begin(EEPROM_SIZE);
  int addr = 0;
  const int MAX_NAME_LENGTH = 32;

  for (int i = 0; i < NUM_RELAYS; i++)
  {
    if (addr + 3 >= EEPROM_SIZE)
    {
      break;
    }

    // Zapisz stan podstawowy
    EEPROM.write(addr++, relayStates[i].isOn ? 1 : 0);
    EEPROM.write(addr++, relayStates[i].isAuto ? 1 : 0);
    
    // Zapisz długość nazwy
    byte nameLength = relayStates[i].name.length();
    if (nameLength > MAX_NAME_LENGTH) {
      nameLength = MAX_NAME_LENGTH;
    }
    EEPROM.write(addr++, nameLength);
    
    // Zapisz nazwę
    if (addr + nameLength >= EEPROM_SIZE) {
      continue;
    }
    
    for (int j = 0; j < nameLength; j++) {
      EEPROM.write(addr++, relayStates[i].name[j]);
    }

    yield();
  }

  bool commitSuccess = EEPROM.commit();
  if (!commitSuccess) {
  } else {
    printEPROM();
   
  }
}

void loadState()
{
  const int EEPROM_SIZE = 512;
  const int MAX_NAME_LENGTH = 32;
  EEPROM.begin(EEPROM_SIZE);
  int addr = 0;


  // Inicjalizuj wszystkie przekaźniki do stanu domyślnego
  for (int i = 0; i < NUM_RELAYS; i++) {
    relayStates[i].isOn = false;
    relayStates[i].isAuto = true;
    relayStates[i].name = relayNames[i];
    relayStates[i].linkedSensor = "";
    relayStates[i].isForcedByThermostat = false;
  }

  // Próba odczytu danych z EEPROM
  for (int i = 0; i < NUM_RELAYS; i++) {
    if (addr + 3 >= EEPROM_SIZE) {
      break;
    }

    // Odczytaj stan podstawowy
    relayStates[i].isOn = EEPROM.read(addr++) == 1;
    relayStates[i].isAuto = EEPROM.read(addr++) == 1;
    
    // Odczytaj długość nazwy
    byte nameLength = EEPROM.read(addr++);
    if (nameLength > MAX_NAME_LENGTH) {
      continue;
    }
    
    // Odczytaj nazwę
    if (addr + nameLength > EEPROM_SIZE) {
      continue;
    }
    
    char name[MAX_NAME_LENGTH + 1] = {0};
    for (int j = 0; j < nameLength; j++) {
      name[j] = EEPROM.read(addr++);
    }
    relayStates[i].name = String(name);

    // Ustaw stan fizyczny przekaźnika
    ExpOutput.digitalWrite(relayPins[i], relayStates[i].isOn ? LOW : HIGH);

    String relaySummary = "PODSUMOWANIE przekaźnika " + String(i) + ":\n" +
                         "  - Nazwa: " + relayStates[i].name + "\n" +
                         "  - Stan: " + String(relayStates[i].isOn ? "WŁĄCZONY" : "WYŁĄCZONY") + "\n" +
                         "  - Tryb: " + String(relayStates[i].isAuto ? "AUTOMATYCZNY" : "RĘCZNY") + "\n" +
                         "  - Adres EEPROM: " + String(addr - nameLength - 3);
    addDebugLog(relaySummary);
    
    yield();
  }

  addDebugLog("\n----- EEPROM: Zakończono wczytywanie stanu -----");
  
  // Wyślij pełny stan po załadowaniu
  sendRelayStates();
}

// Dodaj po deklaracji iotWebConf
void wifiConnected()
{
  String message = "WiFi connected, IP: " + WiFi.localIP().toString();
  addDebugLog(message);
  
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
  addDebugLog("Configuration was updated");
}

void readConfigFile() {
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
  
  addDebugLog("Konfiguracja została załadowana z pamięci");
}

void setup()
{
  Serial.begin(9600);
  
  readConfigFile();
  Wire.begin();
  
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

  iotWebConf.setConfigPin(CONFIG_PIN);
  iotWebConf.setStatusPin(LED_BUILTIN);
  
  delay(3000);

  iotWebConf.setApTimeoutMs(0);
  iotWebConf.init();

  

  server.on("/", handleRoot);
  server.on("/config", []{ iotWebConf.handleConfig(); });
  server.onNotFound([](){ iotWebConf.handleNotFound(); });
  server.on("/options", HTTP_OPTIONS, handleOptions); // Dodajemy obsługę OPTIONS
  

  server.begin();

  prepareDataForWebServer();

  webSocket.begin();
  webSocket.onEvent(onWsEvent);

  initInputExpander();
  initOutputExpander();

  otaStart();

  loadState();

  // Wyślij informację o restarcie
  String resetJSON;
  serializeJson(resetDoc, resetJSON);
  webSocket.broadcastTXT(resetJSON);
  
}

void loop()
{
  //ESP.wdtFeed(); // Reset watchdog timer

  // Sprawdź czy przycisk jest wciśnięty (stan niski bo INPUT_PULLUP)
  if (digitalRead(CONFIG_PIN) == LOW)
  {
    if (buttonDownTime == 0) // Przycisk został właśnie wciśnięty
    {
      addDebugLog("Button pressed");
      addDebugLog("time " + String(millis()));
      buttonDownTime = millis();
     
    }
    else if ((millis() - buttonDownTime) > CONFIG_RESET_TIMEOUT)
    {
      addDebugLog("Resetting configuration...");
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
  
  if (iotWebConf.getState() == 4)
  {
    if (ESP.getFreeHeap() < 4096)
    { // Ostrzeżenie przy małej ilości pamięci
      addDebugLog("Low memory: " + String(ESP.getFreeHeap()));
    }

   // checkThermostatInputs(); // Dodaj sprawdzanie stanów termostatów
    
    manifoldLogic();

    webSocket.loop();

    // Wysyłaj logi co 5 sekund
    static unsigned long lastLogSend = 0;
    if (millis() - lastLogSend > 5000) {
      sendDebugLogs();
      lastLogSend = millis();
    }
  }
  server.handleClient();
  ArduinoOTA.handle();
}

// Obsługa eventow Websocket
void onWsEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{
  switch (type)
  {
  case WStype_DISCONNECTED:
    addDebugLog("Client disconnected from WebSocket");
    break;
  case WStype_CONNECTED:
  {
    addDebugLog("New client connected to WebSocket");
    // Wyślij pełny stan przekaźników
    sendRelayStates();
    // Wyślij zebrane logi
    sendDebugLogs();
  }
  break;
  case WStype_TEXT:
  {
    StaticJsonDocument<2048> docInput;
    String messageText = String((char *)payload).substring(0, length);
    
    // Daj na serial wiadomość w celu debugu 

    Serial.println("Received JSON: ");



    DeserializationError error = deserializeJson(docInput, messageText);
    if (error) {
      Serial.println("Błąd parsowania JSON: " + String(error.c_str()));
      return;
    }

    
    
    if (docInput["type"] == "command") {
      // Daj na serial wiadomość w celu debugu  command i action
      Serial.println("Received command:");
      //messageText do serial.println(messageText);
      Serial.println(messageText);
      if (docInput["action"] == "toggleRelay") {
        int relayId = docInput["relayId"];
        if (relayId >= 0 && relayId < NUM_RELAYS) {
          bool newState = !relayStates[relayId].isOn;
          updateRelayState(relayId, newState);
          addDebugLog("Przełączono przekaźnik " + String(relayId) + " na " + (newState ? "ON" : "OFF"));
        }
      }
    }
    // Obsługa starego formatu dla kompatybilności
    // else if (docInput["pin"]) {
    //   String pinStr = docInput["pin"];
    //   if (pinStr.startsWith("pin_")) {
    //     int relayId = pinStr.substring(4).toInt();
    //     if (relayId >= 0 && relayId < NUM_RELAYS) {
    //       bool newState = docInput["state"] == "ON";
    //       updateRelayState(relayId, newState);
    //       addDebugLog("Przełączono przekaźnik " + String(relayId) + " na " + (newState ? "ON" : "OFF"));
    //     }
    //   }
    // }
  }
  break;
  }
}
