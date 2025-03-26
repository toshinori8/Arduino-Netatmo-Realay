#include "manifoldLogic.h"
#include <WebSocketsServer.h>

void manifoldLogic()
{
  bool anyForcedON = false;
  bool anyInputON = false;
  unsigned long currentMillis = millis();

  delay(400);

  // Sprawdź czy którykolwiek przekaźnik jest wymuszony
  for (int i = 0; i < NUM_RELAYS; i++) {
    if (!relayStates[i].isAuto) {
      anyForcedON = true;
      break;
    }
  }

  // Sprawdź stan pieca
  bool woodStoveOn = (String(ExpInput.digitalRead(P6)) == "true");

  // Obsługa przekaźników
  if (anyForcedON) {
    // Jeśli są wymuszone przekaźniki, ustaw ich stany zgodnie z relayStates
    for (int i = 0; i < NUM_RELAYS; i++) {
      ExpOutput.digitalWrite(relayPins[i], relayStates[i].isOn ? LOW : HIGH);
    }
  } else {
    // Jeśli nie ma wymuszonych przekaźników, wyłącz wszystkie
    ExpOutput.digitalWrite(P7, HIGH); // Pompa OFF
    for (int i = 0; i < NUM_RELAYS; i++) {
      ExpOutput.digitalWrite(relayPins[i], HIGH); // Pinoutput OFF
    }
  }

  // Wysyłanie statusu co 9 sekund
  if (millis() - lastSendTime > 9000) {
    // Przygotuj jeden spójny JSON ze stanem
    StaticJsonDocument<2048> stateDoc;
    stateDoc["type"] = "state";
    stateDoc["timestamp"] = currentMillis;
    stateDoc["manifoldTemp"] = manifoldTemp;
    stateDoc["woodStove"] = woodStoveOn;
    
    // Dodaj stany przekaźników
    JsonArray relays = stateDoc.createNestedArray("relays");
    for (int i = 0; i < NUM_RELAYS; i++) {
      JsonObject relay = relays.createNestedObject();
      relay["id"] = i;
      relay["name"] = relayStates[i].name;
      relay["isOn"] = relayStates[i].isOn;
      relay["isAuto"] = relayStates[i].isAuto;
      relay["linkedSensor"] = relayStates[i].linkedSensor;
    }

    String outputJSON;
    serializeJson(stateDoc, outputJSON);
    webSocket.broadcastTXT(outputJSON);
    lastSendTime = currentMillis;

    addDebugLog("Wysłano stan: " + outputJSON);
  }
} 