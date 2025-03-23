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

// #include <Wire.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_SSD1306.h>

// Adafruit_SSD1306 display(128, 64, &Wire, -1); // Użyj -1, jeśli nie używasz resetu

const int NUM_RELAYS = 6;                             // liczba przekaźników
const int relayPins[NUM_RELAYS] = {0, 1, 2, 3, 4, 5}; // piny przekaźników na ekspanderze
const char* relayNames[NUM_RELAYS] = {
  "Przedpokój",
  "Sypialnia",
  "Kuchnia",
  "Łazienka",
  "Klatka schodowa",
  "Gościnny"
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
const char DEFAULT_WIFI_SSID[] = "oooooio";
const char DEFAULT_WIFI_PASSWORD[] = "pmgana921";

void handleRoot();
DNSServer dnsServer;
WebServer server(80);
IotWebConf iotWebConf(thingName, &dnsServer, &server, wifiInitialApPassword);

String forced;
String woodStove;
String state;
String pin;

// Struktura do przechowywania stanu przekaźnika
struct RelayState
{
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
    server.sendHeader("Cache-Control", "no-cache"); // Dodaj nagłówek Cache-Control
    server.send(200, "text/html", webpage);
  }
}

// utworzenie obiektu klasy Timers z trzema odliczającymi
// Timers<3> timers;

// obiekty ekspanderów PCF8574
PCF8574* ExpInput = nullptr;
PCF8574* ExpOutput = nullptr;

float manifoldTemp;

void manifoldLogic()
{
  bool anyForcedON = 0;
  bool anyInputON = 0;
  docPins["manifoldTemp"] = manifoldTemp;
  unsigned long currentMillis = millis();

  delay(400);

  for (int i = 0; i < 6; i++)
  {
    String curr_pin = String(ExpInput->digitalRead(i));

    if (docPins["pins"]["pin_" + String(i)]["forced"] == "true")
    {
      anyForcedON = 1;
    }
  }

  if (String(ExpInput->digitalRead(P6)) == "true")
  {
    docPins["WoodStove"] = "on";
  }
  if (String(ExpInput->digitalRead(P6)) == "false")
  {
    docPins["WoodStove"] = "off";
  }

  if (anyForcedON == 1)
  {
    for (int i = 0; i < 6; i++)
    {
      if (docPins["pins"]["pin_" + String(i)]["forced"] == "true")
      {
        ExpOutput->digitalWrite(i, HIGH);
      }
      else
      {
        ExpOutput->digitalWrite(i, LOW);
      }
    }
  }

  if (anyForcedON == 0)
  {
    ExpOutput->digitalWrite(P7, HIGH); // Pompa OFF

    for (int i = 0; i < 6; i++)
    {
      ExpOutput->digitalWrite(i, HIGH); // Pinoutput OFF
    }
  }

  if (millis() - lastSendTime > 9000)
  {
    String outputJSON;
    serializeJson(docPins, outputJSON);
    addDebugLog("JSON status: " + outputJSON);

    webSocket.broadcastTXT(outputJSON);
    lastSendTime = millis();

    addDebugLog("Status: anyInputON=" + String(anyInputON) + ", anyForcedON=" + String(anyForcedON));
  }
}

void otaStart();

