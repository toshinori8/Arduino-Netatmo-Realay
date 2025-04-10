#include <Arduino.h>


// w manifoldlogic chcialbym dodac kilka rzeczy. po pierwsze mozliwosc taką że gdy temp.temperature przekroczy np. 60 stopni to otwierany jest kolejny dodatkowy pokój. musi tez byc zapisywany parametr valve do managera pokojow (czy jest aktualnie otwarty)


// Gemini
// Okay, rozumiem. Chcesz dodać dwie funkcjonalności do manifoldLogicNew:

// Otwieranie dodatkowego zaworu przy wysokiej temperaturze kolektora: Gdy temp.temperature przekroczy określony próg (np. 60°C), system powinien otworzyć zawór dodatkowego pokoju (innego niż główny i wtórny), aby rozproszyć nadmiar ciepła.
// Zapisywanie stanu valve w RoomManager: Stan valve (czy zawór danego pokoju jest aktualnie otwarty przez logikę sterowania) powinien być poprawnie aktualizowany w obiekcie RoomData wewnątrz RoomManager.
// Zmodyfikujemy manifoldLogicNew i dodamy małą funkcję pomocniczą do RoomManager.

// Krok 1: Dodaj metodę do RoomManager (w roomManager.h)

// Potrzebujemy metody, która pozwoli zaktualizować stan valve dla konkretnego pokoju. Dodaj tę publiczną metodę do klasy RoomManager:


// W pliku roomManager.h, wewnątrz definicji klasy RoomManager
// NOWE: Definicja progu wysokiej temperatury kolektora
float HIGH_MANIFOLD_TEMP_THRESHOLD = 60.0; // Próg w stopniach Celsjusza

