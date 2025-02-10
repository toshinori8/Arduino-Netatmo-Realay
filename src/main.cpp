#include <Arduino.h>
#include <ArduinoOTA.h>
#include "PCF8574.h"
#include <Wire.h>
#include "Timers.h"
#include <ArduinoJson.h>
StaticJsonDocument<8192> docPins;
StaticJsonDocument<1124> docRooms;

#include <ESP8266WiFi.h>
#include <espnow.h>
#include <now.h>
#include <ESP8266WebServer.h>
#include <webPage.h>
#include <WebSocketsServer.h>
#include <functional>
#include <IotWebConf.h>
#include <roomManager.h>

bool useGaz_ = true; // use gas? button on webPage
const char thingName[] = "netatmo_relay";
const char wifiInitialApPassword[] = "12345678";
void handleRoot();

String forced;
String woodStove;
String state;
String pin;
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
  docPins["netatmoData"] = "";
}

void handleRoot()
{
  if (iotWebConf.handleCaptivePortal())
  {
    return;
  }
  server.send(200, "text/html", webpage);
}



void fetchNetatmo() {
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
// Obsługa eventow Websocket
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
    StaticJsonDocument<200> docInput;
    // handle incoming text message from client
    String messageText = String((char *)payload).substring(0, length);
    Serial.println("Message from client: " + messageText);
    // parse the JSON message
    DeserializationError error = deserializeJson(docInput, messageText);
    if (error)
    {
      Serial.println("Error parsing JSON");
      return;
    }else {
      Serial.println("JSON parsed");
      
    }
    //  {"id":206653929,"command":"act_temperature","targetTemperature":"15.5","forced":false}
    // {"id":1812451076,"command":"act_temperature","targetTemperature":"26.5","forced":false}	
   
   
   
   if(docInput["command"] == "act_temperature"){
      int id = docInput["id"];
      float targetTemperature = docInput["targetTemperature"];
      bool forced = docInput["forced"];
      // manager.updateOrAddRoom(RoomData(nullptr, id, nullptr, targetTemperature, nullptr, nullptr));
      manager.updateOrAddRoom(RoomData("", id, 0, targetTemperature, 0.0, false));
      manager.setTemperature(id, targetTemperature);
    }
// {"rooms":[],"meta":{"pin_0":{"state":"OFF","forced":"false"},"pin_1":{"state":"OFF","forced":"false"},"pin_2":{"state":"OFF","forced":"false"},"pin_3":{"state":"OFF","forced":"false"},"WoodStove":"off","manifoldTemp":0,"netatmoData":""}}

    //   int id = docInput["id"];
    //   float targetTemperature = docInput["targetTemperature"];
    //   bool forced = docInput["forced"];
    //   manager.updateOrAddRoom(RoomData(NULL, id, NULL, targetTemperature, NULL, NULL));
      

    // }

    // if (docInput["usegaz"])
    // {
    //   String gas = String(docInput["usegaz"]);
    //   Serial.println("usegaz - detected");
    //   if (gas == "true")
    //   {
    //     useGaz_ = true;
    //   }
    //   else
    //   {
    //     useGaz_ = false;
    //   }
    // }

    // extract the pin number and state from the JSON message
    // pin = String(docInput["pin"]);
    // forced = String(docInput["forced"]);
    // state = String(docInput["state"]);

    // if (forced == "true")
    // {
    //   docPins[pin]["state"] = "ON";
    //   docPins[pin]["forced"] = "true";
    // }
    // else
    // {
    //   docPins[pin]["state"] = "OFF";
    //   docPins[pin]["forced"] = "false";
    // }

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
   
    String data =  manager.getRoomsAsJson();
    webSocket.broadcastTXT(data);
}



void setup()
{

  Serial.begin(115200);

  Serial.println();
  Serial.println("Starting up...");

  // -- Initializing the configuration.
  iotWebConf.init();
  iotWebConf.setApTimeoutMs(5000);
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

}




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

    if (docPins["pin_" + String(i)]["forced"] == "true")
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

      if (docPins["pin_" + String(i)]["forced"] == "true")
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
    
    manifoldLogic();

    timers.process();
  }
}
