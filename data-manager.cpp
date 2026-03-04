#include "data-manager.h"

void DataManager::updateWeather(float latitude, float longitude, const char* apiKey) {
  HTTPClient http;
  String url = String("https://api.openweathermap.org/data/2.5/weather?lat=")
    + latitude + "&lon=" + longitude + "&appid=" + apiKey + "&units=metric";

  http.begin(url);
  int httpCode = http.GET();

  if (httpCode == 200) {
    String payload = http.getString();
    deserializeJson(weatherData, payload);
    lastWeatherUpdate = millis();
    Serial.println("[DATA] Weather updated");
  } else {
    Serial.print("[DATA] Weather update failed: ");
    Serial.println(httpCode);
  }
  http.end();
}

void DataManager::updateCalendar(const char* calendarUrl) {
  HTTPClient http;
  http.begin(calendarUrl);
  int httpCode = http.GET();

  if (httpCode == 200) {
    String payload = http.getString();
    deserializeJson(calendarData, payload);
    lastCalendarUpdate = millis();
    Serial.println("[DATA] Calendar updated");
  } else {
    Serial.print("[DATA] Calendar update failed: ");
    Serial.println(httpCode);
  }
  http.end();
}

void DataManager::updateSunTimes(float latitude, float longitude) {
  HTTPClient http;
  String url = String("https://api.sunrise-sunset.org/json?lat=")
    + latitude + "&lng=" + longitude + "&formatted=0";

  http.begin(url);
  int httpCode = http.GET();

  if (httpCode == 200) {
    String payload = http.getString();
    deserializeJson(sunData, payload);
    lastSunUpdate = millis();
    Serial.println("[DATA] Sun times updated");
  } else {
    Serial.print("[DATA] Sun times update failed: ");
    Serial.println(httpCode);
  }
  http.end();
}
