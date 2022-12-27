#include <Arduino.h>

#include "PCF8574.h"
#include <Wire.h>
#include "Timers.h"
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <webPage.h>
#include <WebSocketsServer.h>
unsigned long lastSendTime = 0;
WebSocketsServer webSocket(8080);

void onWsEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
 
    case WStype_DISCONNECTED:
      // obsługa rozłączenia klienta
      Serial.println("Client disconnected from WebSocket ");
      break;
    // case WStype_TEXT: 
    //   // obsługa wiadomości tekstowej od klienta
    //   String messageText = String((char*)payload).substring(0, length);
    //   //Serial.println("Otrzymano wiadomość tekstową od klienta: " + messageText);
    //   break; // dodaj instrukcję break; tutaj
    case WStype_CONNECTED:
      // obsługa połączenia klienta
      String message = "{\"response\":\"connected\"}";
      webSocket.sendTXT(num, message);
      break;
  }
}



StaticJsonDocument<200> doc;
 


#include <IotWebConf.h>
const char thingName[] = "Netatmo_Relay";
const char wifiInitialApPassword[] = "pmgana921";
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



//ESP8266WebServer server(80); // utworzenie obiektu serwera HTTP na porcie 80



// utworzenie obiektu klasy Timers z trzema odliczającymi
Timers<3> timers;
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


void mainValveController(){


  
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
//timers.attach(0, 1000, mainValveController);



 

}



void loop() {

   

  doc["pin_1"]="OFF";
  doc["pin_2"]="OFF";
  doc["pin_3"]="OFF";
  doc["pin_4"]="OFF";
  doc["pin_5"]="OFF";
  doc["pin_6"]="OFF";


  delay(600);
  iotWebConf.doLoop();

  //timers.process();
  
  bool anyInputON = false; 
  
   // sprawdzenie stanu 6 wejść na pierwszym ekspanderze
  for (int i = 0; i < 6; i++) {
    
    if (String(ExInput.digitalRead(i)) == "1") {
     
        doc["pin_" + String(i)] = "ON";
        anyInputON = true; // znaleziono zwarte wejście
        ExOutput.digitalWrite(i, LOW); // włączenie odpowiedniego pinu na drugim ekspanderze
        
       // Serial.print("OGRZEWANIE POKÓJ :  ");
       // Serial.print(i);

        ExOutput.digitalWrite(P6, LOW); // pin 7 (piec)(pompa) 
        doc["piec_pompa"] = "ON";
        ExOutput.digitalWrite(P7, LOW); // pin 8 (LED INDICATOR)
        doc["led"] = "ON";
    }else {
          ExOutput.digitalWrite(i, HIGH);  // wyłączenie reszty pinow na drugim ekspanderze 
          ExOutput.digitalWrite(P7, HIGH);   // wyłączenie pinu LED 
          doc["pin_" + String(i)] = "OFF";
          doc["piec_pompa"] = "OFF";
          doc["led"] = "OFF";

    }
    
  }

   //  nie znaleziono zwartych wejść, wyłącz wszystko na drugim ekspanderze
  if (!anyInputON) {

        for (int i = 0; i < 8; i++) {
          ExOutput.digitalWrite(i,  HIGH);
        }
  }
   String outputJSON;
  serializeJson(doc, outputJSON);
 //Serial.println(outputJSON);



  // sprawdzanie, czy upłynęło co najmniej 20 sekund od ostatniego wysłania
  if (millis() - lastSendTime > 9000) {
    // wysyłanie ciągu znaków do wszystkich połączonych klientów
    webSocket.broadcastTXT(outputJSON);
    Serial.println(outputJSON);
    // zapamiętanie czasu ostatniego wysłania
    lastSendTime = millis();
  }


    webSocket.loop();
}


