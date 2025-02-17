#include <math.h>
#include <GyverHTTP.h>
#include <Wire.h>
#include <ArduinoMqttClient.h>
#include <Arduino_ConnectionHandler.h>
#include <GSON.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include "Weather.h"
#include "MuxData.h"
#include "config.h"
#include "getWeather.h"
#include <TimeLib.h>
#include <TimeAlarms.h>


const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3 * 3600; // Смещение для MSK (UTC+3)
const int daylightOffset_sec = 0;

void printAddress(byte addr[8]) {
    for (byte i = 0; i < 8; i++) {
        if (addr[i] < 16) Serial.print("0");
        Serial.print(addr[i], HEX);
        if (i < 7) {
            Serial.print(":");
        }
    }
}

void setupDS18b20() {
    oneWire.reset_search(); // сброс поиска
    if (!oneWire.search(ds18b20.addr)) {
        Serial.println("No DS18B20 sensor found"); // не найден датчик
        return; // выход из функции
    }
    Serial.print("Found device with address: ");
    printAddress(ds18b20.addr); // вывод адреса найденного устройства
    Serial.println();
    if (OneWire::crc8(ds18b20.addr, 7) != ds18b20.addr[7] || ds18b20.addr[0] != 0x28) {
        Serial.println("Sensor is not a DS18B20"); // проверка типа датчика
        return; // выход из функции
    }
    Serial.println("Sensor is a DS18B20"); // подтверждение типа датчика
}

void startMeasurementDS18b20() {
    if (!ds18b20.started) { // если измерение не начато
        oneWire.reset(); // сброс OneWire
        oneWire.select(ds18b20.addr); // выбор устройства
        oneWire.write(0x44); // запуск измерения
        ds18b20.started = true; // установка состояния начато
        ds18b20.busy = true; // установка состояния занято
        ds18b20.startTime = millis(); // запоминаем время начала
        Serial.println("Measurement started"); // вывод сообщения о начале измерения
    }
}

void checkBusyDS18B20() {
    if (ds18b20.started && ds18b20.busy) { // если измерение начато и занято
        if (oneWire.read_bit()) { // проверяем завершение измерения
            Serial.println("Measurement completed"); // вывод сообщения о завершении
            ds18b20.busy = false; // сброс состояния занятости
        } else if (millis() - ds18b20.startTime >= 750) { // таймаут 750 мс
            ds18b20.temperature = -127; // неверное значение температуры
            Serial.println("Timeout: Measurement assumed completed"); // сообщение о таймауте
            ds18b20.busy = false; // сброс состояния
        }
    }
}

void getDataDS18B20() {
    if (ds18b20.started && !ds18b20.busy) { // если измерение начато и не занято
        oneWire.reset(); // сброс OneWire
        oneWire.select(ds18b20.addr); // выбор устройства
        oneWire.write(0xBE); // запрос на чтение данных
        for (int i = 0; i < 9; i++) {
            ds18b20.data[i] = oneWire.read(); // считываем данные
        }
        if (OneWire::crc8(ds18b20.data, 8) != ds18b20.data[8]) {
            Serial.println("CRC check failed. Invalid data received."); // проверка CRC
            ds18b20.temperature = -127; // неверное значение температуры
        } else {
            int16_t rawTemperature = (ds18b20.data[1] << 8) | ds18b20.data[0]; // преобразование данных
            ds18b20.temperature = (float)rawTemperature / 16.0; // вычисление температуры
        }
        Serial.print("Temperature: "); // вывод температуры
        Serial.print(ds18b20.temperature);
        Serial.println(" C");
        // Проверка температуры и отправка уведомлений
        if (ds18b20.temperature > 30.0) {
            bot.sendMessage(fb::Message("Внимание! Температура зашкаливает: " + String(ds18b20.temperature) + " C", CHAT_ID));
        } else if (ds18b20.temperature < 10.0) {
            bot.sendMessage(fb::Message("Внимание! Температура слишком низкая для растения: " + String(ds18b20.temperature) + " C", CHAT_ID));
        }
        ds18b20.started = false; // сброс для следующего измерения
    }
}

String getMuxData(CD74HC4067& mux);

