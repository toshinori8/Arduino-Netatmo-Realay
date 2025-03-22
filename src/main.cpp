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

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 display(128, 64, &Wire, -1); // Użyj -1, jeśli nie używasz resetu

const int NUM_RELAYS = 6;                             // liczba przekaźników
const int relayPins[NUM_RELAYS] = {0, 1, 2, 3, 4, 5}; // piny przekaźników na ekspanderze

unsigned long lastSendTime = 0;
unsigned long previousMillis = 0;      // Variable to store the previous time
const long interval = 480 * 60 * 1000; // Interval at which to reset the NodeMCU
WebSocketsServer webSocket(8080);

bool useGaz_ = true; // use gas? button on webPage

StaticJsonDocument<200> docPins;

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
PCF8574 ExpInput(0x20);  // utworzenie obiektu dla pierwszego ekspandera
PCF8574 ExpOutput(0x26); // utworzenie obiektu dla drugiego ekspandera


float manifoldTemp;

void manifoldLogic()
{

  bool anyForcedON = 0;
  bool anyInputON = 0;
  docPins["manifoldTemp"] = manifoldTemp;
  unsigned long currentMillis = millis(); // Get the current time

  delay(400);

  // Update JSON doc from six states on first expander

  for (int i = 0; i < 6; i++)
  {
    String curr_pin = String(ExpInput.digitalRead(i));

    if (docPins["pins"]["pin_" + String(i)]["forced"] == "true")
    {
      anyForcedON = 1;
    }
  }

  // check woodStove init input

  // Serial.println(String(ExpInput.digitalRead(P6)) + " ExpInput.digital P6");
  if (String(ExpInput.digitalRead(P6)) == "true")
  {

    // Serial.println("WoodStove operating ");
    docPins["WoodStove"] = "on";
  }
  if (String(ExpInput.digitalRead(P6)) == "false")
  {

    // Serial.println("WoodStove off ");
    docPins["WoodStove"] = "off";
  }

  if (anyForcedON == 1)
  {
    for (int i = 0; i < 6; i++)
    {

      if (docPins["pins"]["pin_" + String(i)]["forced"] == "true")
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
    // Serial.println("Pump OFF");
    //   ExpOutput.digitalWrite(P6, HIGH); // LED OFF
    ExpOutput.digitalWrite(P7, HIGH); // Pompa OFF
    //   doc["piec_pompa"] = "OFF";
    //   doc["led"] = "OFF";

    // Serial.println("All valves OFF");
    for (int i = 0; i < 6; i++)
    {

      ExpOutput.digitalWrite(i, HIGH); // Pinoutput OFF
    }
  }
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
      docPins[pin]["state"] = "ON";
      docPins[pin]["forced"] = "true";
    }
    else
    {

      docPins[pin]["state"] = "OFF";
      docPins[pin]["forced"] = "false";
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

void saveState()
{
  const int EEPROM_SIZE = 512;
  EEPROM.begin(EEPROM_SIZE);
  int addr = 0;
  const int MAX_SENSOR_NAME = 32; // Maksymalna długość nazwy czujnika

  for (int i = 0; i < NUM_RELAYS; i++)
  {
    // Sprawdź czy mamy wystarczająco miejsca na podstawowe dane (2 bajty)
    if (addr + 2 >= EEPROM_SIZE)
    {
      Serial.println("EEPROM: Brak miejsca na zapis stanu przekaźnika");
      break;
    }

    EEPROM.write(addr++, relayStates[i].isOn ? 1 : 0);
    EEPROM.write(addr++, relayStates[i].isAuto ? 1 : 0);

    // Ogranicz długość nazwy czujnika
    byte sensorNameLength = relayStates[i].linkedSensor.length();
    if (sensorNameLength > MAX_SENSOR_NAME)
    {
      sensorNameLength = MAX_SENSOR_NAME;
      Serial.println("EEPROM: Nazwa czujnika zbyt długa, przycinanie");
    }

    // Sprawdź czy mamy wystarczająco miejsca na długość nazwy i samą nazwę
    if (addr + 1 + sensorNameLength >= EEPROM_SIZE)
    {
      Serial.println("EEPROM: Brak miejsca na zapis nazwy czujnika");
      EEPROM.write(addr++, 0); // Zapisz pustą nazwę
      continue;
    }

    EEPROM.write(addr++, sensorNameLength);

    // Zapisz nazwę czujnika
    for (int j = 0; j < sensorNameLength; j++)
    {
      EEPROM.write(addr++, relayStates[i].linkedSensor[j]);
    }

    yield(); // Daj czas watchdogowi na reset
  }

  // Zapisz zmiany
  bool commitSuccess = EEPROM.commit();
  if (!commitSuccess)
  {
    Serial.println("EEPROM: Błąd podczas zapisu");
  }
  else
  {
    Serial.println("EEPROM: Zapisano stan przekaźników");
  }
}

void loadState()
{
  const int EEPROM_SIZE = 512;
  const int MAX_SENSOR_NAME = 32;
  EEPROM.begin(EEPROM_SIZE);
  int addr = 0;

  Serial.println("\n----- EEPROM: Wczytywanie stanu przekaźników -----");
  Serial.println("Adres EEPROM: 0x" + String(addr, HEX));
  Serial.println("Rozmiar EEPROM: " + String(EEPROM_SIZE) + " bajtów");
  Serial.println("Liczba przekaźników: " + String(NUM_RELAYS));

  // Wyświetl pierwsze 16 bajtów EEPROM w celach diagnostycznych
  Serial.print("Pierwsze 16 bajtów EEPROM: ");
  for (int i = 0; i < 16 && i < EEPROM_SIZE; i++)
  {
    byte value = EEPROM.read(i);
    Serial.print("0x");
    if (value < 16)
      Serial.print("0");
    Serial.print(value, HEX);
    Serial.print(" ");
  }
  Serial.println();

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
    Serial.println("\n--- Przekaźnik " + String(i) + " (adres: 0x" + String(addr, HEX) + ") ---");

    // Sprawdź czy mamy wystarczająco danych do odczytu
    if (addr + 2 >= EEPROM_SIZE)
    {
      Serial.println("BŁĄD: Brak miejsca w EEPROM dla przekaźnika " + String(i));
      break;
    }

    // Odczytaj podstawowe dane
    byte isOnByte = EEPROM.read(addr);
    byte isAutoByte = EEPROM.read(addr + 1);

    Serial.print("Stan (bajt): 0x");
    Serial.print(isOnByte, HEX);
    Serial.print(", Auto (bajt): 0x");
    Serial.println(isAutoByte, HEX);

    // Sprawdź poprawność danych
    if (isOnByte > 1 || isAutoByte > 1)
    {
      Serial.println("UWAGA: Nieprawidłowe wartości stanu przekaźnika, używam domyślnych");
      addr += 2; // Przejdź do następnych danych
      continue;
    }

    relayStates[i].isOn = isOnByte == 1;
    relayStates[i].isAuto = isAutoByte == 1;
    addr += 2;

    // Odczytaj długość nazwy czujnika
    if (addr >= EEPROM_SIZE)
    {
      Serial.println("BŁĄD: Brak danych o długości nazwy czujnika");
      continue;
    }

    byte sensorNameLength = EEPROM.read(addr++);
    Serial.print("Długość nazwy czujnika (bajt): 0x");
    Serial.print(sensorNameLength, HEX);
    Serial.print(" (");
    Serial.print(sensorNameLength);
    Serial.println(" znaków)");

    // Sprawdź czy długość nazwy jest sensowna
    if (sensorNameLength > MAX_SENSOR_NAME || sensorNameLength == 0)
    {
      Serial.println("BŁĄD: Nieprawidłowa długość nazwy czujnika");
      continue;
    }

    // Sprawdź czy mamy wystarczająco danych do odczytu nazwy
    if (addr + sensorNameLength > EEPROM_SIZE)
    {
      Serial.println("BŁĄD: Brak pełnych danych nazwy czujnika");
      continue;
    }

    // Odczytaj nazwę czujnika
    char sensorName[MAX_SENSOR_NAME + 1] = {0}; // +1 na null terminator

    Serial.print("Dane nazwy czujnika (hex): ");
    for (int j = 0; j < sensorNameLength && j < MAX_SENSOR_NAME; j++)
    {
      byte charValue = EEPROM.read(addr + j);
      Serial.print("0x");
      if (charValue < 16)
        Serial.print("0");
      Serial.print(charValue, HEX);
      Serial.print(" ");

      // Sprawdź czy znak jest drukowalny (ASCII 32-126)
      if (charValue >= 32 && charValue <= 126)
      {
        sensorName[j] = (char)charValue;
      }
      else
      {
        Serial.print("\nUWAGA: Wykryto nieprawidłowy znak ASCII: ");
        Serial.println(charValue);
        sensorName[j] = '_'; // Zastąp nieprawidłowy znak
      }
    }
    Serial.println();

    addr += sensorNameLength;
    relayStates[i].linkedSensor = String(sensorName);

    // Zastosuj odczytany stan do przekaźnika
    ExpOutput.digitalWrite(relayPins[i], relayStates[i].isOn ? LOW : HIGH);

    Serial.println("PODSUMOWANIE przekaźnika " + String(i) + ":");
    Serial.println("  - Stan: " + String(relayStates[i].isOn ? "WŁĄCZONY" : "WYŁĄCZONY"));
    Serial.println("  - Tryb: " + String(relayStates[i].isAuto ? "AUTOMATYCZNY" : "RĘCZNY"));
    Serial.println("  - Czujnik: '" + relayStates[i].linkedSensor + "'");

    yield(); // Daj czas watchdogowi
  }

  Serial.println("\n----- EEPROM: Zakończono wczytywanie stanu -----");

  // Jeśli wykryto problemy, zainicjuj EEPROM domyślnymi wartościami
  if (addr < 6)
  { // Jeśli odczytano mniej niż 6 bajtów (minimum dla jednego przekaźnika)
    Serial.println("UWAGA: Wykryto problemy z danymi EEPROM, inicjalizuję domyślnymi wartościami");
    for (int i = 0; i < NUM_RELAYS; i++)
    {
      relayStates[i].isOn = false;
      relayStates[i].isAuto = true;
      relayStates[i].linkedSensor = "";
      ExpOutput.digitalWrite(relayPins[i], HIGH); // Wyłącz wszystkie przekaźniki
    }
    // Zapisz domyślne wartości
    saveState();
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

  Wire.begin();                              // Inicjalizacja magistrali I2C
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Adres OLED

  // Dodaj na początku setup
  // ESP.wdtDisable();                       // Wyłącz watchdog na czas setupu
  // ESP.wdtEnable(WATCHDOG_TIMEOUT * 1000); // Włącz watchdog z timeoutem w ms

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
  // pauza na trzy sekundy dla przycisniecia przycisku flash by wlaczyc tryb AP
  delay(3000);

  // Zwiększ timeout na połączenie WiFi
  iotWebConf.setApTimeoutMs(10000);

  // Inicjalizacja z większym debugowaniem
  Serial.println("Starting up...");
  Serial.println("Initializing IotWebConf...");
  iotWebConf.init();

  /*  // Sprawdź czy mamy zapisaną konfigurację
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
   } */

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
  // Serial.println("WebSocket server started");

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
  // timers.attach(0, 1000, functionName);

  loadState(); // Dodaj to na końcu setup()

  // Po inicjalizacji WebSocket wyślij informację
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
      Serial.println("Button pressed");
      Serial.println("time " + String(millis()) + "");
      buttonDownTime = millis();
      // load state and print
      loadState();
    }
    else if ((millis() - buttonDownTime) > CONFIG_RESET_TIMEOUT)
    {
      Serial.println("Resetting configuration...");
      iotWebConf.getSystemParameterGroup()->applyDefaultValue(); // Reset do wartości domyślnych
      iotWebConf.saveConfig();                                   // Zapisz pustą konfigurację
      ESP.restart();                                             // Zrestartuj urządzenie */
      // save state
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
      Serial.println("Low memory: " + String(ESP.getFreeHeap()));
    }

    // Dodaj yield() w długich operacjach
  //  yield(); // Pozwól watchdogowi się zresetować

    // Dodaj to w głównej pętli
    if (millis() - lastSaveTime >= SAVE_INTERVAL)
    {
      saveState();
      lastSaveTime = millis();
    }


    webSocket.loop();

    manifoldLogic();
  
  
  }
     server.handleClient();
   ArduinoOTA.handle();

  
}

// Zmodyfikuj funkcję obsługującą zmianę stanu przekaźnika
void handleRelay(int relayIndex, bool state)
{
  digitalWrite(relayPins[relayIndex], state ? HIGH : LOW);
  relayStates[relayIndex].isOn = state;
  // Opcjonalnie możesz tu dodać natychmiastowy zapis stanu
  // saveState();
}
