#include "getWeather.h"
#include "config.h"
#include "Weather.h"

extern String city; // Объявление внешней переменной city

void getWeather() {
    if (WiFi.status() == WL_CONNECTED) {
        WiFiClient client;
        HTTPClient http;

        String url = "http://api.weatherapi.com/v1/current.json?key=" + apiKey + "&q=" + city;
        Serial.print("Requesting weather data from: ");
        Serial.println(url);

        http.begin(client, url);
        int httpCode = http.GET();

        if (httpCode > 0) {
            if (httpCode == HTTP_CODE_OK) {
                String payload = http.getString();
                Serial.println("Weather data received:");
                Serial.println(payload);
                // Десериализация JSON
                DynamicJsonDocument doc(1024);
                DeserializationError error = deserializeJson(doc, payload);
                if (!error) {
                    String description = doc["current"]["condition"]["text"].as<String>();
                    // Использование функции для перевода описания погоды
                    description = getWeatherDescription(description);

                    float temperature = doc["current"]["temp_c"];
                    String weatherMessage = "☁️Погода в " + city + ": " + description + ", Температура: " + String(temperature) + "°C";
                    bot.sendMessage(fb::Message(weatherMessage, CHAT_ID));
                } else {
                    Serial.println("Failed to parse JSON!"); // Ошибка десериализации
                }
            }
        } else {
            Serial.printf("Error on HTTP request: %s\n", http.errorToString(httpCode).c_str());
        }
        http.end();
    } else {
        Serial.println("WiFi not connected");
    }
}
