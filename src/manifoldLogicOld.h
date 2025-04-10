#include <Arduino.h>
void manifoldLogicNew()
{

  // --- Room Heating Logic ---
  const std::vector<RoomData> &rooms = manager.getAllRooms();
  std::vector<int> forcedRoomsNeedingHeatIDs; // Store IDs of forced rooms needing heat
  int primaryRoomId = -1;
  float lowestTemp = 100.0;
  int secondaryRoomId = -1;
  float smallestPositiveDifference = 100.0;

  // --- First Pass: Identify forced rooms needing heat and find the primary candidate ---
  for (const auto &room : rooms)
  {
    if (!room.forced)
      continue; // Skip non-forced rooms

    // Determine effective target temperature
    float effectiveTargetTemperature;
    if (useGaz_)
    {
      effectiveTargetTemperature = max(room.targetTemperatureNetatmo, room.targetTemperatureFireplace);
    }
    else
    {
      effectiveTargetTemperature = room.targetTemperatureFireplace;
    }

    // Check if room needs heating
    if (room.currentTemperature < effectiveTargetTemperature)
    {
      forcedRoomsNeedingHeatIDs.push_back(room.ID); // Add to list

      // Check if this room has the lowest temperature so far
      if (room.currentTemperature < lowestTemp)
      {
        lowestTemp = room.currentTemperature;
        primaryRoomId = room.ID;
      }
    }
  }

  // --- Second Pass (if primary found): Find the secondary candidate ---
  if (primaryRoomId != -1)
  {
    for (int roomId : forcedRoomsNeedingHeatIDs)
    {
      if (roomId == primaryRoomId)
        continue; // Skip the primary room

      // Find the room data again (could optimize by storing pointers/references)
      for (const auto &room : rooms)
      {
        if (room.ID == roomId)
        {
          // Determine effective target temperature again (could optimize)
          float effectiveTargetTemperature;
          if (useGaz_)
          {
            effectiveTargetTemperature = max(room.targetTemperatureNetatmo, room.targetTemperatureFireplace);
          }
          else
          {
            effectiveTargetTemperature = room.targetTemperatureFireplace;
          }

          float difference = effectiveTargetTemperature - room.currentTemperature;
          if (difference > 0 && difference < smallestPositiveDifference)
          { // Must need heat and be smallest diff
            smallestPositiveDifference = difference;
            secondaryRoomId = roomId;
          }
          break; // Found the room data
        }
      }
    }
  }

  // --- Control Relays ---
  Serial.println("--- Heating Logic ---");
  // Turn OFF all room relays initially
  for (int i = 0; i < 6; i++)
  {
    ExpOutput.digitalWrite(i, HIGH); // LOW = OFF
  }

  // przerwij funkcje jesli nie ma temepratury  lub jest za niska
  if (isnan(manifoldTemp) || manifoldTemp < manifoldMinTemp)
  {
    Serial.println("no temperature sensor data from manifold or temperature too low");
    return;
  }
  else
  {

    // Activate primary room relay
    if (primaryRoomId != -1)
    {
      for (const auto &room : rooms)
      {
        if (room.ID == primaryRoomId)
        {
          if (room.pinNumber >= 0 && room.pinNumber < 6)
          {
            relayMode(LOW);
            ExpOutput.digitalWrite(room.pinNumber, HIGH); // HIGH = ON

            Serial.printf("Primary heating ON: Room %s (Pin %d, Temp %.1f, Lowest Temp)\n",
                          room.name.c_str(), room.pinNumber, room.currentTemperature);
          }
          break;
        }
      }
    }
    else
    {
      Serial.println("No primary forced room needs heating.");
    }

    // Activate secondary room relay
    if (secondaryRoomId != -1)
    {
      for (const auto &room : rooms)
      {
        if (room.ID == secondaryRoomId)
        {
          if (room.pinNumber >= 0 && room.pinNumber < 6)
          {
            // Check if it's the same pin as primary - avoid double logging if so
            if (primaryRoomId == -1 || room.pinNumber != manager.getRoomByID(primaryRoomId).pinNumber)
            {
              //  relayMode(LOW);
              ExpOutput.digitalWrite(room.pinNumber, HIGH); // HIGH = OFF
                                                            //  docPins["pins"]["pin_" + String(room.pinNumber)]["state"] = "ON";
              Serial.printf("Secondary heating ON: Room %s (Pin %d, Temp %.1f, Smallest Diff %.1f)\n",
                            room.name.c_str(), room.pinNumber, room.currentTemperature, smallestPositiveDifference);

              // Create a mutable copy of the room data
              RoomData updatedRoom = room;
              // Update valve status
              updatedRoom.valve = true;

              // Update or add the room with the modified copy
              manager.updateOrAddRoom(updatedRoom);
            }
            else
            {
              Serial.printf("Secondary room (%s) shares pin with primary. Already ON.\n", room.name.c_str());
            }
          }
          break;
        }
      }
    }
    else if (primaryRoomId != -1)
    {
      Serial.println("No suitable secondary forced room found.");
      relayMode(HIGH);
    }

    // --- Gas/Pump Control ---

    // if there is forced room, is primary room
    if (primaryRoomId != -1 && useGaz_ == true)
    {

      ExpOutput.digitalWrite(P6, LOW);
      ExpOutput.digitalWrite(P7, LOW);

      Serial.println("Gas mode ON - P6/P7 ON");
    }
    // ONLY KOMINEK
    else if (primaryRoomId != -1 && useGaz_ == false)
    {
      ExpOutput.digitalWrite(P6, HIGH);
      ExpOutput.digitalWrite(P7, LOW);

      Serial.println("Gas mode OFF - P6 OFF / P7 ON ");
    }
    // else if no primary room, turn off gas/pump
    else if (primaryRoomId == -1 && useGaz_ == false)
    {
      ExpOutput.digitalWrite(P6, HIGH);
      ExpOutput.digitalWrite(P7, HIGH);

      Serial.println("Gas mode OFF NO HEATING - P6/P7 OFF");
    }

    // --- Update docPins forced status and final room info ---
    for (const auto &room : rooms)
    {
      if (room.pinNumber >= 0 && room.pinNumber < 6)
      {
        docPins["pins"]["pin_" + String(room.pinNumber)]["forced"] = room.forced ? "true" : "false";
      }
    }
    docPins["roomsInfo"] = manager.getRoomsAsJson(); // Send updated state including valve status

    Serial.println("--- End Heating Logic ---");
  }
  if (manifoldTemp <= manifoldMinTemp)
  {
    Serial.println("--- End Heating Logic --- Manifold too cold");
    relayMode(HIGH);
  }
}   // end function