#define LGFX_MAKERFABS_TOUCHCAMERA
#include <LovyanGFX.hpp>
#include <LGFX_AUTODETECT.hpp>
static LGFX lcd;
static LGFX_Sprite sprite(&lcd);
#include "credentials.h"
#include "definitions.h"
#include "utils.h"
#include <NTPClient.h>
#include <WiFiMulti.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include "data-manager.h"
#include "screen-manager.h"
#include "screens.h"

WiFiMulti wifiMulti;
WiFiUDP ntpUDP;
// TODO timezone, available in weather api
NTPClient timeClient(ntpUDP, "uk.pool.ntp.org", 0, 60000);

// TFT_eSPI_Button btnL, btnR;
// TFT_eSPI_Button *buttons[] = { &btnL, &btnR };
// uint8_t buttonCount = sizeof(buttons) / sizeof(buttons[0]);

int32_t lastRender = 0;
bool transitioning = false;
// #include <lvgl.h>

DataManager dataManager;
ScreenManager screenManager;

void setup() {
  Serial.begin(115200);

  initTft();
  initWifi();
  initNtp();
  initDataManager();

  // Register all screens
  screenManager.registerScreen(drawClockWeatherScreen);
  // screenManager.registerScreen(drawHourlyForecastScreen);
  // screenManager.registerScreen(drawDailyForecastScreen);
  // screenManager.registerScreen(drawSunTimesScreen);
  // screenManager.registerScreen(drawCalendarScreen);
  // screenManager.registerScreen(drawSystemScreen);

  // Setup navigation buttons
  // btnL.initButtonUL(&tft, tft.width() - 105, tft.height() - 30, 50, 30,
  //                   TFT_WHITE, TFT_WHITE, TFT_BLACK, const_cast<char *>("<-"), 1);
  // btnL.setLabelDatum(0, 0, MC_DATUM);
  //
  // btnR.initButtonUL(&tft, tft.width() - 50, tft.height() - 30, 50, 30,
  //                   TFT_WHITE, TFT_WHITE, TFT_BLACK, const_cast<char *>("->"), 1);
  // btnR.setLabelDatum(0, 0, MC_DATUM);
  // btnR.drawButton();
  // btnL.drawButton();
  // TODO how to persist?
  if (false && lcd.touch()) {
    Serial.println("Has touch");
    if (lcd.width() < lcd.height()) lcd.setRotation(lcd.getRotation() ^ 1);

    lcd.setTextDatum(textdatum_t::middle_center);
    lcd.drawString("touch the arrow marker.", lcd.width()>>1, lcd.height() >> 1);
    lcd.setTextDatum(textdatum_t::top_left);

    std::uint16_t fg = TFT_WHITE;
    std::uint16_t bg = TFT_BLACK;
    if (lcd.isEPD()) std::swap(fg, bg);
    lcd.calibrateTouch(nullptr, fg, bg, std::max(lcd.width(), lcd.height()) >> 3);
  }
}

/* void drawControls() {
  int pos[2] = { 0, 0 };
  ft6236_pos(pos);

  for (uint8_t buttonIndex = 0; buttonIndex < buttonCount; buttonIndex++) {
    TFT_eSPI_Button *btn = buttons[buttonIndex];

    if (pos[0] != -1 && pos[1] != -1) {
      btn->press(btn->contains(tft.width() - pos[0], pos[1]));
    } else {
      btn->press(false);
    }

    if (btn->justReleased()) {
      btn->drawButton(false);
    } else if (btn->justPressed()) {
      btn->drawButton(true);
      if (btn == &btnL) {
        screenManager.prevScreen();
        transitioning = true;
      } else if (btn == &btnR) {
        screenManager.nextScreen();
        transitioning = true;
      }
    } else {
      btn->drawButton();
    }
  }
} */

void loop() {
  unsigned long frameStart = millis();
  if (transitioning) {
    transitioning = false;
    lcd.fillScreen(TFT_BLACK);
  }
    // lcd.fillScreen(TFT_BLACK);

  // Check for auto-rotation every 60 seconds
  screenManager.checkAutoRotate();

  // Update data if needed
  if (dataManager.shouldUpdateWeather()) {
    dataManager.updateWeather(LATITUDE, LONGITUDE, WEATHER_API_KEY);
  }
  /* if (dataManager.shouldUpdateCalendar()) {
    dataManager.updateCalendar(CALENDAR_URL);
  }
  if (dataManager.shouldUpdateSun()) {
    dataManager.updateSunTimes(LATITUDE, LONGITUDE);
  } */

  // Render current screen
  // drawControls();
  screenManager.render();

  // TODO wifiKeepAlive task
  // TODO time budget and delay so that fps target met
  // return this->updateInterval - (millis() - frameStart);
  //
  /* int remainingTimeBudget = ui.update();

  if (remainingTimeBudget > 0) {
    // You can do some work here
    // Don't do stuff if you are below your
    // time budget.
    delay(remainingTimeBudget);
  } */
  lcd.setCursor(0, lcd.height() - 16);
  lcd.print(F(" FPS: "));
  lcd.print((1 * 1000) / (millis() - frameStart));
  lcd.println("    ");
  delay(50);  // 20 FPS
}

void initTft() {
  lcd.init();
  lcd.setRotation(3);
  lcd.setBrightness(128);
  lcd.setColorDepth(16);

  lcd.fillScreen(TFT_BLACK);
}

void initWifi() {
  lcd.setCursor(0, 0);
  lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  Serial.print(F("[ WIFI ] Connecting..."));
  lcd.print(F("[ WIFI ] Connecting..."));

  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WIFI_SSID_1, WIFI_PSK_1);
  wifiMulti.addAP(WIFI_SSID_2, WIFI_PSK_2);
  wifiMulti.addAP(WIFI_SSID_3, WIFI_PSK_3);

  while (wifiMulti.run() != WL_CONNECTED) {
    Serial.println(F("\n[ WIFI ] ERROR: Could not connect to wifi, rebooting..."));
    Serial.flush();
    ESP.restart();
  }
  lcd.println(F("done!"));
  Serial.print(F("\n[ WIFI ] connected, SSID: "));
  Serial.print(WiFi.SSID());
  Serial.print(F(", IP:"));
  Serial.println(WiFi.localIP());
  Serial.println();
}

void initNtp() {
  Serial.println(F("[ NTP ] Obtaining time..."));
  lcd.print(F("[ NTP ] Obtaining time..."));
  timeClient.begin();
  while (!timeClient.update()) {
    timeClient.forceUpdate();
  }
  lcd.println(F("done!"));
  Serial.print(F("[ NTP ] time: "));
  Serial.println(timeClient.getFormattedTime());
  Serial.print(F("[ NTP ] epoch: "));
  Serial.println(timeClient.getEpochTime());
}

void initDataManager() {
  Serial.println(F("[ DataManager ] Getting initial data..."));
  lcd.print(F("[ DataManager ] Getting initial data..."));
  dataManager.updateWeather(LATITUDE, LONGITUDE, WEATHER_API_KEY);
  // dataManager.updateCalendar(CALENDAR_URL);
  // dataManager.updateSunTimes(LATITUDE, LONGITUDE);
  lcd.println(F("done!"));
}
