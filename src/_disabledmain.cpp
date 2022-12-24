// #include <Arduino.h>
// #include <ArduinoJson.h>
// #include <ESP8266WiFi.h>
// #include <ESP8266HTTPClient.h>
// #include <WiFiClient.h>

// String clientId = "639e1b5f4fdefadabd00613c";
// String clientSecret = "WpXK2jPXAgsRc8SL8ggzJB6yzKGMOiWDqfHgodR";
// String usernameN = "zbigniew.karski@gmail.com";
// String passwordN = "Pmgana921!";

// const char* ssid = "|oooooi|";
// const char* password = "pmgana921";





// void getHomeData(String accessToken);


// void parseResponseToken(String response) {
//   // Utwórz obiekt parsera JSON
//   StaticJsonDocument<256> doc;

//   // Próbuj zparseować odpowiedź
//   DeserializationError error = deserializeJson(doc, response);
//   if (error) {
//     Serial.println("Wystąpił błąd podczas parsowania odpowiedzi");
//     return;
//   }

//   // Pobierz informacje o tokenie dostępu
//   String accessToken = doc["access_token"];
//   String refreshToken = doc["refresh_token"];
//   int expiresIn = doc["expires_in"];

//   // Wyświetl informacje o tokenie dostępu
//   Serial.println("Token dostępu: " + accessToken);
//   Serial.println("Token odświeżania: " + refreshToken);
//   Serial.println("Czas ważności: " + String(expiresIn) + " sekund");

//   delay(10000);

//   Serial.println("Pobieranie danych domu");

//   getHomeData(accessToken);



// }





// void getAccesToken(){


//   HTTPClient http;
//   WiFiClient client;


//  // Skonfiguruj żądanie POST
//   String url = "https://api.netatmo.com/oauth2/token";




// http.begin( client, url.c_str() );

//   http.addHeader("Content-Type", "application/x-www-form-urlencoded");

//   // Utwórz ciało żądania
//   String body = "grant_type=password";
//   body += "&client_id=" + clientId;
//   body += "&client_secret=" + clientSecret;
//   body += "&username=" + usernameN;
//   body += "&password=" + passwordN;
//   body += "&scope=read_homecoach";

//   // Wyślij żądanie POST
//   int httpCode = http.POST(body);

//   Serial.println("HTTP CODE "+ httpCode);
//   if (httpCode > 0) {
//     // Odczytaj odpowiedź serwera
//     String response = http.getString();


//     Serial.println(response);
//     //parseResponseToken(response);


//   } else {
//     Serial.println("Wystąpił błąd podczas wysyłania żądania");

//   }

//   // Zakończ połączenie
//   http.end();


// }







// void getHomeData(String accessToken) {
//   // Utwórz obiekt HTTPClient
//   HTTPClient http;
//   WiFiClient client;
  

//   // Skonfiguruj żądanie GET
//   String url = "https://api.netatmo.com/api/gethomedata?access_token=" + accessToken;
//   http.begin(client,url);

//   // Dodaj nagłówek autoryzacji
//   http.addHeader("Authorization", "Bearer " + accessToken);

//   // Wyślij żądanie GET
//   int httpCode = http.GET();
//   if (httpCode > 0) {
//     // Odczytaj odpowiedź serwera
//     String response = http.getString();

//     // Parsuj odpowiedź serwera
//     DynamicJsonDocument doc(1024);
//     DeserializationError error = deserializeJson(doc, response);
//     if (error) {
//       Serial.println("Błąd podczas parsowania odpowiedzi");
//       return;
//     }

//     // Pobierz pożądane pola z odpowiedzi
//     String homeName = doc["body"]["homes"][0]["name"];
//     String homeCountry = doc["body"]["homes"][0]["country"];


//     // Wyświetl informacje o stacji

// Serial.println("homeName: " + homeName);
// Serial.println("homeCountry: " + homeCountry);
// Serial.println("-------------- RAW JSON -----------");

// Serial.println(response);

//     // ...
//   } else {
//     Serial.println("Wystąpił błąd podczas wysyłania żądania");
//   }

//   // Zakończ połączenie
//   http.end();
// }


// void setup() {
//   // Połącz się z WiFi
//   WiFi.begin(ssid, password);
//   Serial.begin(9600);
//   Serial.println("WiFi Connecting");

//   while (WiFi.status() != WL_CONNECTED) {
//     delay(500);
//     Serial.print(".");
//   }
    

//   getAccesToken();
  
// }

// void loop() {

//   delay(20000);
// }

