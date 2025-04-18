#include <Arduino.h>
// #include <ArduinoJson.h>
  // Inicjalizacja ESP-NOW
  if (esp_now_init() != ERR_OK)
  {
    Serial.println("Błąd inicjalizacji ESP-NOW");
    return;
  }
  esp_now_register_recv_cb(onDataRecv);


void onDataRecv(uint8_t *mac, uint8_t *data, uint8_t len) {
    // Funkcja wywoływana po odebraniu danych
    Serial.print("Received data: ");
    Serial.println((char*)data);

    docPins["Serial data "] = (char*)data;

    // // Możesz przetworzyć dane JSON i zaktualizować stan temperatury
    // StaticJsonDocument<200> sensorData;
    // DeserializationError error = deserializeJson(sensorData, (char*)data);
    // if (!error) {
    //     float temperature = sensorData["temperature"];
    //     Serial.println(temperature);  // Wyświetlanie odebranej temperatury
    //     doc["temperature"] = temperature;  // Możesz dodać temperaturę do JSON
    // } else {
    //     Serial.println("Error parsing sensor data");
    // }
}