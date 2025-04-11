// --- EEPROM Settings ---
#define MAX_ROOMS_EEPROM 6 // Matches 62-byte limit
const byte EEPROM_MAGIC_VALUE = 0xAB;

// Define EEPROM Addresses
const int ADDR_MAGIC = 0;  // 1 byte (0)
const int ADDR_USE_GAZ = ADDR_MAGIC + sizeof(byte);  // 1 byte (1)
const int ADDR_MIN_OPERATING_TEMP = ADDR_USE_GAZ + sizeof(bool);  // 4 bytes (2)
const int ADDR_ROOM_COUNT = ADDR_MIN_OPERATING_TEMP + sizeof(float);  // 1 byte (6)
const int ADDR_ROOM_DATA_START = ADDR_ROOM_COUNT + sizeof(byte);  // (7)
const int ROOM_DATA_SIZE = sizeof(int) + sizeof(int) + sizeof(bool) + sizeof(float); // 13 bytes
const int EEPROM_SIZE = ADDR_ROOM_DATA_START + (MAX_ROOMS_EEPROM * ROOM_DATA_SIZE); // 7 + (4*13) = 59 bytes

// Saves current settings to EEPROM
void saveSettings(RoomManager &mgr, bool currentUseGaz, float manifoldMinTemp) {
  Serial.println("Saving settings to EEPROM...");
  
  EEPROM.begin(EEPROM_SIZE);
  
  // Write all data first
  EEPROM.put(ADDR_MAGIC, EEPROM_MAGIC_VALUE);
  EEPROM.put(ADDR_USE_GAZ, currentUseGaz);
  EEPROM.put(ADDR_MIN_OPERATING_TEMP, manifoldMinTemp);
  
  const std::vector<RoomData> &currentRooms = mgr.getAllRooms();
  byte roomCountToSave = min((byte)currentRooms.size(), (byte)MAX_ROOMS_EEPROM);
  EEPROM.put(ADDR_ROOM_COUNT, roomCountToSave);
  
  int currentAddr = ADDR_ROOM_DATA_START;
  for (byte i = 0; i < roomCountToSave; ++i) {
    const RoomData &room = currentRooms[i];
    EEPROM.put(currentAddr, room.ID);
    currentAddr += sizeof(int);
    EEPROM.put(currentAddr, room.pinNumber);
    currentAddr += sizeof(int);
    EEPROM.put(currentAddr, room.forced);
    currentAddr += sizeof(bool);
    EEPROM.put(currentAddr, room.targetTemperatureFireplace);
    currentAddr += sizeof(float);
  }
  
  // Single commit at the end
  if (!EEPROM.commit()) {
    Serial.println("EEPROM commit failed!");
  } else {
    Serial.println("Settings saved successfully");
  }
  EEPROM.end();
}

bool loadSettings(RoomManager &mgr, bool &outUseGaz, float &outManifoldTemp) {
  Serial.println("Loading settings from EEPROM...");
  
  EEPROM.begin(EEPROM_SIZE);
  
  byte magic;
  EEPROM.get(ADDR_MAGIC, magic);
  if (magic != EEPROM_MAGIC_VALUE) {
    Serial.println("No valid settings found in EEPROM");
    EEPROM.end();
    return false;
  }
  
  EEPROM.get(ADDR_USE_GAZ, outUseGaz);
  EEPROM.get(ADDR_MIN_OPERATING_TEMP, outManifoldTemp);
  
  byte roomCount;
  EEPROM.get(ADDR_ROOM_COUNT, roomCount);
  
  int currentAddr = ADDR_ROOM_DATA_START;
  for (byte i = 0; i < roomCount; ++i) {
    int id, pin;
    bool forced;
    float temp;
    
    EEPROM.get(currentAddr, id);
    currentAddr += sizeof(int);
    EEPROM.get(currentAddr, pin);
    currentAddr += sizeof(int);
    EEPROM.get(currentAddr, forced);
    currentAddr += sizeof(bool);
    EEPROM.get(currentAddr, temp);
    currentAddr += sizeof(float);
    
    // Update or add room
    RoomData room;
    room.ID = id;
    room.pinNumber = pin;
    room.forced = forced;
    room.targetTemperatureFireplace = temp;
    mgr.updateOrAddRoom(room);
  }
  
  EEPROM.end();
  return true;
}