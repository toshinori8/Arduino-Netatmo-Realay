#include <Arduino.h>
// --- EEPROM Save/Load Functions ---

// --- EEPROM Settings ---
#define MAX_ROOMS_EEPROM 6 // Max rooms to save in EEPROM (Adjusted to 6 based on user feedback)

// Define EEPROM Addresses
const int ADDR_MAGIC = 0;
const int ADDR_USE_GAZ = ADDR_MAGIC + sizeof(byte);
const int ADDR_MIN_OPERATING_TEMP = ADDR_USE_GAZ + sizeof(bool);
const int ADDR_ROOM_COUNT = ADDR_MIN_OPERATING_TEMP + sizeof(bool);
const int ADDR_ROOM_DATA_START = ADDR_ROOM_COUNT + sizeof(byte);

// Calculate size needed for one room's data
const int ROOM_DATA_SIZE = sizeof(int) + sizeof(int) + sizeof(bool) + sizeof(float); // id, pin, forced, temp (4+4+1+4 = 13 bytes)

// Calculate total EEPROM size needed
// const int EEPROM_SIZE = ADDR_ROOM_DATA_START + (MAX_ROOMS_EEPROM * ROOM_DATA_SIZE); // 3 + (6 * 13) = 81 bytes
const int EEPROM_SIZE = 62; // Use 62 bytes as requested by user

#define MAX_ROOMS_EEPROM 4 // Max rooms to save in EEPROM (Adjusted to 4 based on 62-byte limit)

const byte EEPROM_MAGIC_VALUE = 0xAB; // Choose a magic byte value


// Saves current settings (useGaz and room data) to EEPROM using individual writes, single commit
void saveSettings(RoomManager &mgr, bool currentUseGaz)
{
  Serial.println("Saving settings to EEPROM (individual fields)...");

  // 1. Write Magic Byte
  EEPROM.put(ADDR_MAGIC, EEPROM_MAGIC_VALUE);
  Serial.printf("  Putting Magic Byte: 0x%X at Addr: %d\n", EEPROM_MAGIC_VALUE, ADDR_MAGIC);
  if (!EEPROM.commit())
  {
    Serial.println("ERROR! EEPROM commit failed after magic byte");
  }

  // 2. Write useGaz flag
  EEPROM.put(ADDR_USE_GAZ, currentUseGaz);
  Serial.printf("  Putting useGaz: %s at Addr: %d\n", currentUseGaz ? "true" : "false", ADDR_USE_GAZ);
  if (!EEPROM.commit())
  {
    Serial.println("ERROR! EEPROM commit failed after useGaz");
  }

  // 3. Write Minimal Operating Temperature  minOperatingTemp
  EEPROM.put(ADDR_MIN_OPERATING_TEMP, minOperatingTemp);
     if (!EEPROM.commit())
  {
    Serial.println("ERROR! EEPROM commit failed after minOperatingTemp");
  }

  // 4. Write Room Count
  const std::vector<RoomData> &currentRooms = mgr.getAllRooms();
  byte roomCountToSave = min((byte)currentRooms.size(), (byte)MAX_ROOMS_EEPROM);
  EEPROM.put(ADDR_ROOM_COUNT, roomCountToSave);
  Serial.printf("  Putting roomCount: %d at Addr: %d\n", roomCountToSave, ADDR_ROOM_COUNT);
  if (!EEPROM.commit())
  {
    Serial.println("ERROR! EEPROM commit failed after room count");
  }
  Serial.printf("Saving %d rooms to EEPROM.\n", roomCountToSave);


  // 5. Write Room Data
  int currentAddr = ADDR_ROOM_DATA_START;
  for (byte i = 0; i < roomCountToSave; ++i)
  {
    int roomId = currentRooms[i].ID;
    int roomPin = currentRooms[i].pinNumber;
    bool roomForced = currentRooms[i].forced;
    float roomFireplaceTemp = currentRooms[i].targetTemperatureFireplace;

    Serial.printf("  Putting Room ID: %d, Pin: %d, Forced: %s, Fireplace Temp: %.1f at Addr: %d\n",
                  roomId, roomPin, roomForced ? "true" : "false", roomFireplaceTemp, currentAddr);

    EEPROM.put(currentAddr, roomId);
    Serial.printf("  Putting Room ID: %d at Addr: %d\n", roomId, currentAddr);
    currentAddr += sizeof(int);

    EEPROM.put(currentAddr, roomPin);
    Serial.printf("  Putting Room Pin: %d at Addr: %d\n", roomPin, currentAddr - sizeof(int));
    currentAddr += sizeof(int);

    EEPROM.put(currentAddr, roomForced);
    Serial.printf("  Putting Room Forced: %s at Addr: %d\n", roomForced ? "true" : "false", currentAddr - sizeof(int));
    currentAddr += sizeof(bool);

    EEPROM.put(currentAddr, roomFireplaceTemp);
    Serial.printf("  Putting Room FireplaceTemp: %.1f at Addr: %d\n", roomFireplaceTemp, currentAddr - sizeof(float));
    currentAddr += sizeof(float);
  }
  Serial.printf("Finished putting fields. Last write Addr: %d. Attempting commit...\n", currentAddr - sizeof(float)); // Debug last address used

  // 6. Commit all changes once at the end
  if (EEPROM.commit())
  {
    Serial.println("EEPROM successfully committed");
  }
  else
  {
    Serial.println("ERROR! EEPROM commit failed after writing all data.");
    // Note: currentAddr here is the address *after* the last write.
    // The actual failure point during commit isn't directly reported by the library.
  }
}