// Funkcja skanująca urządzenia I2C
void scanI2CDevices() {
  addDebugLog("Skanowanie urządzeń I2C...");
  byte error, address;
  int deviceCount = 0;
  
  for(address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    
    if (error == 0) {
      deviceCount++;
      addDebugLog("Znaleziono urządzenie I2C pod adresem 0x" + String(address, HEX));
      
      // Sprawdź czy to PCF8574
      if (address == 0x20 || address == 0x26) {
        addDebugLog("Wykryto PCF8574 pod adresem 0x" + String(address, HEX));
      }
    }
  }
  
  if (deviceCount == 0) {
    addDebugLog("Nie znaleziono żadnych urządzeń I2C!");
  } else {
    addDebugLog("Znaleziono " + String(deviceCount) + " urządzeń I2C");
  }
}
void blinkOutput(int timer)
{
  addDebugLog("Test migania diod - wszystkie wyjścia");
  
  // Włącz wszystkie wyjścia
  for (int i = 0; i < 7; i++)
  {
    ExpOutput->digitalWrite(i, LOW);
    addDebugLog("Włączono wyjście " + String(i));
  }
  delay(timer);
  
  // Wyłącz wszystkie wyjścia
  for (int i = 0; i < 7; i++)
  {
    ExpOutput->digitalWrite(i, HIGH);
    addDebugLog("Wyłączono wyjście " + String(i));
  }
  
  addDebugLog("Test migania zakończony");
}
void initInputExpander()
{
  addDebugLog("Inicjalizacja ekspandera wejść...");
  
  // Próba znalezienia ekspandera wejść
  ExpInput = new PCF8574(0x20);
  
  if (ExpInput->begin()) {
    addDebugLog("Ekspander wejść (0x20) zainicjalizowany pomyślnie");
    
    // ustawienie pinów jako wejścia i włączenie wbudowanych rezystorów podciągających
    ExpInput->pinMode(P0, INPUT_PULLUP);
    ExpInput->pinMode(P1, INPUT_PULLUP);
    ExpInput->pinMode(P2, INPUT_PULLUP);
    ExpInput->pinMode(P3, INPUT_PULLUP);
    ExpInput->pinMode(P4, INPUT_PULLUP);
    ExpInput->pinMode(P5, INPUT_PULLUP);
    ExpInput->pinMode(P6, INPUT_PULLUP);
    ExpInput->pinMode(P7, INPUT_PULLUP);
    
    // Ustawienie stanu początkowego
    ExpInput->digitalWrite(P0, HIGH);
    ExpInput->digitalWrite(P1, HIGH);
    ExpInput->digitalWrite(P2, HIGH);
    ExpInput->digitalWrite(P3, HIGH);
    ExpInput->digitalWrite(P4, HIGH);
    ExpInput->digitalWrite(P5, HIGH);
    ExpInput->digitalWrite(P6, HIGH);
    ExpInput->digitalWrite(P7, HIGH);
  } else {
    addDebugLog("BŁĄD: Nie udało się zainicjalizować ekspandera wejść (0x20)");
  }
}


void initOutputExpander()
{
  addDebugLog("Inicjalizacja ekspandera wyjść...");
  
  // Próba znalezienia ekspandera wyjść
  ExpOutput = new PCF8574(0x26);
  
  if (ExpOutput->begin()) {
    addDebugLog("Ekspander wyjść (0x26) zainicjalizowany pomyślnie");
    
    for (int i = 0; i < 7; i++) {
      ExpOutput->pinMode(i, OUTPUT);
      ExpOutput->digitalWrite(i, HIGH);
    }
    
    // Wykonaj test migania diod
    blinkOutput(1000);
  } else {
    addDebugLog("BŁĄD: Nie udało się zainicjalizować ekspandera wyjść (0x26)");
  }
}


void useGaz(void)
{

  ExpOutput->digitalWrite(P6, LOW); // LED ON
  ExpOutput->digitalWrite(P7, LOW); // Pompa ON
  docPins["piec_pompa"] = "ON";
  docPins["led"] = "ON";
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
    String message = "{\"response\":\"connected\"}";
    webSocket.sendTXT(num, message);
    // Wyślij zebrane logi po połączeniu
    sendDebugLogs();
  }
  break;
  case WStype_TEXT:
  {
    StaticJsonDocument<200> docInput;
    // handle incoming text message from client
    String messageText = String((char *)payload).substring(0, length);
    addDebugLog("Otrzymano wiadomość: " + messageText);
    
    // parse the JSON message
    DeserializationError error = deserializeJson(docInput, messageText);
    if (error)
    {
      addDebugLog("Błąd parsowania JSON: " + String(error.c_str()));
      return;
    }

    if (docInput["usegaz"])
    {
      String gas = String(docInput["usegaz"]);
      addDebugLog("Wykryto usegaz: " + gas);
      useGaz_ = (gas == "true");
    }

    // Sprawdź czy wiadomość zawiera informacje o pinie
    if (docInput["pin"])
    {
      String pin = String(docInput["pin"]);
      String state = String(docInput["state"]);
      String forced = String(docInput["forced"]);
      
      addDebugLog("Przetwarzanie wiadomości dla pinu: " + pin + ", stan: " + state + ", forced: " + forced);

      // Konwertuj "pin_X" na numer pinu
      int pinNumber = pin.substring(4).toInt();
      if (pinNumber >= 0 && pinNumber < NUM_RELAYS) {
        bool newState = (state == "ON");
        
        // Aktualizuj stan w docPins
        docPins[pin]["state"] = state;
        docPins[pin]["forced"] = forced;
        
        // Steruj przekaźnikiem
        ExpOutput->digitalWrite(pinNumber, newState ? LOW : HIGH);
        addDebugLog("Ustawiono stan pinu " + String(pinNumber) + " na " + (newState ? "ON" : "OFF"));
      } else {
        addDebugLog("Nieprawidłowy numer pinu: " + String(pinNumber));
      }
    }
  }
  break;
  }
}