void syncTime() {
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    struct tm timeinfo;
    while(!getLocalTime(&timeinfo)){
        Serial.print(".");
        delay(1000);
    }
    setTime(timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, 
            timeinfo.tm_mday, timeinfo.tm_mon+1, timeinfo.tm_year+1900);
}

void triggerAlarm() {
    bot.sendMessage(fb::Message("⏰ Время по будильнику! Проверьте ваше растение!", CHAT_ID));
}

void checkWateringTime() {
    if (wateringScheduled && now() >= nextWateringTime) {
        bot.sendMessage(fb::Message("Пора полить ваш цветок!", CHAT_ID));
        wateringScheduled = false; // Сбросить флаг после уведомления
    }
}


void setWateringTime(int hour, int minute) {
    time_t nowTime = now();
    tmElements_t timeElements;
    breakTime(nowTime, timeElements);


    timeElements.Hour = hour;
    timeElements.Minute = minute;
    timeElements.Second = 0;
   
    nextWateringTime = makeTime(timeElements);
   
    // Если время уже прошло, устанавливаем его на завтра
    if (nextWateringTime <= nowTime) {
        nextWateringTime += 24 * 60 * 60; // Установить на следующий день
    }
    wateringScheduled = true; // Установить флаг
}


String muxResults = getMuxData(mux);


void greetUser() {
    fb::Message msg("🌼 Здравствуйте! Я ваш помощник по уходу за растениями! Как я могу у вам обращаться? Пожалуйста введите своё имя на английском🥺", CHAT_ID);
    bot.sendMessage(msg);
}

