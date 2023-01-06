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
unsigned long previousMillis = 0;  // Variable to store the previous time
const long interval = 480 * 60 * 1000;  // Interval at which to reset the NodeMCU 
WebSocketsServer webSocket(8080);






  StaticJsonDocument<200> doc;
  StaticJsonDocument<200> docInput;


 

#include <IotWebConf.h>
const char thingName[] = "Netatmo_Relay";
const char wifiInitialApPassword[] = "12345678";
void handleRoot();
DNSServer dnsServer;
WebServer server(80);
IotWebConf iotWebConf(thingName, &dnsServer, &server, wifiInitialApPassword);



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
PCF8574 ExInput(0x20);  // utworzenie obiektu dla pierwszego ekspandera
PCF8574 ExOutput(0x26); // utworzenie obiektu dla drugiego ekspandera


void otaStart();

void initInputExpander(){
// ustawienie pinów jako wejścia i włączenie wbudowanych rezystorów podciągających

ExInput.pinMode(P0, INPUT_PULLUP);
ExInput.pinMode(P1, INPUT_PULLUP);
ExInput.pinMode(P2, INPUT_PULLUP);
ExInput.pinMode(P3, INPUT_PULLUP);
ExInput.pinMode(P4, INPUT_PULLUP);
ExInput.pinMode(P5, INPUT_PULLUP);

ExInput.digitalWrite(P0, HIGH);
ExInput.digitalWrite(P1, HIGH);
ExInput.digitalWrite(P2, HIGH);
ExInput.digitalWrite(P3, HIGH);
ExInput.digitalWrite(P4, HIGH);
ExInput.digitalWrite(P5, HIGH);
};

void initOutputExpander(){
   for (int i = 0; i < 7; i++) {
 
  ExOutput.pinMode(i, OUTPUT);
  ExOutput.digitalWrite(i,  HIGH);
}
};

void blinkOutput(int timer){
  for (int i = 0; i < 7; i++) {
  ExOutput.digitalWrite(i,  LOW);
  delay(timer);
  ExOutput.digitalWrite(i,  HIGH);
  }
}


// Obsługa eventow Websocket 
void onWsEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
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
        // handle incoming text message from client
        String messageText = String((char*)payload).substring(0, length);
        // parse the JSON message
        DeserializationError error = deserializeJson(docInput, messageText);
        if (error) {
          Serial.println("Error parsing JSON");
          return;
        }
        // extract the pin number and state from the JSON message
        int pin = docInput["pin"];
        int state = docInput["state"];
        //if (state == "LOW"){state=LOW;};
        
        Serial.println(messageText);
                  
         ExInput.digitalWrite(P0, HIGH);
        // set the state of the input pin on the PCF8574 input expander
        
      }
      break;
  }
}




void setup() {

  Serial.begin(9600);

  Serial.println();
  Serial.println("Starting up...");

  // -- Initializing the configuration.
  iotWebConf.init();

  // -- Set up required URL handlers on the web server.
  server.on("/", handleRoot);
  server.on("/config", []{ iotWebConf.handleConfig(); });
  
  server.onNotFound([](){ iotWebConf.handleNotFound(); });
  Serial.println("Ready.");

  // uruchomienie serwera HTTP
  server.begin();
  Serial.println("HTTP server started");

  // uruchomienie serwera WebSocket
  webSocket.begin();
  webSocket.onEvent(onWsEvent);
  Serial.println("WebSocket server started");

 //  ustawienie pinów jako wejścia i włączenie wbudowanych rezystorów podciągających
 initInputExpander();
 initOutputExpander();

// INICJALIZACJA PCF 
Serial.print("Init input Expander...");
if (ExInput.begin( )) {
Serial.println("OK");
} else {
Serial.println("error");
}

initInputExpander();

Serial.print("Init output Expander...");
if (ExOutput.begin()) {
Serial.println("OK");
} else {
Serial.println("error");
}

blinkOutput(20);

otaStart();
initInputExpander();
//timers.attach(0, 1000, functionName);



 

}









void loop() {

unsigned long currentMillis = millis();  // Get the current time

delay(400);


  doc["pin_1"]="OFF";
  doc["pin_2"]="OFF";
  doc["pin_3"]="OFF";
  doc["pin_4"]="OFF";
  doc["pin_5"]="OFF";
  doc["pin_6"]="OFF";
  
  bool anyInputON = false; 

// Utworzenie JSON ze stanu 6 wejść na pierwszym ekspanderze
for (int i = 0; i < 6; i++) {
    // odczytanie stanu pinu i przypisanie do zmiennej curr_pin
    String curr_pin = String(ExInput.digitalRead(i));

    //  
    if (curr_pin == "1") {
        doc["pin_" + String(i)] = "ON";
        anyInputON = true;       

    } else {
        doc["pin_" + String(i)] = "OFF";
       
    }
}

  // Jeśli jakiś pin jest w stanie LOW - zwarty do GROUND - to włączamy LED i pompę
if(anyInputON){
                ExOutput.digitalWrite(P6, LOW);  // LED ON
                ExOutput.digitalWrite(P7, LOW);  // Pompa ON
                doc["piec_pompa"] = "ON";
                doc["led"] = "ON";
    
    for (int i = 0; i < 6; i++) {

        if(doc["pin_" + String(i)] =="ON"){
                ExOutput.digitalWrite(i, HIGH);
        }
        if(doc["pin_" + String(i)] =="OFF"){
                 ExOutput.digitalWrite(i, LOW);

                // int intervals[] = {5 * 1000, 7 * 1000, 3 * 1000, 4 * 1000, 2 * 1000, 6 * 1000};  // tablica z interwałami dla każdego pinu

                // for (int i = 0; i < 6; i++) {
                //   // timers.attach(i, intervals[i], std::bind(blinkOutputValve<i>, intervals[i]));
                // }


        }


    }

}else{
                // Serial.println("Wyłączam pompę i led");
                ExOutput.digitalWrite(P6, HIGH);  // LED OFF
                ExOutput.digitalWrite(P7, HIGH);  // Pompa OFF
                doc["piec_pompa"] = "OFF";
                doc["led"] = "OFF";

                for (int i = 0; i < 6; i++) {
                
                ExOutput.digitalWrite(i, HIGH);


                }

}

  
    String outputJSON;
    serializeJson(doc, outputJSON);
    //Serial.println(outputJSON);



  // sprawdzanie, czy upłynęło co najmniej 20 sekund od ostatniego wysłania
  if (millis() - lastSendTime > 9000) {
      // wysyłanie info do klientów
      webSocket.broadcastTXT(outputJSON);
      lastSendTime = millis();
  }

  
  // Sprawdzenie, czy upłynęło 20 minut od ostatniego restartu
  // if (currentMillis - previousMillis >= interval) {
  //   ESP.restart();  // Restart the NodeMCU
  //   previousMillis = currentMillis; 
  // }

      iotWebConf.doLoop();
      //timers.process();
      webSocket.loop();
      ArduinoOTA.handle();
}