void manifoldLogicNew()
{
    Serial.println("--- Start Heating Logic ---");

    // przerwij funkcje jesli nie ma temepratury lub jest za niska
    if (isnan(temp.temperature)) {
         Serial.println("Manifold temperature sensor data is invalid (NaN). Aborting logic.");
         // Upewnij się, że wszystko jest wyłączone
         relayMode(HIGH); // Wyłącz wszystkie zawory pokojów
         ExpOutput.digitalWrite(P6, HIGH); // Wyłącz gaz
         ExpOutput.digitalWrite(P7, HIGH); // Wyłącz pompę kominka
         Serial.println("--- End Heating Logic (Invalid Temp) ---");
         return;
    }

    if (temp.temperature < minOperatingTemp) {
        Serial.printf("Manifold temperature (%.1f C) is below minimum operating temp (%.1f C). Aborting logic.\n", temp.temperature, minOperatingTemp);
        // Upewnij się, że wszystko jest wyłączone
        relayMode(HIGH); // Wyłącz wszystkie zawory pokojów
        ExpOutput.digitalWrite(P6, HIGH); // Wyłącz gaz
        ExpOutput.digitalWrite(P7, HIGH); // Wyłącz pompę kominka
        Serial.println("--- End Heating Logic (Temp Too Low) ---");
        return;
    }

    // --- Logika ogrzewania pokoi ---
    const std::vector<RoomData> &rooms = manager.getAllRooms();
    std::vector<int> forcedRoomsNeedingHeatIDs; // ID wymuszonych pokoi potrzebujących ciepła
    int primaryRoomId = -1;                     // ID pokoju głównego (najniższa temp)
    float lowestTemp = 100.0;
    int secondaryRoomId = -1;                   // ID pokoju wtórnego (najmniejsza różnica dodatnia)
    float smallestPositiveDifference = 100.0;
    int highTempRoomId = -1;                    // NOWE: ID pokoju do otwarcia przy wysokiej temp. kolektora
    float highestTempForHighTempRoom = -100.0;  // NOWE: Najwyższa temp. dla pokoju "nadmiarowego"

    // NOWE: Zbiór ID pokoi, których zawory zostaną aktywowane
    std::set<int> activeRoomIDs;

    // --- Pierwszy przebieg: Identyfikacja wymuszonych pokoi potrzebujących ciepła i kandydata głównego ---
    Serial.println("Pass 1: Identifying forced rooms needing heat...");
    for (const auto &room : rooms)
    {
        if (!room.forced || !room.reachable) // Pomiń niewymuszone lub nieosiągalne
            continue;

        // Określ efektywną temperaturę docelową
        float effectiveTargetTemperature;
        if (useGaz_) {
            effectiveTargetTemperature = max(room.targetTemperatureNetatmo, room.targetTemperatureFireplace);
        } else {
            effectiveTargetTemperature = room.targetTemperatureFireplace;
        }

        // Sprawdź, czy pokój potrzebuje ogrzewania
        if (room.currentTemperature < effectiveTargetTemperature) {
            forcedRoomsNeedingHeatIDs.push_back(room.ID); // Dodaj do listy
            Serial.printf("  Room %s (ID %d, Temp %.1f, Target %.1f) needs heat.\n",
                          room.name.c_str(), room.ID, room.currentTemperature, effectiveTargetTemperature);

            // Sprawdź, czy ten pokój ma najniższą temperaturę do tej pory
            if (room.currentTemperature < lowestTemp) {
                lowestTemp = room.currentTemperature;
                primaryRoomId = room.ID;
            }
        } else {
             Serial.printf("  Room %s (ID %d, Temp %.1f, Target %.1f) does NOT need heat.\n",
                          room.name.c_str(), room.ID, room.currentTemperature, effectiveTargetTemperature);
        }
    }

    if (primaryRoomId != -1) {
        Serial.printf("Primary candidate: Room ID %d (Lowest Temp: %.1f C)\n", primaryRoomId, lowestTemp);
    } else {
        Serial.println("No primary candidate found (no forced rooms need heat).");
    }


    // --- Drugi przebieg (jeśli znaleziono głównego): Znajdź kandydata wtórnego ---
    if (primaryRoomId != -1) {
        Serial.println("Pass 2: Identifying secondary candidate...");
        for (int roomId : forcedRoomsNeedingHeatIDs) {
            if (roomId == primaryRoomId) continue; // Pomiń pokój główny

            // Znajdź dane pokoju ponownie (można zoptymalizować)
            for (const auto &room : rooms) {
                if (room.ID == roomId) {
                    // Określ efektywną temperaturę docelową ponownie (można zoptymalizować)
                    float effectiveTargetTemperature;
                    if (useGaz_) {
                        effectiveTargetTemperature = max(room.targetTemperatureNetatmo, room.targetTemperatureFireplace);
                    } else {
                        effectiveTargetTemperature = room.targetTemperatureFireplace;
                    }

                    float difference = effectiveTargetTemperature - room.currentTemperature;
                    if (difference > 0 && difference < smallestPositiveDifference) { // Musi potrzebować ciepła i mieć najmniejszą różnicę
                        smallestPositiveDifference = difference;
                        secondaryRoomId = roomId;
                    }
                    break; // Znaleziono dane pokoju
                }
            }
        }
        if (secondaryRoomId != -1) {
             Serial.printf("Secondary candidate: Room ID %d (Smallest Diff: %.1f C)\n", secondaryRoomId, smallestPositiveDifference);
        } else {
             Serial.println("No suitable secondary candidate found.");
        }
    }

    // --- NOWE: Trzeci przebieg (jeśli temp. kolektora wysoka): Znajdź dodatkowy pokój do otwarcia ---
    if (temp.temperature > HIGH_MANIFOLD_TEMP_THRESHOLD) {
        Serial.printf("Pass 3: High manifold temp (%.1f C > %.1f C). Identifying extra room...\n",
                      temp.temperature, HIGH_MANIFOLD_TEMP_THRESHOLD);
        for (const auto &room : rooms) {
            // Pomiń pokój główny i wtórny, jeśli zostały wybrane
            if (room.ID == primaryRoomId || room.ID == secondaryRoomId || !room.reachable) {
                continue;
            }

            // Wybierz pokój z najwyższą temperaturą (strategia "zrzutu ciepła")
            // Można tu zastosować inną logikę, np. pokój najdłużej niegrzany
            if (room.currentTemperature > highestTempForHighTempRoom) {
                highestTempForHighTempRoom = room.currentTemperature;
                highTempRoomId = room.ID;
            }
        }
        if (highTempRoomId != -1) {
             Serial.printf("High temp candidate: Room ID %d (Highest Temp: %.1f C)\n", highTempRoomId, highestTempForHighTempRoom);
        } else {
             Serial.println("No suitable extra room found for high temp dissipation.");
        }
    }


    // --- Sterowanie przekaźnikami ---
    Serial.println("Controlling Relays...");
    // Wyłącz WSZYSTKIE przekaźniki pokojów na początku
    relayMode(HIGH); // HIGH = OFF dla wszystkich pinów 0-5

    // Aktywuj przekaźnik pokoju głównego
    if (primaryRoomId != -1) {
        for (const auto &room : rooms) {
            if (room.ID == primaryRoomId) {
                if (room.pinNumber >= 0 && room.pinNumber < 6) {
                    ExpOutput.digitalWrite(room.pinNumber, LOW); // LOW = ON
                    activeRoomIDs.insert(room.ID); // NOWE: Dodaj do aktywnych
                    Serial.printf("  Primary heating ON: Room %s (Pin %d)\n",
                                  room.name.c_str(), room.pinNumber);
                } else {
                    Serial.printf("  Warning: Invalid pin number (%d) for primary room %s (ID %d)\n",
                                  room.pinNumber, room.name.c_str(), room.ID);
                }
                break;
            }
        }
    }

    // Aktywuj przekaźnik pokoju wtórnego
    if (secondaryRoomId != -1) {
        for (const auto &room : rooms) {
            if (room.ID == secondaryRoomId) {
                if (room.pinNumber >= 0 && room.pinNumber < 6) {
                    // Sprawdź, czy to nie ten sam pin co główny (jeśli główny istnieje)
                    bool alreadyActive = activeRoomIDs.count(room.ID); // Sprawdź czy ID już jest (np. jeśli pin jest ten sam co primary)

                    ExpOutput.digitalWrite(room.pinNumber, LOW); // LOW = ON (nawet jeśli już był, dla pewności)
                    if (!alreadyActive) {
                         activeRoomIDs.insert(room.ID); // NOWE: Dodaj do aktywnych, jeśli jeszcze nie ma
                         Serial.printf("  Secondary heating ON: Room %s (Pin %d)\n",
                                       room.name.c_str(), room.pinNumber);
                    } else {
                         Serial.printf("  Secondary room %s (Pin %d) shares pin/ID with already active room. Already ON.\n",
                                       room.name.c_str(), room.pinNumber);
                    }
                } else {
                     Serial.printf("  Warning: Invalid pin number (%d) for secondary room %s (ID %d)\n",
                                  room.pinNumber, room.name.c_str(), room.ID);
                }
                break;
            }
        }
    }

    // NOWE: Aktywuj przekaźnik pokoju "nadmiarowego" ciepła
    if (highTempRoomId != -1) {
        for (const auto &room : rooms) {
            if (room.ID == highTempRoomId) {
                 if (room.pinNumber >= 0 && room.pinNumber < 6) {
                    bool alreadyActive = activeRoomIDs.count(room.ID); // Sprawdź czy ID już jest aktywne

                    ExpOutput.digitalWrite(room.pinNumber, LOW); // LOW = ON
                     if (!alreadyActive) {
                         activeRoomIDs.insert(room.ID); // NOWE: Dodaj do aktywnych, jeśli jeszcze nie ma
                         Serial.printf("  High Temp Dissipation ON: Room %s (Pin %d)\n",
                                       room.name.c_str(), room.pinNumber);
                    } else {
                         Serial.printf("  High Temp room %s (Pin %d) shares pin/ID with already active room. Already ON.\n",
                                       room.name.c_str(), room.pinNumber);
                    }
                } else {
                     Serial.printf("  Warning: Invalid pin number (%d) for high temp room %s (ID %d)\n",
                                  room.pinNumber, room.name.c_str(), room.ID);
                }
                break;
            }
        }
    }

    // --- Sterowanie Gazem / Pompą Kominka ---
    Serial.println("Controlling Boiler/Pump...");
    bool anyRoomHeating = !activeRoomIDs.empty(); // Sprawdź, czy jakikolwiek pokój jest grzany

    if (anyRoomHeating) {
        if (useGaz_) {
            // Tryb Gazowy: Włącz gaz (P6) i pompę kominka (P7) - zakładając, że pompa jest potrzebna przy gazie?
            // Jeśli pompa P7 jest *tylko* dla kominka, to powinna być HIGH przy useGaz_ = true. Dostosuj wg potrzeb.
            ExpOutput.digitalWrite(P6, LOW);  // Włącz gaz
            ExpOutput.digitalWrite(P7, LOW);  // Włącz pompę (jeśli potrzebna przy gazie)
            // ExpOutput.digitalWrite(P7, HIGH); // Alternatywnie: Wyłącz pompę kominka przy gazie
            Serial.println("  Gas mode ON - P6 ON, P7 ON (or OFF depending on setup)");
        } else {
            // Tryb Kominkowy: Wyłącz gaz (P6), włącz pompę kominka (P7)
            ExpOutput.digitalWrite(P6, HIGH); // Wyłącz gaz
            ExpOutput.digitalWrite(P7, LOW);  // Włącz pompę kominka
            Serial.println("  Fireplace mode ON - P6 OFF, P7 ON");
        }
    } else {
        // Brak ogrzewania: Wyłącz gaz (P6) i pompę kominka (P7)
        ExpOutput.digitalWrite(P6, HIGH); // Wyłącz gaz
        ExpOutput.digitalWrite(P7, HIGH); // Wyłącz pompę kominka
        Serial.println("  No rooms heating - P6 OFF, P7 OFF");
    }


    // --- ZMIANA: Aktualizacja statusu 'valve' w RoomManager ---
    Serial.println("Updating valve status in RoomManager...");
    // Iteruj przez wszystkie pokoje w managerze i ustaw ich status valve
    // na podstawie tego, czy ich ID znajduje się w zbiorze activeRoomIDs
    // Używamy const_cast ostrożnie lub lepiej - dodajemy metodę w RoomManager
    // (Użyliśmy nowej metody updateValveStatus)
    for (const auto& room : manager.getAllRooms()) { // Iterujemy po kopii lub stałej referencji
         bool isActive = activeRoomIDs.count(room.ID) > 0;
         manager.updateValveStatus(room.ID, isActive); // Wywołaj nową metodę
    }


    // --- Aktualizacja docPins dla WebSocket ---
    // (Ta część wydaje się zbędna, bo broadcastWebsocket() wysyła cały stan z managera,
    // który teraz zawiera poprawny status 'valve'. Można ją usunąć lub zostawić dla debugowania.)
    Serial.println("Updating docPins for WebSocket (redundant?)...");
    for (const auto &room : manager.getAllRooms()) { // Użyj danych z managera
        if (room.pinNumber >= 0 && room.pinNumber < 6) {
            String pinStr = "pin_" + String(room.pinNumber);
            // Ustaw stan na podstawie zaktualizowanego room.valve
            docPins["pins"][pinStr]["state"] = room.valve ? "ON" : "OFF";
            docPins["pins"][pinStr]["forced"] = room.forced ? "true" : "false";
        }
    }
    // docPins["roomsInfo"] = manager.getRoomsAsJson(); // To jest wysyłane w broadcastWebsocket()

    Serial.println("--- End Heating Logic ---");
}
// Wyjaśnienie zmian:

