#include "config.h"


// Определения переменных
const char* ssid = "wifi name";
const char* password = "password";
const String apiKey = "api"; //API ключ для OpenWeatherMap до 14.01

OneWire oneWire(ONE_WIRE_BUS); // Замените ONE_WIRE_BUS на ваш конкретный пин

// Объекты
FastBot2 bot;
CD74HC4067 mux(16, 5, 4, 0);
ESP8266WebServer server(80);

// Датчик
DS18B20Sensor ds18b20 = {
    {0},
    0,
    false,
    false,
    {0},
    0
};

// Инициализация
BotState currentState = NORMAL; // Инициализация в нормальное состояние
String city = ""; // Служебная переменная для хранения города

String userName = "";
String wateringTime = ""; // Хранит время полива в формате "HH:MM"
bool wateringScheduled = false;
time_t nextWateringTime = 0;
int wateringDelay = 0;
unsigned long previousMillis = 0; // Сохраняет время последнего запроса
const long interval = 600000; // Интервал между запросами (600000 мс = 10 минут)
