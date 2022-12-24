#include <Arduino.h>

#include "PCF8574.h"
#include <Wire.h>
#include "Timers.h"
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char* ssid = "|oooooi|";
const char* password = "pmgana921";

ESP8266WebServer server(80); // utworzenie obiektu serwera HTTP na porcie 80



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


  StaticJsonDocument<10> doc;
  Serial.begin(9600);


WiFi.begin(ssid, password);
Serial.println("Connecting to WiFi");
while (WiFi.status() != WL_CONNECTED) {
delay(500);
Serial.print(".");
}
Serial.println();
Serial.print("Connected to WiFi, IP address: ");
Serial.println(WiFi.localIP());

// uruchomienie serwera HTTP
server.begin();
Serial.println("HTTP server started");


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

ExInput.digitalWrite(P0, HIGH);
ExInput.digitalWrite(P1, HIGH);
ExInput.digitalWrite(P2, HIGH);
ExInput.digitalWrite(P3, HIGH);
ExInput.digitalWrite(P4, HIGH);
ExInput.digitalWrite(P5, HIGH);

blinkOutput(20);

otaStart();

timers.attach(0, 1000, mainValveController);
}



void loop() {

 timers.process();
  
  bool anyInputON = false; 
  
   // sprawdzenie stanu 6 wejść na pierwszym ekspanderze
  for (int i = 0; i < 6; i++) {
    
    

    if (String(ExInput.digitalRead(i)) == "1") {
     

       // doc["pin" + String(i)] = String(i);

        anyInputON = true; // znaleziono zwarte wejście
        ExOutput.digitalWrite(i, LOW); // włączenie odpowiedniego pinu na drugim ekspanderze
        
        Serial.print("OGRZEWANIE POKÓJ :  ");
        Serial.print(i);

        ExOutput.digitalWrite(P6, LOW); // pin 7 (piec)(pompa)
        ExOutput.digitalWrite(P7, LOW); // pin 8 (LED INDICATOR)

    }else {
         // doc["pin" + String(i)] = String(i);
          ExOutput.digitalWrite(i, HIGH);   // wyłączenie reszty pinuw na drugim ekspanderze  
    }
    
  }

   //  nie znaleziono zwartych wejść, wyłącz wszystko na drugim ekspanderze
  if (!anyInputON) {

        for (int i = 0; i < 7; i++) {
          ExOutput.digitalWrite(i,  HIGH);
        }

       Serial.println("Wyłączam pompę i piec");
  }
  // String outputJSON;
  // serializeJson(doc, outputJSON);
  // Serial.println(outputJSON);



}