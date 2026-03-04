#pragma once
#include "data-manager.h"
#include <NTPClient.h>
#include <TFT_eSPI.h>
#include <cstring>

extern TFT_eSPI tft;
extern DataManager dataManager;
extern NTPClient timeClient;
extern bool transitioning;

unsigned long lastFullRedraw = 0;
const unsigned long REDRAW_INTERVAL = 1000;

String lastTime = "";
char lastDate[32] = {0};

// Screen 1: Clock + Current Weather
void drawClockWeatherScreen() {
  // Check if we need a full redraw
  bool needsRedraw =
      transitioning || (millis() - lastFullRedraw > REDRAW_INTERVAL);

  if (!needsRedraw) {
    // return;
  }
  // tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  // Draw time (large)
  String newTime = timeClient.getFormattedTime();
  if (lastTime != newTime) {
    tft.setTextSize(4);
    tft.setCursor(20, 20);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.print(newTime);
    lastTime = newTime;
  }

  // Draw date
  time_t epochTime = timeClient.getEpochTime();
  struct tm *timeinfo = localtime(&epochTime);
  char dateStr[32];
  strftime(dateStr, sizeof(dateStr), "%A, %B %d, %Y", timeinfo);

  if (strcmp(lastDate, dateStr) != 0) {
    tft.setTextSize(2);
    tft.setCursor(20, 100);
    tft.print(dateStr);
    strcpy(lastDate, dateStr);
  }

  // Draw current weather (from weatherData)
  JsonDocument current = dataManager.getWeatherData();
  if (!current.isNull()) {
    tft.setTextSize(3);
    tft.setCursor(20, 160);
    tft.print("T: ");
    tft.print(current["main"]["temp"].as<int>());
    tft.print("C");

    tft.setTextSize(2);
    tft.setCursor(20, 210);
    const char *desc = current["weather"][0]["main"] | "N/A";
    tft.print(desc);

    // Draw humidity & pressure
    tft.setCursor(20, 250);
    tft.print("H: ");
    tft.print(current["main"]["humidity"].as<int>());
    tft.print("%");

    tft.setCursor(20, 280);
    tft.print("P: ");
    tft.print(current["main"]["pressure"].as<int>());
    tft.print(" hPa");
  } else {
    tft.setTextSize(2);
    tft.setCursor(20, 160);
    tft.print("Weather data loading...");
  }

  lastFullRedraw = millis();
}

// Screen 2: Hourly Forecast
void drawHourlyForecastScreen() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(20, 20);
  tft.print("Hourly Forecast (Next 24h)");

  JsonArray hourly = dataManager.getWeatherData()["hourly"];

  if (!hourly.isNull()) {
    int y = 60;
    int hourCount = 0;
    for (JsonObject hour : hourly) {
      if (hourCount >= 6)
        break; // Show 6 hours per view

      char timeStr[16];
      time_t hourTime = hour["dt"].as<time_t>();
      struct tm *timeinfo = localtime(&hourTime);
      strftime(timeStr, sizeof(timeStr), "%H:%M", timeinfo);

      tft.setCursor(20, y);
      tft.print(timeStr);
      tft.print(" | ");
      tft.print(hour["temp"].as<int>());
      tft.print("C | ");
      const char *desc = hour["weather"][0]["main"] | "N/A";
      tft.print(desc);

      y += 40;
      hourCount++;
    }
  } else {
    tft.setTextSize(2);
    tft.setCursor(20, 100);
    tft.print("Loading forecast...");
  }

  tft.setTextSize(1);
  tft.setCursor(20, tft.height() - 30);
  tft.print("Swipe or tap -> to see more hours");
}

// Screen 3: Daily Forecast
void drawDailyForecastScreen() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(20, 20);
  tft.print("7-Day Forecast");

  JsonArray daily = dataManager.getWeatherData()["daily"];

  if (!daily.isNull()) {
    int y = 60;
    int dayCount = 0;
    for (JsonObject day : daily) {
      if (dayCount >= 4)
        break; // Show 4 days per view

      char dateStr[16];
      time_t dayTime = day["dt"].as<time_t>();
      struct tm *timeinfo = localtime(&dayTime);
      strftime(dateStr, sizeof(dateStr), "%a %m/%d", timeinfo);

      tft.setCursor(20, y);
      tft.print(dateStr);
      tft.print(" | ");
      tft.print(day["temp"]["max"].as<int>());
      tft.print("/");
      tft.print(day["temp"]["min"].as<int>());
      tft.print("C | ");
      const char *desc = day["weather"][0]["main"] | "N/A";
      tft.print(desc);

      y += 50;
      dayCount++;
    }
  } else {
    tft.setTextSize(2);
    tft.setCursor(20, 100);
    tft.print("Loading forecast...");
  }
}

// Screen 4: Sunrise/Sunset
void drawSunTimesScreen() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(3);
  tft.setCursor(20, 40);
  tft.print("Sun Times");

  JsonDocument &sunDoc = dataManager.getSunData();
  JsonObject results = sunDoc["results"];

  if (!results.isNull()) {
    // Today
    tft.setTextSize(2);
    tft.setCursor(20, 120);
    tft.print("Today:");

    const char *sunriseStr = results["sunrise"] | "N/A";
    const char *sunsetStr = results["sunset"] | "N/A";

    tft.setCursor(40, 160);
    tft.print("Sunrise: ");
    tft.print(sunriseStr);

    tft.setCursor(40, 200);
    tft.print("Sunset: ");
    tft.print(sunsetStr);

    // Day length
    tft.setCursor(40, 240);
    tft.print("Civil Twilight: ");
    const char *civilTwilightBegin = results["civil_twilight_begin"] | "N/A";
    tft.print(civilTwilightBegin);
  } else {
    tft.setTextSize(2);
    tft.setCursor(20, 100);
    tft.print("Loading sun data...");
  }
}

// Screen 5: Calendar Events
void drawCalendarScreen() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(20, 20);
  tft.print("Upcoming Events");

  JsonArray events = dataManager.getCalendarData()["events"];

  if (!events.isNull()) {
    int y = 70;
    int eventCount = 0;
    for (JsonObject event : events) {
      if (eventCount >= 5)
        break; // Show 5 events

      tft.setCursor(20, y);
      tft.setTextSize(2);
      const char *title = event["title"] | "Untitled";
      tft.print(title);

      tft.setTextSize(1);
      tft.setCursor(20, y + 25);
      const char *eventTime = event["time"] | "N/A";
      tft.print(eventTime);

      y += 50;
      eventCount++;
    }
  } else {
    tft.setTextSize(2);
    tft.setCursor(20, 100);
    tft.print("No events or loading...");
  }
}

// Screen 6: System Info (your debug screen)
void drawSystemScreen() {
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0, 2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  tft.print(F("IP: "));
  tft.println(WiFi.localIP());

  tft.print(F("Time: "));
  tft.println(timeClient.getFormattedTime());

  tft.print(F("Free Heap: "));
  tft.print(ESP.getFreeHeap());
  tft.println(F(" bytes"));

  tft.print(F("WiFi Signal: "));
  tft.print(WiFi.RSSI());
  tft.println(F(" dBm"));

  tft.print(F("Uptime: "));
  unsigned long uptime = millis() / 1000;
  tft.print(uptime / 3600);
  tft.print("h ");
  tft.print((uptime % 3600) / 60);
  tft.println("m");
}
