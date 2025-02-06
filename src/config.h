#ifndef CONFIG_H
#define CONFIG_H

#include <OneWire.h>
#include <FastBot2.h>
#include <CD74HC4067.h>
#include <ESP8266WebServer.h>
#include <Arduino.h>
#include <TimeLib.h>

// Константы для бота и датчиков
#define BOT_TOKEN "7939415831:AAFTq2tiWJpiH3KgoT09Ymhw80G2tbke5Hs"
#define CHAT_ID "860718018"
#define ONE_WIRE_BUS 2
#define TDS_CHANNEL 4
#define PH_CHANNEL 11
#define TURBIDITY_CHANNEL 15

// WiFi и API ключи
extern const char* ssid;
extern const char* password;
extern const String apiKey; //!!API ключ для OpenWeatherMap до 14.01

// Объекты
extern FastBot2 bot;
extern CD74HC4067 mux;
extern ESP8266WebServer server;

extern OneWire oneWire; // Объявление oneWire


// Структура для датчика
struct DS18B20Sensor {
    byte addr[8];
    float temperature;
    bool started;
    bool busy;
    byte data[9];
    unsigned long startTime;
};

// Перечисление для состояния бота
enum BotState { NORMAL, WAITING_FOR_CITY,WAITING_FOR_ALARM_TIME};

// Переменные
extern DS18B20Sensor ds18b20;
extern BotState currentState;
extern String city;
extern String userName;
extern String wateringTime;
extern bool wateringScheduled;
extern time_t nextWateringTime;
extern int wateringDelay;
extern unsigned long previousMillis;
extern const long interval;

#endif // CONFIG_H
