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
String state;
String pin;

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

  ExpInput.pinMode(P0, INPUT_PULLUP);
  ExpInput.pinMode(P1, INPUT_PULLUP);
  ExpInput.pinMode(P2, INPUT_PULLUP);
  ExpInput.pinMode(P3, INPUT_PULLUP);
  ExpInput.pinMode(P4, INPUT_PULLUP);
  ExpInput.pinMode(P5, INPUT_PULLUP);

  ExpInput.digitalWrite(P0, HIGH);
  ExpInput.digitalWrite(P1, HIGH);
  ExpInput.digitalWrite(P2, HIGH);
  ExpInput.digitalWrite(P3, HIGH);
  ExpInput.digitalWrite(P4, HIGH);
  ExpInput.digitalWrite(P5, HIGH);
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
        if(gas=="true"){
        useGaz_ = true;
        }else{
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

void setup()
{

  Serial.begin(9600);

  Serial.println();
  Serial.println("Starting up...");

  // -- Initializing the configuration.
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
}


void loop()
{
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

 if (anyForcedON==1){
  for (int i = 0; i < 6; i++)
  {
    
    
    if (doc["pin_" + String(i)]["forced"] == "true")
    {
      ExpOutput.digitalWrite(i, HIGH);
    }else{
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
    for (int i = 0; i < 6; i++){

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
    Serial.println(String(anyForcedON)+": anyForcedON");
  }

  // Sprawdzenie, czy upłynęło 20 minut od ostatniego restartu
  // if (currentMillis - previousMillis >= interval) {
  //   ESP.restart();  // Restart the NodeMCU
  //   previousMillis = currentMillis;
  // }

  iotWebConf.doLoop();
  // timers.process();
  webSocket.loop();
  ArduinoOTA.handle();
}