// HIGH_MANIFOLD_TEMP_THRESHOLD: Zdefiniowano stałą dla progu temperatury.
// highTempRoomId / highestTempForHighTempRoom: Zmienne do śledzenia dodatkowego pokoju przy wysokiej temperaturze.
// activeRoomIDs (std::set): Zbiór przechowujący ID wszystkich pokoi, których zawory powinny być aktualnie otwarte. Użycie set ułatwia sprawdzanie, czy dany ID już istnieje i zapobiega duplikatom.
// Sprawdzanie temp.temperature na początku: Dodano bardziej solidne sprawdzanie isnan i minOperatingTemp na samym początku, aby uniknąć błędów w dalszej logice i zapewnić wyłączenie systemów.
// Trzeci przebieg (Pass 3): Dodano logikę do wyszukiwania highTempRoomId, jeśli temp.temperature przekracza próg. Wybiera pokój (inny niż główny/wtórny) z najwyższą aktualną temperaturą.
// Sterowanie przekaźnikami:
// Wszystkie przekaźniki są wyłączane na początku (relayMode(HIGH)).
// Gdy przekaźnik dla pokoju głównego, wtórnego lub "nadmiarowego" jest włączany (ExpOutput.digitalWrite(pin, LOW)), jego ID jest dodawane do zbioru activeRoomIDs.
// Dodano sprawdzanie, czy pin jest prawidłowy (>= 0 && < 6).
// Dodano sprawdzanie, czy ID pokoju nie jest już aktywne (np. gdy pokój wtórny ma ten sam pin co główny), aby uniknąć podwójnego logowania "ON".
// Sterowanie Gazem/Pompą: Logika została uproszczona - jeśli jakikolwiek pokój jest w activeRoomIDs, włącz odpowiednie źródło ciepła (gaz lub kominek). W przeciwnym razie wyłącz oba.
// Aktualizacja valve w RoomManager: Po zakończeniu całej logiki sterowania przekaźnikami, iterujemy przez wszystkie pokoje w manager i wywołujemy nową metodę manager.updateValveStatus(room.ID, isActive), gdzie isActive jest sprawdzane przez activeRoomIDs.count(room.ID) > 0. To zapewnia, że stan valve w RoomManager odzwierciedla rzeczywisty stan przekaźnika.
// Aktualizacja docPins: Ta sekcja jest prawdopodobnie zbędna, ponieważ broadcastWebsocket() wysyła cały JSON z manager.getRoomsAsJson(), który teraz będzie zawierał poprawny stan valve pobrany bezpośrednio z RoomData. Zostawiłem ją jednak, ale oznaczona jako potencjalnie redundantna.
// Logowanie: Dodano więcej szczegółowych logów, aby łatwiej było śledzić działanie logiki.
// Pamiętaj, aby przetestować tę logikę, szczególnie zachowanie przy wysokiej temperaturze kolektora i poprawność aktualizacji statusu valve w danych wysyłanych przez WebSocket.