// Loads settings from EEPROM using individual reads and updates manager and useGaz_
// Returns true if successful (magic byte matched), false otherwise
bool loadSettings(RoomManager &mgr)
{
  Serial.println("Loading settings from EEPROM (individual fields)...");

  // 1. Read and Check Magic Byte
  byte magic = 0; // Initialize to prevent compiler warning
  EEPROM.get(ADDR_MAGIC, magic);
  if (magic != EEPROM_MAGIC_VALUE)
  {
    Serial.println("EEPROM magic byte mismatch! Using default settings.");
    return false; // Indicate failure to load valid settings
  }
  Serial.println("EEPROM magic byte matched. Applying settings.");

  // 2. Read useGaz flag
  EEPROM.get(ADDR_USE_GAZ, useGaz_);
  docPins["usegaz"] = useGaz_ ? "true" : "false"; // Update JSON doc
  Serial.printf("Loaded useGaz_ = %s\n", useGaz_ ? "true" : "false");

  // 3. Read Minimal Operating Temperature
  EEPROM.get(ADDR_MIN_OPERATING_TEMP, minOperatingTemp);
  Serial.printf("Loaded minOperatingTemp = %.1f\n", minOperatingTemp);

  // 4. Read Room Count
  byte roomCountToLoad = 0; // Initialize to prevent compiler warning
  EEPROM.get(ADDR_ROOM_COUNT, roomCountToLoad);
  Serial.printf("Loading %d rooms from EEPROM.\n", roomCountToLoad);

  // 5. Read Room Data
  int currentAddr = ADDR_ROOM_DATA_START;
  for (byte i = 0; i < roomCountToLoad; ++i)
  {
    int loadedId;
    int loadedPin;
    bool loadedForced;
    float loadedFireplaceTemp;

    EEPROM.get(currentAddr, loadedId);
    currentAddr += sizeof(int);
    EEPROM.get(currentAddr, loadedPin);
    currentAddr += sizeof(int);
    EEPROM.get(currentAddr, loadedForced);
    currentAddr += sizeof(bool);
    EEPROM.get(currentAddr, loadedFireplaceTemp);
    currentAddr += sizeof(float);

    Serial.printf("  Room ID: %d, Pin: %d, Forced: %s, Fireplace Temp: %.1f at Addr: %d\n",
                  loadedId, loadedPin, loadedForced ? "true" : "false", loadedFireplaceTemp, ADDR_ROOM_DATA_START + i * ROOM_DATA_SIZE);
    // Update pin mapping in the manager FIRST
    mgr.idToPinMap[loadedId] = loadedPin;

    // Now try to update the room data if it exists (it might not exist yet if fetch hasn't run)
    bool roomFound = false;
    // Get a mutable reference to the rooms vector
    std::vector<RoomData> &mgrRooms = const_cast<std::vector<RoomData> &>(mgr.getAllRooms());
    for (auto &room : mgrRooms)
    {
      if (room.ID == loadedId)
      {
        room.pinNumber = loadedPin; // Update pin number based on loaded data
        room.forced = loadedForced;
        room.targetTemperatureFireplace = loadedFireplaceTemp;
        Serial.printf("  Applied settings to existing room %d\n", loadedId);
        roomFound = true;
        break;
      }
    }
    if (!roomFound)
    {
      // If the room doesn't exist in the manager's list yet,
      // we still updated the pin map. The fetchJsonData function, when it runs,
      // should create the room. The updateOrAddRoom logic needs to be checked
      // to ensure it preserves the loaded fireplace temp and forced status
      // if the fetched data doesn't override them.
      // Let's create a placeholder room now to ensure the data is stored.
      // We'll use default values for fields not stored in EEPROM.
      Serial.printf("  Room %d not found in manager, creating placeholder with loaded settings.\n", loadedId);
      RoomData placeholderRoom;
      placeholderRoom.ID = loadedId;
      placeholderRoom.pinNumber = loadedPin;
      placeholderRoom.forced = loadedForced;
      placeholderRoom.targetTemperatureFireplace = loadedFireplaceTemp;
      // Set other fields to defaults or leave them as initialized by RoomData()
      placeholderRoom.name = std::string("Loaded Room ") + std::to_string(loadedId); // Placeholder name
      mgr.addRoom(placeholderRoom);                                                  // Add the placeholder
    }
  }

  return true; // Indicate successful load
}
// --- End EEPROM Save/Load Functions ---