#include <Wire.h>

int aht_read_errors = 0;
const int MAX_AHT_ERRORS_BEFORE_RESET = 5;


class AHT10 {
private:
    uint8_t address;
    bool initialized = false;
    
    static const uint8_t AHT10_CALIBRATE_CMD = 0xE1;
    static const uint8_t AHT10_SOFT_RESET_CMD = 0xBA;

    static const uint8_t AHT10_MEASURE_CMD[3];
    static const uint8_t AHT10_DEFAULT_DELAY = 75;    // ms
    static const uint8_t AHT10_HUMIDITY_DELAY = 75;  // ms
    static const uint8_t AHT10_ATTEMPTS = 3;
    
    bool writeCommand(const uint8_t* cmd, uint8_t len);
    bool readData(uint8_t* data, uint8_t len);
    
public:
    AHT10(uint8_t addr = 0x38) : address(addr) {}
    
    bool begin();
    bool measure(float* temperature, float* humidity);
    bool softReset() {
        Serial.println("Sending AHT10 Soft Reset command (0xBA)...");
        uint8_t cmd = AHT10_SOFT_RESET_CMD;
        if (!writeCommand(&cmd, 1)) {
            Serial.println("AHT10 Soft Reset command failed");
            return false;
        }
        delay(20); // Krótki delay po resecie (wg datasheet AHT20, może być pomocny)
        return true;
    }
};

const uint8_t AHT10::AHT10_MEASURE_CMD[3] = {0xAC, 0x33, 0x00};

bool AHT10::begin() {
    Wire.begin();
    
     if (!softReset()) {
         // Możesz zdecydować, czy kontynuować mimo błędu resetu
         Serial.println("Soft Reset failed, attempting to continue initialization...");
    }

    // Send calibration command
    uint8_t cmd = AHT10_CALIBRATE_CMD;
    if (!writeCommand(&cmd, 1)) {
        Serial.println("AHT10 calibration command failed");
        return false;
    }
    
    delay(AHT10_DEFAULT_DELAY);
    
    uint8_t status = 0;
    if (!readData(&status, 1)) {
        Serial.println("AHT10 status read failed");
        return false;
    }
    
    if ((status & 0x68) != 0x08) {  // Check if calibrated
        Serial.println("AHT10 calibration failed");
        return false;
    }
    
    initialized = true;
    return true;
}

bool AHT10::measure(float* temperature, float* humidity) {
    if (!initialized) return false;
    
    if (!writeCommand(AHT10_MEASURE_CMD, sizeof(AHT10_MEASURE_CMD))) {
        Serial.println("AHT10 measurement command failed");
        return false;
    }
    
    uint8_t data[6];
    bool success = false;
    uint8_t delay_ms = (humidity != nullptr) ? AHT10_HUMIDITY_DELAY : AHT10_DEFAULT_DELAY;
    
    for (int i = 0; i < AHT10_ATTEMPTS; i++) {
        delay(delay_ms);
        
        if (!readData(data, 6)) {
            continue;
        }
        
        if ((data[0] & 0x80) == 0x80) {  // Check if busy
            continue;
        }
        
        if (humidity != nullptr && data[1] == 0 && data[2] == 0 && (data[3] >> 4) == 0) {
            // Unrealistic humidity value
            continue;
        }
        
        success = true;
        break;
    }
    
    if (!success) {
        Serial.println("AHT10 measurement timed out");
        return false;
    }
    
    uint32_t raw_temperature = ((data[3] & 0x0F) << 16) | (data[4] << 8) | data[5];
    uint32_t raw_humidity = ((data[1] << 16) | (data[2] << 8) | data[3]) >> 4;
    
    *temperature = ((200.0 * raw_temperature) / 1048576.0) - 50.0;
    
    if (humidity != nullptr) {
        if (raw_humidity == 0) {
            *humidity = NAN;
        } else {
            *humidity = raw_humidity * 100.0 / 1048576.0;
        }
    }
    
    return true;
}

bool AHT10::writeCommand(const uint8_t* cmd, uint8_t len) {
    Wire.beginTransmission(address);
    Wire.write(cmd, len);
    return Wire.endTransmission() == 0;
}

bool AHT10::readData(uint8_t* data, uint8_t len) {
    Wire.requestFrom(address, len);
    if (Wire.available() < len) return false;
    
    for (uint8_t i = 0; i < len; i++) {
        data[i] = Wire.read();
    }
    
    return true;
}


// Example usage:
// AHT10 sensor;    AHT10 sensor(0x38);    // 0x38 default address
 
// void setup() {
//     Serial.begin(9600);
//     if (!sensor.begin()) {
//         Serial.println("Failed to initialize AHT10");
//         while(1);
//     }
// }

// void loop() {
//     float temp, hum;
//     if (sensor.measure(&temp, &hum)) {
//         Serial.print("Temperature: ");
//         Serial.print(temp);
//         Serial.print(" °C, Humidity: ");
//         Serial.print(hum);
//         Serial.println(" %");
//     } else {
//         Serial.println("Measurement failed");
//     }
    
//     delay(2000);
// }