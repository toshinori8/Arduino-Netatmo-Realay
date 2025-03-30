#ifndef MANIFOLD_LOGIC_H
#define MANIFOLD_LOGIC_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "PCF8574.h"
#include <WebSocketsServer.h>

// Definicja struktury RelayState
struct RelayState {
  bool isOn;
  bool isAuto;
  String name;
  String linkedSensor;
  bool isForcedByThermostat;
};

// Deklaracje zewnętrznych zmiennych
extern float manifoldTemp;
extern PCF8574 ExpInput;
extern PCF8574 ExpOutput;
extern unsigned long lastSendTime;
extern WebSocketsServer webSocket;
extern RelayState relayStates[];
extern const int NUM_RELAYS;
extern const int relayPins[];
extern StaticJsonDocument<200> docPins;
extern void addDebugLog(const String& message);

// Deklaracja funkcji
void manifoldLogic();

#endif // MANIFOLD_LOGIC_H 