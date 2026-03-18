#pragma once
#include "data-manager.h"
#include "lgfx_setup.h"
#include <NTPClient.h>
#include <cstring>

extern LGFX lcd;
extern DataManager dataManager;
extern NTPClient timeClient;
extern bool transitioning;

unsigned long lastFullRedraw = 0;
const unsigned long REDRAW_INTERVAL = 1000;

String lastTime = "";
char lastDate[32] = {0};
unsigned long lastWeatherDraw = 0;

// Screen 1: Clock + Current Weather
void drawClockWeatherScreen() {
  lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  lcd.setTextDatum(textdatum_t::middle_left);

  int16_t TOP_BAR_HEIGHT = 50;
  int16_t MIDDLE_BAR_HEIGHT = (lcd.height() / 2) - TOP_BAR_HEIGHT;
  int16_t SIDE_ICON_WIDTH = lcd.width() / 5;
  int16_t CENTRE_SEGMENT_WIDTH = (lcd.width() / 2) - SIDE_ICON_WIDTH;
  int16_t PAD = 2;

  // TOP_BAR
  lcd.drawRect(0, 0, lcd.width(), TOP_BAR_HEIGHT, TFT_GREEN);

  // TOP MIDDLE_GROUP
  // WEATHER_ICON
  lcd.drawRect(0, TOP_BAR_HEIGHT, SIDE_ICON_WIDTH, MIDDLE_BAR_HEIGHT, TFT_BLUE);
  int iconPadding = PAD * 2;
  lcd.drawRect(iconPadding, TOP_BAR_HEIGHT + iconPadding,
               SIDE_ICON_WIDTH - (iconPadding * 2),
               SIDE_ICON_WIDTH - (iconPadding * 2), TFT_WHITE);
  // TEMP_AND_DESCRIPTION
  lcd.drawRect(SIDE_ICON_WIDTH, TOP_BAR_HEIGHT, CENTRE_SEGMENT_WIDTH,
               MIDDLE_BAR_HEIGHT, TFT_BLUE);
  // RH_hPa_WIND
  lcd.drawRect((lcd.width() / 2), TOP_BAR_HEIGHT, CENTRE_SEGMENT_WIDTH,
               MIDDLE_BAR_HEIGHT, TFT_BLUE);
  // WIND_COMPASS
  lcd.drawRect(lcd.width() - (SIDE_ICON_WIDTH), TOP_BAR_HEIGHT, SIDE_ICON_WIDTH,
               MIDDLE_BAR_HEIGHT, TFT_BLUE);
  // end TOP MIDDLE_GROUP

  // BOTTOM MIDDLE_GROUP
  lcd.drawRect(0, lcd.height() / 2, lcd.width(), MIDDLE_BAR_HEIGHT, TFT_RED);
  // end BOTTOM MIDDLE_GROUP

  String newTime = timeClient.getFormattedTime();
  if (lastTime != newTime) {
    lcd.setTextSize(4);
    lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    lcd.drawString(newTime, PAD, TOP_BAR_HEIGHT / 2);
    lastTime = newTime;
  }

  time_t epochTime = timeClient.getEpochTime();
  struct tm *timeinfo = localtime(&epochTime);
  char dateStr[32];
  strftime(dateStr, sizeof(dateStr), "%A, %Y-%m-%d", timeinfo);

  if (strcmp(lastDate, dateStr) != 0) {
    lcd.setTextSize(2);
    lcd.drawString(dateStr, lcd.width() - lcd.textWidth(dateStr),
                   TOP_BAR_HEIGHT / 2);
    strcpy(lastDate, dateStr);
  }
  // TODO use lcd.setClipRect to define drawing bounds to not have to care about
  // overdraw if (dataManager.getLastWeatherUpdate() > lastWeatherDraw) {
  //   JsonDocument current = dataManager.getWeatherData();
  //   if (!current.isNull()) {

  // float temp = current["main"]["temp"].as<float>()
  float temp = epochTime & 1 ? 9.69 : 10.22;

  char tempStr[6];
  sprintf(tempStr, "%.2f", temp);
  lcd.setTextSize(4);
  lcd.setTextDatum(textdatum_t::top_left);
  int tempWidth = lcd.textWidth(tempStr);
  int tempSpacing = (CENTRE_SEGMENT_WIDTH - tempWidth) / 2;
  lcd.drawString(tempStr, SIDE_ICON_WIDTH + tempSpacing,
                 TOP_BAR_HEIGHT + (PAD * 4));

  lcd.setTextSize(2);

  // TODO clip to rect
  String description = epochTime & 1 ? "clear sky" : "scattered clouds";
  int descriptionWidth = lcd.textWidth(description);
  int descriptionX =
      descriptionWidth > SIDE_ICON_WIDTH + CENTRE_SEGMENT_WIDTH
          ? PAD
          : (SIDE_ICON_WIDTH + CENTRE_SEGMENT_WIDTH - descriptionWidth) / 2;
  lcd.setTextDatum(textdatum_t::bottom_left);
  lcd.drawString(description, descriptionX, TOP_BAR_HEIGHT + MIDDLE_BAR_HEIGHT);

  /// humidity, pressure, wind speed stack
  int STACK_ITEM_HEIGHT = MIDDLE_BAR_HEIGHT / 3;
  int itemY = TOP_BAR_HEIGHT + (PAD * 4);
  // humidity
  int humidity =
      epochTime & 1 ? 1 : 42; // current["main"]["humidity"].as<int>();
  char humidityStr[7];
  sprintf(humidityStr, "%d%% rH", humidity);
  int humidityWidth = lcd.textWidth(humidityStr);
  int humidityX =
      (lcd.width() / 2) + ((CENTRE_SEGMENT_WIDTH - humidityWidth) / 2);
  lcd.drawRect((lcd.width() / 2), TOP_BAR_HEIGHT, CENTRE_SEGMENT_WIDTH,
               STACK_ITEM_HEIGHT, TFT_WHITE);

  lcd.setTextDatum(textdatum_t::top_left);
  lcd.drawString(humidityStr, humidityX, itemY);
  itemY += STACK_ITEM_HEIGHT;

  // pressure
  int pressure =
      epochTime & 1 ? 1069 : 420; // current["main"]["pressure"].as<int>();
  char pressureStr[9];
  sprintf(pressureStr, "%d hPa", pressure);
  int pressureWidth = lcd.textWidth(pressureStr);
  int pressureX =
      (lcd.width() / 2) + ((CENTRE_SEGMENT_WIDTH - pressureWidth) / 2);

  lcd.drawRect((lcd.width() / 2), TOP_BAR_HEIGHT + STACK_ITEM_HEIGHT,
               CENTRE_SEGMENT_WIDTH, STACK_ITEM_HEIGHT, TFT_WHITE);

  lcd.setTextDatum(textdatum_t::top_left);
  lcd.drawString(pressureStr, pressureX, itemY);
  itemY += STACK_ITEM_HEIGHT;

  // wind
  float wind =
      epochTime & 1 ? 2.5f : 10.0f; // current["main"]["wind"].as<int>();
  char windStr[9];
  sprintf(windStr, "%.1f m/s", wind);
  int windWidth = lcd.textWidth(windStr);
  int windX = (lcd.width() / 2) + ((CENTRE_SEGMENT_WIDTH - windWidth) / 2);

  lcd.drawRect((lcd.width() / 2), TOP_BAR_HEIGHT + (STACK_ITEM_HEIGHT * 2),
               CENTRE_SEGMENT_WIDTH, STACK_ITEM_HEIGHT, TFT_WHITE);

  lcd.setTextDatum(textdatum_t::top_left);
  lcd.drawString(windStr, windX, itemY);

  /// wind direction compass
  lcd.drawRect((lcd.width() - SIDE_ICON_WIDTH) + iconPadding,
               TOP_BAR_HEIGHT + iconPadding,
               SIDE_ICON_WIDTH - (iconPadding * 2),
               SIDE_ICON_WIDTH - (iconPadding * 2), TFT_WHITE);
  /*
  lcd.print(current["wind"]["deg"].as<int>());
  lcd.print(" ");
  if(!current["wind"]["gust"].isNull()) {
    lcd.print(current["wind"]["gust"].as<float>());
    lcd.println("m/s");
  } else {
    lcd.println();
  }

  // lcd.setCursor(20, 280);
  if (!current["clouds"].isNull()) {
    lcd.print("C: ");
    lcd.print(current["clouds"]["all"].as<int>());
    lcd.print("%");
  }
*/

  // BOTTOM_BAR
  lcd.drawRect(0, lcd.height() - TOP_BAR_HEIGHT, lcd.width(), TOP_BAR_HEIGHT,
               TFT_GREEN);

  lcd.setTextSize(4);
  lcd.setTextDatum(textdatum_t::middle_right);
  int sunTimesY = lcd.height() - TOP_BAR_HEIGHT + (lcd.fontHeight() / 1);
  // TODO timezone
  time_t sunRiseTime = 1773728936; // TODO
  struct tm *sunRiseTimeInfo = localtime(&sunRiseTime);
  char sunRiseStr[16];
  strftime(sunRiseStr, sizeof(sunRiseStr), "%H:%M", sunRiseTimeInfo);
  int sunRiseX = lcd.width() / 2 -
                 (PAD * 4); // - lcd.textWidth(sunRiseStr);// - (PAD * 4);
  lcd.drawString(sunRiseStr, sunRiseX, sunTimesY);

  // TODO timezone
  lcd.setTextDatum(textdatum_t::middle_left);
  time_t sunSetTime = 1773771882; // TODO
  struct tm *sunSetTimeInfo = localtime(&sunSetTime);
  char sunSetStr[16];
  strftime(sunSetStr, sizeof(sunSetStr), "%H:%M", sunSetTimeInfo);
  int sunSetX = lcd.width() / 2 + (PAD * 4);
  lcd.drawString(sunSetStr, sunSetX, sunTimesY);
  // TODO image
  // https://wsrv.nl/?url=openweathermap.org/payload/api/media/file/10d%402x.png&output=jpg&quality=100
  // } else {
  // lcd.setTextSize(2);
  // lcd.setCursor(20, 160);
  // lcd.print("Weather data loading...");
  // }
  lastWeatherDraw = millis();
  // }
}
/*
// Screen 2: Hourly Forecast
void drawHourlyForecastScreen() {
  lcd.fillScreen(TFT_BLACK);
  lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  lcd.setTextSize(2);
  lcd.setCursor(20, 20);
  lcd.print("Hourly Forecast (Next 24h)");

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

      lcd.setCursor(20, y);
      lcd.print(timeStr);
      lcd.print(" | ");
      lcd.print(hour["temp"].as<int>());
      lcd.print("C | ");
      const char *desc = hour["weather"][0]["main"] | "N/A";
      lcd.print(desc);

      y += 40;
      hourCount++;
    }
  } else {
    lcd.setTextSize(2);
    lcd.setCursor(20, 100);
    lcd.print("Loading forecast...");
  }

  lcd.setTextSize(1);
  lcd.setCursor(20, lcd.height() - 30);
  lcd.print("Swipe or tap -> to see more hours");
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
} */