void updateh(fb::Update& u) {
    String userMessage;
    if (u.isQuery()) {
        Serial.println("NEW QUERY");
        Serial.println(u.query().data());
        bot.answerCallbackQuery(u.query().id(), "query answered");
        String queryData = String(u.query().data().c_str());
        if (queryData == "/readings") {
            // Обработка получения показаний
            if (ds18b20.busy) {
                bot.sendMessage(fb::Message("Измерение в процессе. Пожалуйста, подождите.", u.query().from().id()));
            } else {
                startMeasurementDS18b20();
                delay(1000);
                getDataDS18B20();
                String muxResults = getMuxData(mux);
                String results = "Текущая температура: " + String(ds18b20.temperature) + " C\n" + muxResults;
                bot.sendMessage(fb::Message(results, u.query().from().id()));
            }
        } else if (queryData == "/notifications") {
            // Обработка кнопки "Уведомления"
            fb::Message msg("Выберите действие:", u.query().from().id());
            fb::InlineMenu menu("Таймер ; Будильник", "timer;alarm");
            msg.setInlineMenu(menu);
            bot.sendMessage(msg);
        } else if (queryData == "timer") {
            // Обработка кнопки "Таймер"
            fb::Message msg("Через сколько вам удобно установить таймер?", u.query().from().id());
            fb::InlineMenu menu("5 мин ; 10 мин ; 15 мин ; 30 мин ; 45 мин ; 1 час ; 2 часа",
                                "timer_5;timer_10;timer_15;timer_30;timer_45;timer_60;timer_120");
            msg.setInlineMenu(menu);
            bot.sendMessage(msg);
            } else if (queryData.startsWith("timer_")) {
            // Обработка выбора времени для таймера
            if (queryData == "timer_5") wateringDelay = 5 * 60; // 5 минут
            else if (queryData == "timer_10") wateringDelay = 10 * 60; // 10 минут
            else if (queryData == "timer_15") wateringDelay = 15 * 60; // 15 минут
            else if (queryData == "timer_30") wateringDelay = 30 * 60; // 30 минут
            else if (queryData == "timer_45") wateringDelay = 45 * 60; // 45 минут
            else if (queryData == "timer_60") wateringDelay = 1 * 60 * 60; // 1 час
            else if (queryData == "timer_120") wateringDelay = 2 * 60 * 60; // 2 часа


            bot.sendMessage(fb::Message("Таймер установлен на " + String(wateringDelay / 60) + " минут(ы).", u.query().from().id()));
           
            // Здесь можно добавить логику для отсчитывания времени на таймер
        } else if (queryData == "alarm") {
            // Обработка кнопки "Будильник"
            fb::Message msg("Укажите время для будильника в формате HH:MM (например 09:30):", u.query().from().id());
            bot.sendMessage(msg);
            currentState = WAITING_FOR_ALARM_TIME;
        } else if (currentState == WAITING_FOR_ALARM_TIME) {
            // Обработка ввода времени для будильника
            int hour, minute;
            if (sscanf(userMessage.c_str(), "%d:%d", &hour, &minute) == 2) {
                setWateringTime(hour, minute);  // Используем вашу функцию
                bot.sendMessage(fb::Message("Будильник установлен на " + String(hour) + ":" + String(minute) + ".", u.query().from().id()));
                currentState = NORMAL; // Возвращаемся в нормальное состояние
        } else {
                bot.sendMessage(fb::Message("Неверный формат. Пожалуйста, введите время в формате HH:MM.", u.message().chat().id()));
            }
        } else if (queryData == "watering") {
            // Обработка кнопки "Полив"
            fb::Message msg("Через сколько вам удобно полить цветок?", u.query().from().id());
            fb::InlineMenu menu("5 мин ; 10 мин ; 15 мин ; 30 мин ; 45 мин ; 1 час ; 2 часа",
                                "watering_5;watering_10;watering_15;watering_30;watering_45;watering_60;watering_120");
            msg.setInlineMenu(menu);
            bot.sendMessage(msg);
        } else if (queryData.startsWith("watering_")) {
            // Обработка выбора времени полива
            if (queryData == "watering_5") wateringDelay = 5 * 60; // 5 минут
            else if (queryData == "watering_10") wateringDelay = 10 * 60; // 10 минут
            else if (queryData == "watering_15") wateringDelay = 15 * 60; // 15 минут
            else if (queryData == "watering_30") wateringDelay = 30 * 60; // 30 минут
            else if (queryData == "watering_45") wateringDelay = 45 * 60; // 45 минут
            else if (queryData == "watering_60") wateringDelay = 1 * 60 * 60; // 1 час
            else if (queryData == "watering_120") wateringDelay = 2 * 60 * 60; // 2 часа
           
            time_t nowTime = now();
            nextWateringTime = nowTime + wateringDelay; // Установка следующего времени полива
            bot.sendMessage(fb::Message("Время полива установлено через " + String(wateringDelay / 60) + " минут(ы).", u.query().from().id()));
        } else if (queryData == "/advice") {
            // Обработка кнопки "Советы"
            fb::Message msg("Выберите категорию совета:", u.query().from().id());
            fb::InlineMenu menu("Температура ; Влажность ; Почва ; Удобрение ; Корни",
                                "temperature;humidity;soil;fertilizer;roots");
            msg.setInlineMenu(menu);
            bot.sendMessage(msg);
        } else if (queryData == "temperature") {
            // Советы по температуре
            String adviceMessage = "🌡️ Температура:\n"
                            "Оптимальная температура для большинства комнатных растений составляет 20-25°C. "
                            "Избегайте резких перепадов температуры.";
            bot.sendMessage(fb::Message(adviceMessage, u.query().from().id()));

            // Запрос о полезности совета
            fb::Message feedbackMsg("Был ли этот совет полезен?", u.query().from().id());
            fb::InlineMenu feedbackMenu("👍 ; 👎", "helpful;not_helpful");
            feedbackMsg.setInlineMenu(feedbackMenu);
            bot.sendMessage(feedbackMsg);
        } else if (queryData == "humidity") {
            // Советы по влажности
            String adviceMessage = "💧 Влажность:\n"
                                "Комнатные растения предпочитают влажность на уровне 50-70%. "
                                "Проверьте уровень влажности и используйте увлажнитель, если необходимо.";
            bot.sendMessage(fb::Message(adviceMessage, u.query().from().id()));

            // Запрос о полезности совета
            fb::Message feedbackMsg("Был ли этот совет полезен?", u.query().from().id());
            fb::InlineMenu feedbackMenu("👍 ; 👎", "helpful;not_helpful");
            feedbackMsg.setInlineMenu(feedbackMenu);
            bot.sendMessage(feedbackMsg);
        } else if (queryData == "soil") {
            // Советы по почве
            String adviceMessage = "🌱 Почва:\n"
                                "Используйте легкую, хорошо дренированную почву для комнатных растений. "
                                "Регулярно проверяйте уровень pH для ваших растений.";
            bot.sendMessage(fb::Message(adviceMessage, u.query().from().id()));

            // Запрос о полезности совета
            fb::Message feedbackMsg("Был ли этот совет полезен?", u.query().from().id());
            fb::InlineMenu feedbackMenu("👍 ; 👎", "helpful;not_helpful");
            feedbackMsg.setInlineMenu(feedbackMenu);
            bot.sendMessage(feedbackMsg);
        } else if (queryData == "fertilizer") {
            // Советы по удобрениям
            String adviceMessage = "🌼 Удобрение:\n"
                                "Подкармливайте растения каждые 4-6 недель в течение вегетационного периода. "
                                "Используйте удобрения, сбалансированные по основным микроэлементам.";
            bot.sendMessage(fb::Message(adviceMessage, u.query().from().id()));

            // Запрос о полезности совета
            fb::Message feedbackMsg("Был ли этот совет полезен?", u.query().from().id());
            fb::InlineMenu feedbackMenu("👍 ; 👎", "helpful;not_helpful");
            feedbackMsg.setInlineMenu(feedbackMenu);
            bot.sendMessage(feedbackMsg);
        } else if (queryData == "roots") {
            // Советы по корням
            String adviceMessage = "🪴 Корни:\n"
                                "Следите за здоровьем корней вашего растения. "
                                "Если корни перегружены, пересаживайте растение в новую почву.";
            bot.sendMessage(fb::Message(adviceMessage, u.query().from().id()));

            // Запрос о полезности совета
            fb::Message feedbackMsg("Был ли этот совет полезен?", u.query().from().id());
            fb::InlineMenu feedbackMenu("👍 ; 👎", "helpful;not_helpful");
            feedbackMsg.setInlineMenu(feedbackMenu);
            bot.sendMessage(feedbackMsg);
        } else if (queryData == "helpful") {
            bot.sendMessage(fb::Message("Спасибо за ваш отзыв! Рад, что совет был полезен.", u.query().from().id()));
        } else if (queryData == "not_helpful") {
            bot.sendMessage(fb::Message("Спасибо, что сообщили. Мы постоянно улучшаем наши советы!", u.query().from().id()));
        } else if (queryData == "add_substrate") {
            // Обработка кнопки "Добавление субстратов"
            bot.sendMessage(fb::Message("Уведомление о добавлении субстратов установлено.", u.query().from().id()));
        } else if (queryData == "/help") {
            String helpMessage = "🌿 Инструкция по использованию бота:\n\n"
                                "1. 📊 Показания - получение текущих данных:\n"
                                "   • Температура воздуха\n"
                                "   • Кислотность (pH)\n"
                                "   • Мутность воды\n"
                                "   • Содержание солей (TDS)\n\n"
                                "2. 🔔 Уведомления:\n"
                                "   • Настройка времени полива\n"
                                "   • Напоминания о добавлении субстратов\n\n"
                                "3. 💡 Советы - рекомендации по уходу за растением:\n"
                                "   • Оптимальные условия\n"
                                "   • Частота полива\n"
                                "   • Подкормка\n\n"
                                "4. ⚙️ Калибровка - настройка датчиков для точных измерений\n\n"
                                "5. 📈 Статистика - история показаний и графики\n\n"
                                "6. 🔄 Обновить - получение свежих данных\n\n"
                                "❗️ Автоматические уведомления:\n"
                                "• При высокой температуре (>30°C)\n"
                                "• При низкой температуре (<10°C)\n"
                                "• В установленное время полива\n"
                                "• При отклонении показателей от нормы\n\n"
                                "💬 Для использования просто выберите нужную команду в меню.";

            bot.sendMessage(fb::Message(helpMessage, u.query().from().id()));
        } else if (queryData == "/weather") {
            // Проверяем, установлен ли город
            if (city == "") {
                bot.sendMessage(fb::Message("Пожалуйста, введите ваш город:", u.query().from().id()));
                currentState = WAITING_FOR_CITY; // Устанавливаем состояние ожидания города
            } else {
                // Запрос погоды
                getWeather();
                // После успешного запроса погоды предлагаем изменить город
                fb::Message changeCityMsg("Хотите сменить город?", u.query().from().id());
                fb::InlineMenu changeCityMenu("Сменить город ; Нет", "change_city;no_change");
                changeCityMsg.setInlineMenu(changeCityMenu);
                bot.sendMessage(changeCityMsg);
            }
        } else if (queryData == "change_city") {
            bot.sendMessage(fb::Message("Пожалуйста, введите новый город:", u.query().from().id()));
            currentState = WAITING_FOR_CITY; // Снова устанавливаем состояние ожидания города
        } else if (queryData == "no_change") {
            bot.sendMessage(fb::Message("Хорошо, оставим текущий город: " + city, u.query().from().id()));
        }
        } else {
            String userMessage = String(u.message().text().c_str());
        // Проверка, если состояние ожидания города
            if (currentState == WAITING_FOR_CITY) {
                city = userMessage; // Сохраняем город, введенный пользователем
                getWeather(); // Сразу запрашиваем погоду после установки города
                currentState = NORMAL; // Возвращаемся в нормальное состояние
                return;
            }
            if (currentState == WAITING_FOR_ALARM_TIME) {
                int hour, minute;
            if (sscanf(userMessage.c_str(), "%d:%d", &hour, &minute) == 2) {
                if (hour >= 0 && hour <= 23 && minute >= 0 && minute <= 59) {
                    Alarm.alarmOnce(hour, minute, 0, triggerAlarm);
                    bot.sendMessage(fb::Message("Будильник установлен на " + String(hour) + ":" + 
                                      (minute < 10 ? "0" : "") + String(minute), u.message().chat().id()));
        } else {
            bot.sendMessage(fb::Message("Неверное время! Используйте формат 00:00-23:59", u.message().chat().id()));
        }
    } else {
        bot.sendMessage(fb::Message("Неверный формат! Используйте HH:MM", u.message().chat().id()));
    }
    currentState = NORMAL;
    return;
}
        // Обработка обычных сообщений
        Serial.println("NEW MESSAGE");
        Serial.println(u.message().from().username());
        Serial.println(u.message().text());
        //String userMessage = String(u.message().text().c_str());

        // Проверка, если состояние ожидания города
        if (currentState == WAITING_FOR_CITY) {
            city = userMessage; // Сохраняем город, введенный пользователем
            bot.sendMessage(fb::Message("Город установлен: " + city + ". Теперь вы можете использовать команду /weather.", u.message().chat().id()));
            currentState = NORMAL; // Возвращаемся в нормальное состояние
            return;
        }
        if (wateringScheduled) {
            // Обработка времени полива
            int hour, minute;
            if (sscanf(userMessage.c_str(), "%d:%d", &hour, &minute) == 2) {
                // Установка времени полива
                time_t nowTime = now();
                tmElements_t timeElements;
                breakTime(nowTime, timeElements);
                timeElements.Hour = hour;
                timeElements.Minute = minute;
                timeElements.Second = 0;
                time_t userTime = makeTime(timeElements);
                if (userTime <= nowTime) {
                    // Если указанное время уже прошло сегодня, устанавливаем на завтра
                    userTime += 24 * 60 * 60;
                }
                nextWateringTime = userTime;
                wateringTime = String(hour) + ":" + (minute < 10 ? "0" : "") + String(minute);
                bot.sendMessage(fb::Message("Время полива установлено на " + wateringTime + ".", u.message().chat().id()));
                wateringScheduled = false;
            } else {
                bot.sendMessage(fb::Message("Неверный формат. Пожалуйста, введите время в формате 08:00.", u.message().chat().id()));
            }
        } else if (userName == "") {
            userName = userMessage;
            String welcomeMessage = "Приятно познакомиться!🌼🌼 Выберите:";
            fb::Message msg(welcomeMessage, u.message().chat().id());
            fb::InlineMenu menu("Показания ; Уведомления ; Советы ; Погода ; Статистика ; Помощь",
                                "/readings;/notifications;/advice;/weather;/stats;/help");
            msg.setInlineMenu(menu);
            bot.sendMessage(msg);
        }
    }
}

