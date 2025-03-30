#include "manifoldLogic.h"
#include <WebSocketsServer.h>

// Dodaj zmienne statyczne dla stabilności
static unsigned long lastSendTime = 0;
static const unsigned long SEND_INTERVAL = 9000; // Co 9 sekund
static const unsigned long MAX_LOOP_TIME = 500;  // Maksymalny dozwolony czas pętli

void manifoldLogic()
{
  unsigned long startTime = millis();
  unsigned long currentMillis = millis();

  // Zabezpieczenie przed przepełnieniem millis()
  if (currentMillis < lastSendTime) {
    lastSendTime = 0;
  }

  // Sprawdź stan pieca bezpieczniej
  bool woodStoveOn = false;
  
  // Bezpieczne odczytanie stanu pieca
  if (ExpInput.digitalRead(P6) == LOW) {
    woodStoveOn = true;
  }

  // Wysyłanie statusu z interwałem i zabezpieczeniami
  if (currentMillis - lastSendTime >= SEND_INTERVAL) {
    // Ograniczenie rozmiaru dokumentu JSON
    StaticJsonDocument<1024> stateDoc;
    
    stateDoc.clear(); // Wyczyść poprzedni dokument
    stateDoc["type"] = "state";
    stateDoc["timestamp"] = currentMillis;
    stateDoc["manifoldTemp"] = manifoldTemp;
    stateDoc["woodStove"] = woodStoveOn;
    
    // Dodaj stany przekaźników z limitem
    JsonArray relays = stateDoc.createNestedArray("relays");
    for (int i = 0; i < min(NUM_RELAYS, 8); i++) {  // Limit 8 przekaźników
      JsonObject relay = relays.createNestedObject();
      relay["id"] = i;
      relay["name"] = relayStates[i].name;
      relay["isOn"] = relayStates[i].isOn;
      relay["isAuto"] = relayStates[i].isAuto;
    }

    // Bezpieczna serializacja z limitem czasu
    String outputJSON;
    serializeJson(stateDoc, outputJSON);
    
    // Wyślij tylko jeśli długość JSON jest rozsądna
    if (outputJSON.length() < 2048) {
      webSocket.broadcastTXT(outputJSON);
      
      // Log tylko krótkie komunikaty
    }

    lastSendTime = currentMillis;
  }

  // Zabezpieczenie przed długim wykonaniem
  unsigned long executionTime = millis() - startTime;
  if (executionTime > MAX_LOOP_TIME) {
    addDebugLog("Ostrzeżenie: długi czas wykonania manifoldLogic: " + String(executionTime) + "ms");
  }
}