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
//timers.attach(0, 1000, mainValveController);



 

}



void loop() {

  doc["pin_1"]="OFF";
  doc["pin_2"]="OFF";
  doc["pin_3"]="OFF";
  doc["pin_4"]="OFF";
  doc["pin_5"]="OFF";
  doc["pin_6"]="OFF";
  bool anyInputON = false; 
  
  iotWebConf.doLoop();

  delay(800);
  //timers.process();
  
  // sprawdzenie stanu 6 wejść na pierwszym ekspanderze
  for (int i = 0; i < 6; i++) {
        String curr_pin = String(ExInput.digitalRead(i));
        delay(1000);
        if (curr_pin == "1") {
            doc["pin_" + String(i)] = "ON";
                doc["piec_pompa"] = "OFF";
                doc["led"] = "OFF";
            ExOutput.digitalWrite(i, HIGH);
            ExOutput.digitalWrite(P6, LOW);  //  LED ON
            ExOutput.digitalWrite(P7, LOW); // Pompa ON 
            anyInputON = true; // znaleziono zwarte wejście
            Serial.print("znaleziono zwarcie wejście "+  String(i));
        }else 
        }else{
           ExOutput.digitalWrite(i, LOW);
           doc["pin_" + String(i)] = "OFF";
        }else if (!anyInputON)
        {
        
                // Wyłączenie wszystkiego
                      ExOutput.digitalWrite(i, HIGH);
                      ExOutput.digitalWrite(P6, HIGH);  //  LED OFF
                      ExOutput.digitalWrite(P7, HIGH); // Pompa OFF
                doc["pin_" + String(i)] = "OFF";
                doc["piec_pompa"] = "OFF";
                doc["led"] = "OFF";
        }
        //Serial.println(curr_pin);
  };


    //  if(!anyInputON){

    //           String message = "{\"response\":\"brak zwarcia\"}";
    //               webSocket.broadcastTXT(message);

    //         for (int i = 0; i < 8; i++) {
    //             // Wyłączenie wszystkiego 
    //             ExOutput.digitalWrite(i, HIGH);
    //             doc["pin_" + String(i)] = "OFF";
    //             doc["piec_pompa"] = "OFF";
    //             doc["led"] = "OFF";
    //         }
    //     }




    // if(anyInputON == true){
          
    //       for (int i = 0; i < 6; i++) {

    //       String curr_pin2 = String(ExInput.digitalRead(i));

    //               ExOutput.digitalWrite(P6, LOW); // pin 7 (piec, pompa) 
    //               doc["piec_pompa"] = "ON";
    //               ExOutput.digitalWrite(P7, LOW); // pin 8 (LED INDICATOR)
    //               doc["led"] = "ON";

    //               if (curr_pin2 == "1") {
    //               ExOutput.digitalWrite(i, HIGH);
    //               doc["pin_" + String(i)] = "ON"; 
    //               }else{
    //               doc["pin_" + String(i)] = "OFF";
    //               ExOutput.digitalWrite(i, LOW);          
    //               }
        
    //     }
    // }
        
        
        
   
        

        //  ExOutput.digitalWrite(P6, LOW); // wylaczenie pin 7 (piec, pompa) 
        //  ExOutput.digitalWrite(P7, HIGH);   // wyłączenie pinu LED 
        //  
         

    

   
  
  
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
    ArduinoOTA.handle();
}
