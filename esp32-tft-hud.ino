#include "FT6236.h"
#include "credentials.h"
#include "definitions.h"
#include "utils.h"
#include <NTPClient.h>
#include <SPI.h>
#include <TFT_eSPI.h>
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

TFT_eSPI tft = TFT_eSPI();
DataManager dataManager;
ScreenManager screenManager(tft);

TFT_eSPI_Button btnL, btnR;
TFT_eSPI_Button *buttons[] = { &btnL, &btnR };
uint8_t buttonCount = sizeof(buttons) / sizeof(buttons[0]);

int32_t lastRender = 0;
bool transitioning = false;

void setup() {
  Serial.begin(115200);

  initTft();
  initTouch();
  initWifi();
  initNtp();

  tft.fillScreen(TFT_BLACK);
  tft.setSwapBytes(!tft.getSwapBytes());

  // Register all screens
  screenManager.registerScreen(drawClockWeatherScreen);
  // screenManager.registerScreen(drawHourlyForecastScreen);
  // screenManager.registerScreen(drawDailyForecastScreen);
  // screenManager.registerScreen(drawSunTimesScreen);
  // screenManager.registerScreen(drawCalendarScreen);
  // screenManager.registerScreen(drawSystemScreen);

  // Setup navigation buttons
  btnL.initButtonUL(&tft, tft.width() - 105, tft.height() - 30, 50, 30,
                    TFT_WHITE, TFT_WHITE, TFT_BLACK, const_cast<char *>("<-"), 1);
  btnL.setLabelDatum(0, 0, MC_DATUM);

  btnR.initButtonUL(&tft, tft.width() - 50, tft.height() - 30, 50, 30,
                    TFT_WHITE, TFT_WHITE, TFT_BLACK, const_cast<char *>("->"), 1);
  btnR.setLabelDatum(0, 0, MC_DATUM);
  btnR.drawButton();
  btnL.drawButton();

    dataManager.updateWeather(LATITUDE, LONGITUDE, WEATHER_API_KEY);
    // dataManager.updateCalendar(CALENDAR_URL);
    // dataManager.updateSunTimes(LATITUDE, LONGITUDE);
}

void drawControls() {
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
}

void loop() {
  if (transitioning) {
    transitioning = false;
    tft.fillScreen(TFT_BLACK);
  }

  // Check for auto-rotation every 60 seconds
  screenManager.checkAutoRotate();

  // Update data if needed
  if (dataManager.shouldUpdateWeather()) {
    dataManager.updateWeather(LATITUDE, LONGITUDE, WEATHER_API_KEY);
  }
  if (dataManager.shouldUpdateCalendar()) {
    dataManager.updateCalendar(CALENDAR_URL);
  }
  if (dataManager.shouldUpdateSun()) {
    dataManager.updateSunTimes(LATITUDE, LONGITUDE);
  }

  // Render current screen
  drawControls();
  screenManager.render();

  delay(50); // 20 FPS
}

void initTft() {
  tft.init();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);
}

void initTouch() {
  byte error;
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.beginTransmission(TOUCH_I2C_ADD);
  error = Wire.endTransmission();
  if (error == 0) {
    Serial.print("I2C device found at address 0x");
    Serial.print(TOUCH_I2C_ADD, HEX);
    Serial.println("  !");
  } else {
    Serial.print("Unknown error at address 0x");
    Serial.println(TOUCH_I2C_ADD, HEX);
  }
}

void initWifi() {
  tft.setCursor(0, 0, 2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  Serial.print(F("[ WIFI ] Connecting..."));
  tft.print(F("[ WIFI ] Connecting..."));

  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WIFI_SSID_1, WIFI_PSK_1);
  wifiMulti.addAP(WIFI_SSID_2, WIFI_PSK_2);
  wifiMulti.addAP(WIFI_SSID_3, WIFI_PSK_3);

  while (wifiMulti.run() != WL_CONNECTED) {
    Serial.println(F("\n[ WIFI ] ERROR: Could not connect to wifi, rebooting..."));
    Serial.flush();
    ESP.restart();
  }
  tft.println(F("done!"));
  Serial.print(F("\n[ WIFI ] connected, SSID: "));
  Serial.print(WiFi.SSID());
  Serial.print(F(", IP:"));
  Serial.println(WiFi.localIP());
  Serial.println();
}

void initNtp() {
  Serial.println(F("[ NTP ] Obtaining time..."));
  tft.print(F("[ NTP ] Obtaining time..."));
  timeClient.begin();
  while (!timeClient.update()) {
    timeClient.forceUpdate();
  }
  tft.println(F("done!"));
  Serial.print(F("[ NTP ] time: "));
  Serial.println(timeClient.getFormattedTime());
  Serial.print(F("[ NTP ] epoch: "));
  Serial.println(timeClient.getEpochTime());
}