void saveState()
{
  const int EEPROM_SIZE = 512;
  EEPROM.begin(EEPROM_SIZE);
  int addr = 0;
  const int MAX_SENSOR_NAME = 32;

  for (int i = 0; i < NUM_RELAYS; i++)
  {
    if (addr + 2 >= EEPROM_SIZE)
    {
      addDebugLog("EEPROM: Brak miejsca na zapis stanu przekaźnika");
      break;
    }

    EEPROM.write(addr++, relayStates[i].isOn ? 1 : 0);
    EEPROM.write(addr++, relayStates[i].isAuto ? 1 : 0);

    byte sensorNameLength = relayStates[i].linkedSensor.length();
    if (sensorNameLength > MAX_SENSOR_NAME)
    {
      sensorNameLength = MAX_SENSOR_NAME;
      addDebugLog("EEPROM: Nazwa czujnika zbyt długa, przycinanie");
    }

    if (addr + 1 + sensorNameLength >= EEPROM_SIZE)
    {
      addDebugLog("EEPROM: Brak miejsca na zapis nazwy czujnika");
      EEPROM.write(addr++, 0);
      continue;
    }

    EEPROM.write(addr++, sensorNameLength);

    for (int j = 0; j < sensorNameLength; j++)
    {
      EEPROM.write(addr++, relayStates[i].linkedSensor[j]);
    }

    yield();
  }

  bool commitSuccess = EEPROM.commit();
  if (!commitSuccess)
  {
    addDebugLog("EEPROM: Błąd podczas zapisu");
  }
  else
  {
    addDebugLog("EEPROM: Zapisano stan przekaźników");
  }
}

