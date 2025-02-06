# Проект "Гидропонная система с автоматическим управлением" на Arduino

## Описание

Проект "Гидропонная система с автоматическим управлением" представляет собой систему, которая помогает заботиться о растениях с помощью различных сенсоров и уведомлений. Он использует микроконтроллер Arduino (например, ESP8266) для мониторинга температуры и других параметров, автоматизации полива и предоставления пользователю полезной информации о состоянии растений.

## Основные функции

1. **Измерение температуры**:
   - Используется датчик DS18B20 для измерения температуры окружающей среды.
   - В случае превышения пределов (выше 30°C или ниже 10°C) отправляются уведомления пользователю.

2. **Автоматизация полива**:
   - Поддержка установки времени полива.
   - Уведомления о необходимости полива растений.

3. **Получение погоды**:
   - Запрос данных о погоде через API.

4. **Интерфейс общения через Telegram**:
   - Пользователь может взаимодействовать с ботом для получения данных, установки таймеров и получения советов по уходу за растениями.

5. **Веб-интерфейс**:
   - Отображение данных сенсоров в виде веб-интерфейса с графическим отображением текущих показаний.

## Установка и подключение

### Необходимые компоненты

- Микроконтроллер ESP8266
- Датчик температуры DS18B20
- Мультиплексор CD74HC4067 (для работы с несколькими сенсорами)
- Компьютер с установленной средой разработки Arduino IDE и библиотеками: 
  - `GyverHTTP`
  - `ArduinoMqttClient`
  - `ArduinoJson`
  - `TimeLib`
  - `TimeAlarms`
  

## Использование

1. **Запуск**:
   - После загрузки кода на устройство, откройте Serial Monitor, чтобы увидеть адрес IP устройства.
   - Убедитесь, что устройство подключено к Wi-Fi.

2. **Взаимодействие с ботом**:
   - Напишите боту в Telegram, чтобы начать взаимодействие:
     - `Показания` - Получить текущие показания.
     - `Уведомления` - Настроить уведомления.Есть функция таймера и будильника.
     - `Советы` - Получить советы по уходу за растениями.
     - `Погода` - Получить текущую погоду.
     - `Статистика` - Планируется отображения графиков за 10-15 измерения
     - `Помощь` - Пользователь может ознакомиться с функционалом бота

3. **Веб-интерфейс**:
   - Откройте URL, указанный в Serial Monitor, для доступа к веб-интерфейсу с данными сенсоров.

## Программа

### Основные функции

- `setup()`: Настройка и инициализация подключения, запуск сенсоров и бота.
- `loop()`: Основной цикл программы, где происходит сбор данных и отправка уведомлений.

### Примеры команд

- **Получение показаний**:
   ```cpp
   if (u.isQuery() && queryData == "/readings") {
       ...
   }
   ```

- **Установка времени полива**:
   ```cpp
   void setWateringTime(int hour, int minute) {
       ...
   }
   ```

- **Отправка уведомлений**:
   ```cpp
   if (ds18b20.temperature > 30.0) {
       bot.sendMessage(...);
   }
   ```

## Заключение

Проект "Гидропонная система с автоматическим управлением" объединяет аппаратное обеспечение и программное обеспечение для создания интуитивно понятного интерфейса и улучшения ухода за растениями. Используя сенсоры, Telegram-бота и веб-интерфейс, пользователи могут легко контролировать состояние своих растений и получать актуальные уведомления.