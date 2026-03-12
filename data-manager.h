#pragma once
#include <ArduinoJson.h>
#include <HTTPClient.h>

class DataManager {
private:
  JsonDocument weatherData;
  JsonDocument calendarData;
  JsonDocument sunData;

  unsigned long lastWeatherUpdate = 0;
  unsigned long lastCalendarUpdate = 0;
  unsigned long lastSunUpdate = 0;

  const unsigned long WEATHER_UPDATE_INTERVAL = 10 * 60 * 1000;  // 10 min
  const unsigned long CALENDAR_UPDATE_INTERVAL = 30 * 60 * 1000; // 30 min
  const unsigned long SUN_UPDATE_INTERVAL = 24 * 60 * 60 * 1000;  // 24 hours

public:
  void updateWeather(float latitude, float longitude, const char* apiKey);
  void updateCalendar(const char* calendarUrl);
  void updateSunTimes(float latitude, float longitude);

  JsonDocument& getWeatherData() { return weatherData; }
  JsonDocument& getCalendarData() { return calendarData; }
  JsonDocument& getSunData() { return sunData; }

  unsigned long getLastWeatherUpdate() { return lastWeatherUpdate; }
  unsigned long getLastCalendarUpdate() { return lastCalendarUpdate; }
  unsigned long getLastSunUpdate() { return lastSunUpdate; }

  bool shouldUpdateWeather() { return millis() - lastWeatherUpdate > WEATHER_UPDATE_INTERVAL; }
  bool shouldUpdateCalendar() { return millis() - lastCalendarUpdate > CALENDAR_UPDATE_INTERVAL; }
  bool shouldUpdateSun() { return millis() - lastSunUpdate > SUN_UPDATE_INTERVAL; }
};