void loadState()
{
  const int EEPROM_SIZE = 512;
  const int MAX_SENSOR_NAME = 32;
  EEPROM.begin(EEPROM_SIZE);
  int addr = 0;

  addDebugLog("\n----- EEPROM: Wczytywanie stanu przekaźników -----");
  addDebugLog("Adres EEPROM: 0x" + String(addr, HEX));
  addDebugLog("Rozmiar EEPROM: " + String(EEPROM_SIZE) + " bajtów");
  addDebugLog("Liczba przekaźników: " + String(NUM_RELAYS));

  // Wyświetl pierwsze 16 bajtów EEPROM w celach diagnostycznych
  String firstBytes = "Pierwsze 16 bajtów EEPROM: ";
  for (int i = 0; i < 16 && i < EEPROM_SIZE; i++)
  {
    byte value = EEPROM.read(i);
    firstBytes += "0x";
    if (value < 16)
      firstBytes += "0";
    firstBytes += String(value, HEX) + " ";
  }
  addDebugLog(firstBytes);

  // Inicjalizuj wszystkie przekaźniki do stanu domyślnego
  for (int i = 0; i < NUM_RELAYS; i++)
  {
    relayStates[i].isOn = false;
    relayStates[i].isAuto = true;
    relayStates[i].linkedSensor = "";
  }

  // Próba odczytu danych z EEPROM
  for (int i = 0; i < NUM_RELAYS; i++)
  {
    addDebugLog("\n--- Przekaźnik " + String(i) + " (adres: 0x" + String(addr, HEX) + ") ---");

    if (addr + 2 >= EEPROM_SIZE)
    {
      addDebugLog("BŁĄD: Brak miejsca w EEPROM dla przekaźnika " + String(i));
      break;
    }

    byte isOnByte = EEPROM.read(addr);
    byte isAutoByte = EEPROM.read(addr + 1);

    String stateInfo = "Stan (bajt): 0x" + String(isOnByte, HEX) + 
                      ", Auto (bajt): 0x" + String(isAutoByte, HEX);
    addDebugLog(stateInfo);

    if (isOnByte > 1 || isAutoByte > 1)
    {
      addDebugLog("UWAGA: Nieprawidłowe wartości stanu przekaźnika, używam domyślnych");
      addr += 2;
      continue;
    }

    relayStates[i].isOn = isOnByte == 1;
    relayStates[i].isAuto = isAutoByte == 1;
    addr += 2;

    if (addr >= EEPROM_SIZE)
    {
      addDebugLog("BŁĄD: Brak danych o długości nazwy czujnika");
      continue;
    }

    byte sensorNameLength = EEPROM.read(addr++);
    String sensorLengthInfo = "Długość nazwy czujnika (bajt): 0x" + 
                            String(sensorNameLength, HEX) + 
                            " (" + String(sensorNameLength) + " znaków)";
    addDebugLog(sensorLengthInfo);

    if (sensorNameLength > MAX_SENSOR_NAME || sensorNameLength == 0)
    {
      addDebugLog("BŁĄD: Nieprawidłowa długość nazwy czujnika");
      continue;
    }

    if (addr + sensorNameLength > EEPROM_SIZE)
    {
      addDebugLog("BŁĄD: Brak pełnych danych nazwy czujnika");
      continue;
    }

    char sensorName[MAX_SENSOR_NAME + 1] = {0};

    String sensorData = "Dane nazwy czujnika (hex): ";
    for (int j = 0; j < sensorNameLength && j < MAX_SENSOR_NAME; j++)
    {
      byte charValue = EEPROM.read(addr + j);
      sensorData += "0x";
      if (charValue < 16)
        sensorData += "0";
      sensorData += String(charValue, HEX) + " ";

      if (charValue >= 32 && charValue <= 126)
      {
        sensorName[j] = (char)charValue;
      }
      else
      {
        addDebugLog("UWAGA: Wykryto nieprawidłowy znak ASCII: " + String(charValue));
        sensorName[j] = '_';
      }
    }
    addDebugLog(sensorData);

    addr += sensorNameLength;
    relayStates[i].linkedSensor = String(sensorName);

    ExpOutput->digitalWrite(relayPins[i], relayStates[i].isOn ? LOW : HIGH);

    String relaySummary = "PODSUMOWANIE przekaźnika " + String(i) + ":\n" +
                         "  - Stan: " + String(relayStates[i].isOn ? "WŁĄCZONY" : "WYŁĄCZONY") + "\n" +
                         "  - Tryb: " + String(relayStates[i].isAuto ? "AUTOMATYCZNY" : "RĘCZNY") + "\n" +
                         "  - Czujnik: '" + relayStates[i].linkedSensor + "'";
    addDebugLog(relaySummary);

    yield();
  }

  addDebugLog("\n----- EEPROM: Zakończono wczytywanie stanu -----");

  if (addr < 6)
  {
    addDebugLog("UWAGA: Wykryto problemy z danymi EEPROM, inicjalizuję domyślnymi wartościami");
    for (int i = 0; i < NUM_RELAYS; i++)
    {
      relayStates[i].isOn = false;
      relayStates[i].isAuto = true;
      relayStates[i].linkedSensor = "";
      ExpOutput->digitalWrite(relayPins[i], HIGH);
    }
    saveState();
  }
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

void setup()
{
  Serial.begin(9600);
  addDebugLog("Starting up...");
  
  // Inicjalizacja I2C
  Wire.begin();
  addDebugLog("I2C zainicjalizowany");
  
  // Skanowanie urządzeń I2C
  scanI2CDevices();
  
  pinMode(CONFIG_PIN, INPUT_PULLUP);
  addDebugLog("Config pin initialized");

  // Debugowanie przyczyny restartu
  myResetInfo = ESP.getResetInfoPtr();
  String resetReason = "Reset reason: " + String(myResetInfo->reason);
  addDebugLog(resetReason);
  addDebugLog("Free heap: " + String(ESP.getFreeHeap()));

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
  addDebugLog("IotWebConf initialized");
  
  delay(3000);
  addDebugLog("AP mode timeout set to 10 seconds");

  iotWebConf.setApTimeoutMs(10000);
  addDebugLog("Initializing IotWebConf...");
  iotWebConf.init();

  server.on("/", handleRoot);
  server.on("/config", []{ iotWebConf.handleConfig(); });
  server.onNotFound([](){ iotWebConf.handleNotFound(); });
  addDebugLog("Web server routes configured");

  server.begin();
  addDebugLog("HTTP server started");

  prepareDataForWebServer();
  addDebugLog("Web server data prepared");

  webSocket.begin();
  webSocket.onEvent(onWsEvent);
  addDebugLog("WebSocket server started");

  initInputExpander();
  initOutputExpander();
  addDebugLog("I/O expanders initialized");

  otaStart();
  addDebugLog("OTA initialized");

  loadState();
  addDebugLog("Relay states loaded from EEPROM");

  // Wyślij informację o restarcie
  String resetJSON;
  serializeJson(resetDoc, resetJSON);
  webSocket.broadcastTXT(resetJSON);
  
  addDebugLog("Setup completed successfully");
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
      // load state and print
      loadState();
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

    // Dodaj yield() w długich operacjach
    //  yield(); // Pozwól watchdogowi się zresetować

    // Dodaj to w głównej pętli
    if (millis() - lastSaveTime >= SAVE_INTERVAL)
    {
      saveState();
      lastSaveTime = millis();
    }
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

// Zmodyfikuj funkcję obsługującą zmianę stanu przekaźnika
void handleRelay(int relayIndex, bool state)
{
  ExpOutput->digitalWrite(relayPins[relayIndex], state ? LOW : HIGH);
  relayStates[relayIndex].isOn = state;
  // Opcjonalnie możesz tu dodać natychmiastowy zapis stanu
  // saveState();
}
