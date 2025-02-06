#include "MuxData.h"
#include "config.h"

String getMuxData(CD74HC4067& mux) {
    String results = ""; // строка для хранения результатов
    int channels[] = {4, 11, 15}; // каналы для считывания
    String labels[] = {"Содержание солей (TDS)", "Кислотность (pH)", "Мутность (NTU)"}; // Названия показателей
    for (unsigned int i = 0; i < sizeof(channels) / sizeof(channels[0]); i++) {
        int channel = channels[i];
        mux.channel(channel); // выбираем канал
        int value = analogRead(A0); // считываем значение
        // Обработка значений в зависимости от канала
        float calibratedValue;
        String alert = "";
        if (channel == 4) {
            int maxTDS = 500; // Максимально допустимый уровень TDS (ppm)
            calibratedValue = (value / 1023.0) * maxTDS; // Калибровка для TDS
            // Проверка для TDS
            if (calibratedValue > maxTDS) {
                alert = "Предупреждение: содержание солей превышает " + String(maxTDS) + " ppm.";
            }
        } else if (channel == 11) {
            float minPH = 7.0;
            float maxPH = 10.0;
            calibratedValue = minPH + ((value / 1023.0) * (maxPH - minPH)); // Калибровка для pH
            // Проверка для pH
            if (calibratedValue > maxPH) {
                alert = "Предупреждение: кислотность превышает " + String(maxPH) + ".";
            }
        } else if (channel == 15) {
            int maxTurbidity = 40; // Максимально допустимый уровень мутности
            calibratedValue = (value / 1023.0) * maxTurbidity; // Калибровка для мутности
            // Проверка для мутности
            if (calibratedValue > maxTurbidity) {
                alert = "Предупреждение: мутность превышает " + String(maxTurbidity) + " NTU.";
            }
        }
        // Добавляем результат в строку с названием показателя
        results += labels[i] + ": " + String(calibratedValue) + "\n"; // Используем откалиброванные значения
        if (alert != "") {
            results += alert + "\n"; // Добавляем предупреждение, если есть
        }
    }
    return results; // возвращаем результаты
}
