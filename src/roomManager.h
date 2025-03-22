#include <Arduino.h>
#include <iostream>
#include <vector>
#include <string>

struct RoomData
{
    std::string name;         // Nazwa pokoju
    int ID;                   // ID pokoju
    int pinNumber;            // Numer przyporzadkowanego pinu
    float targetTemperature;  // Temperatura zadana
    float currentTemperature; // Temperatura aktualna
    bool forced;              // Czy jest to tryb wymuszony
    std::string deviceMac;    // Addr mac urzadzenia Mia Bluetooth
    bool forced;              // Netatmo device status
    RoomData(std::string name, int pinNumber, float targetTemperature, float currentTemperature, bool forced)
        : name(name), pinNumber(pinNumber), targetTemperature(targetTemperature), currentTemperature(currentTemperature), forced(forced) {}
};

class RoomManager
{
public:
    void addRoom(const RoomData &room)
    {
        rooms.push_back(room);
    }

    RoomData getRoom(int index)
    {
        if (index >= 0 && index < rooms.size())
        {
            return rooms[index];
        }
        else
        {
            // Wyrzuć wyjątek lub zwróć pustą strukturę
            throw std::out_of_range("Index out of range");
        }
    }

    void updateRoom(int index, const RoomData &room)
    {
        if (index >= 0 && index < rooms.size())
        {
            rooms[index] = room;
        }
        else
        {
            // Wyrzuć wyjątek lub zwróć pustą strukturę
            throw std::out_of_range("Index out of range");
        }
    }

    void printRooms()
    {
        for (const auto &room : rooms)
        {
            std::cout << "Nazwa pokoju: " << room.name << std::endl;
            std::cout << "Numer pinu: " << room.pinNumber << std::endl;
            std::cout << "Temperatura zadana: " << room.targetTemperature << std::endl;
            std::cout << "Temperatura aktualna: " << room.currentTemperature << std::endl;
            std::cout << "Tryb wymuszony: " << (room.forced ? "Tak" : "Nie") << std::endl;
            std::cout << std::endl;
        }
    }

    

private:
    std::vector<RoomData> rooms;
};

// int main() {
//     RoomManager manager;

//     // Dodaj pokoje
//     manager.addRoom(RoomData("Pokój 1", 1, 20.0f, 22.0f, false));
//     manager.addRoom(RoomData("Pokój 2", 2, 22.0f, 24.0f, true));
//     manager.addRoom(RoomData("Pokój 3", 3, 20.0f, 22.0f, false));

//     // Wypisz pokoje
//     manager.printRooms();

//     // Zmień dane pokoju
//     RoomData room = manager.getRoom(1);
//     room.targetTemperature = 23.0f;
//     manager.updateRoom(1, room);

//     // Wypisz pokoje ponownie
//     manager.printRooms();

//     return 0;
// }