void handleRoot() {
    String muxResults = getMuxData(mux); // Получаем результаты с мультиплексора
    String html = "<html><head>";
    html += "<style>";
    html += "body { font-family: Arial, sans-serif; margin: 0; padding: 20px; background-color: #f4f4f4; }";
    html += "h1 { color: #4CAF50; }"; // Цвет заголовка
    html += "h2 { color: #2196F3; }"; // Цвет второго заголовка
    html += ".container { max-width: 800px; margin: auto; padding: 20px; background: white; border-radius: 10px; box-shadow: 0 2px 10px rgba(0, 0, 0, 0.1); }";
    html += ".thermometer { width: 50px; height: 300px; background: #e0e0e0; border-radius: 25px; position: relative; margin: 20px auto; }";
    html += ".thermometer-fill { position: absolute; bottom: 0; width: 100%; border-radius: 25px; }";
    html += ".temperature-label { position: absolute; left: 1000px; font-size: 24px; color: #4CAF50; margin-top: -60px; }";
    html += ".results { white-space: pre-wrap; margin-top: 850px; }"; // Пробелы и перенос строк сохраняются
    html += "table { width: 100%; border-collapse: collapse; margin-top: 20px; }";
    html += "th, td { border: 1px solid #ccc; padding: 10px; text-align: left; }";
    html += "th { background-color: #f2f2f2; }";
    html += "</style></head><body>";
    html += "<div class='container'>";

    // Вычисление высоты для термометра
    float temperatureHeight = constrain(ds18b20.temperature * 10, 0, 300);
    // Определение цвета заполнения термометра в зависимости от температуры
    String fillColor;
    if (ds18b20.temperature <= 27) {
        fillColor = "green"; // до 25°C - зеленый
    } else if (ds18b20.temperature > 27 && ds18b20.temperature <= 35) {
        fillColor = "yellow"; // от 25°C до 35°C - желтый
    } else {
        fillColor = "red"; // свыше 35°C - красный
    }
    html += "<h1>Данные сенсоров</h1>";
    html += "<div class='thermometer'>";
    html += "<div class='thermometer-fill' style='height: " + String(temperatureHeight) + "px; background-color: " + fillColor + ";'></div>"; // Изменяем цвет заполнения
    html += "</div>";
    html += "<div class='temperature-label'>" + String(ds18b20.temperature) + " °C</div>";

    // Добавление таблицы для других показателей
    html += "<h2>Качество воды</h2>";
    html += "<table>";
    html += "<tr><th>Показатель</th><th>Значение</th></tr>";

    // Заполнение таблицы (например, вы можете извлекать значения из функции getMuxData)
    String results = muxResults; // Ваши результаты из getMuxData
    String lines[3];
    int lineCount = 0;

    // Разбиваем полученные результаты на строки для таблицы
    int startIndex = 0;
    while (startIndex < results.length()) {
        int endIndex = results.indexOf("\n", startIndex);
        if (endIndex == -1) endIndex = results.length();
        lines[lineCount] = results.substring(startIndex, endIndex);
        startIndex = endIndex + 1;
        lineCount++;
    }
    // Обработка каждой строки для отображения в таблице
    for (int i = 0; i < lineCount; i++) {
        String label = lines[i].substring(0, lines[i].indexOf(":")); // Название показателя
        String value = lines[i].substring(lines[i].indexOf(":") + 1); // Значение показателя
        html += "<tr><td>" + label + "</td><td>" + value + "</td></tr>";
    }

    html += "</table>"; // Закрытие таблицы
    html += "</div>"; // Закрываем контейнер
    html += "</body></html>";
    server.send(200, "text/html; charset=UTF-8", html);
}

void setup() {
    Serial.begin(9600);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println(WiFi.localIP());
    syncTime();
    setupDS18b20();
    bot.attachUpdate(updateh);
    bot.setToken(BOT_TOKEN);
    bot.setPollMode(fb::Poll::Long, 20000);
    greetUser();
    server.on("/", handleRoot); // Установка маршрута на корень
    server.begin();
    getWeather(); // Первоначальный запрос погоды
}

void loop() {
    server.handleClient();
    Alarm.delay(1000);
    if (!ds18b20.busy) {
        startMeasurementDS18b20();
    }
    checkBusyDS18B20();
    if (!ds18b20.busy) {
        getDataDS18B20();
    }

    // Периодический запрос данных о погоде
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis; // обновляем время последнего запроса
        getWeather(); // вызываем запрос к API с погодой
    }
    checkWateringTime();
    delay(3000);
    bot.tick();
}



