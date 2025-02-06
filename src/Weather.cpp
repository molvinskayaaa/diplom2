#include "Weather.h"

String getWeatherDescription(String englishDescription) {
    if (englishDescription == "Sunny") return "☀️ Солнечно";
    if (englishDescription == "Partly cloudy") return "🌤 Частично облачно";
    if (englishDescription == "Cloudy") return "☁️ Облачно";
    if (englishDescription == "Overcast") return "☁️ Пасмурно";
    if (englishDescription == "Mist") return "🌫️ Мгла";
    if (englishDescription == "Patchy rain possible") return "🌧️ Возможен мелкий дождь";
    if (englishDescription == "Patchy snow possible") return "❄️ Возможен мелкий снег";
    if (englishDescription == "Patchy sleet possible") return "🌨️ Возможен мелкий дождь со снегом";
    if (englishDescription == "Patchy freezing drizzle possible") return "🌧️ Возможен мелкий дождь с заморозками";
    if (englishDescription == "Thundery outbreaks possible") return "🌩️ Возможны грозы";
    if (englishDescription == "Blowing snow") return "🌨️ Дующий снег";
    if (englishDescription == "Blizzard") return "🌨️ Метель";
    if (englishDescription == "Fog") return "🌫️ Туман";
    if (englishDescription == "Freezing fog") return "🌫️ Замерзающий туман";
    if (englishDescription == "Patchy light drizzle") return "🌧️ Возможен мелкий дождь";
    if (englishDescription == "Light drizzle") return "🌧️ Легкий дождь";
    if (englishDescription == "Freezing drizzle") return "🌧️ Замерзающий дождь";
    if (englishDescription == "Heavy freezing drizzle") return "🌧️ Сильный замерзающий дождь";
    if (englishDescription == "Patchy light rain") return "🌧️ Возможен легкий дождь";
    if (englishDescription == "Light rain") return "🌧️ Легкий дождь";
    if (englishDescription == "Moderate rain at times") return "🌧️ Временами умеренный дождь";
    if (englishDescription == "Moderate rain") return "🌧️ Умеренный дождь";
    if (englishDescription == "Heavy rain at times") return "🌧️ Временами сильный дождь";
    if (englishDescription == "Heavy rain") return "🌧️ Сильный дождь";
    if (englishDescription == "Light freezing rain") return "🌧️ Легкий замерзающий дождь";
    if (englishDescription == "Moderate or heavy freezing rain") return "🌧️ Умеренный или сильный замерзающий дождь";
    if (englishDescription == "Light sleet") return "🌧️ Легкий дождь со снегом";
    if (englishDescription == "Moderate or heavy sleet") return "🌧️ Умеренный или сильный дождь со снегом";
    if (englishDescription == "Patchy light snow") return "❄️ Возможен легкий снег";
    if (englishDescription == "Light snow") return "❄️ Легкий снег";
    if (englishDescription == "Patchy moderate snow") return "❄️ Возможен умеренный снег";
    if (englishDescription == "Moderate snow") return "❄️ Умеренный снег";
    if (englishDescription == "Patchy heavy snow") return "❄️ Возможен сильный снег";
    if (englishDescription == "Heavy snow") return "❄️ Сильный снег";
    if (englishDescription == "Ice pellets") return "❄️ Ледяные гранулы";
    if (englishDescription == "Light rain shower") return "🌧️ Легкий дождевой ливень";
    if (englishDescription == "Moderate or heavy rain shower") return "🌧️ Умеренный или сильный дождевой ливень";
    if (englishDescription == "Torrential rain shower") return "🌧️ Сильный дождевой ливень";
    if (englishDescription == "Light sleet showers") return "🌧️ Легкие дожди со снегом";
    if (englishDescription == "Moderate or heavy sleet showers") return "🌧️ Умеренные или сильные дожди со снегом";
    if (englishDescription == "Light snow showers") return "❄️ Легкие снегопады";
    if (englishDescription == "Moderate or heavy snow showers") return "❄️ Умеренные или сильные снегопады";
    if (englishDescription == "Light showers of ice pellets") return "❄️ Легкие дожди ледяных гранул";
    if (englishDescription == "Moderate or heavy showers of ice pellets") return "❄️ Умеренные или сильные дожди ледяных гранул";
    if (englishDescription == "Patchy light rain with thunder") return "🌧️ Возможен легкий дождь с грозой";
    if (englishDescription == "Moderate or heavy rain with thunder") return "🌧️ Умеренный или сильный дождь с грозой";
    if (englishDescription == "Patchy light snow with thunder") return "❄️ Возможен легкий снег с грозой";
    if (englishDescription == "Moderate or heavy snow with thunder") return "❄️ Умеренный или сильный снег с грозой";

    return englishDescription; // Возвращаем оригинальное описание, если перевод не найден
}
