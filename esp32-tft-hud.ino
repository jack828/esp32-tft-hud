#include "FT6236.h"
#include "credentials.h"
#include "definitions.h"
#include <NTPClient.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <WiFiMulti.h>
#include <WiFiUdp.h>
#include <time.h>
#include <InfluxDbClient.h>

WiFiMulti wifiMulti;
WiFiUDP ntpUDP;
// TODO timezone
NTPClient timeClient(ntpUDP, "uk.pool.ntp.org", 0, 60000);
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_DB);

TFT_eSPI tft = TFT_eSPI();
TFT_eSPI_Button btnL, btnR;
TFT_eSPI_Button *buttons[] = { &btnL, &btnR };
uint8_t buttonCount = sizeof(buttons) / sizeof(buttons[0]);

typedef void (*ScreenFunction)(void);

int32_t lastRender = 0;
int8_t screenIndex = 0;
bool transitioning = false;
int32_t lastUpdate = 0;
int32_t metricUpdateInterval = 10 * 1000;

void screen1() {
  tft.setCursor(0, 0, 2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  tft.print(F("g'day mate"));
}

void debugScreen() {
  tft.setCursor(0, 0, 2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  tft.print(F("IP: "));
  tft.print(WiFi.localIP());
  tft.print(F(" Time: "));
  tft.print(timeClient.getFormattedTime());
  tft.print(F(" Epoch: "));
  time_t epochTime = timeClient.getEpochTime();
  tft.println(epochTime);
  tft.print(F("Date: "));
  const tm *timeTm = localtime(&epochTime);
  tft.println(asctime(timeTm));


  tft.print(F("FPS: "));
  tft.println((1 * 1000) / (millis() - lastRender));
  lastRender = millis();
  /* tft.setCursor(0, tft.height() - 16, 2); */
  tft.print(F("Free heap: "));
  tft.print(ESP.getFreeHeap());
}

ScreenFunction screens[] = { screen1, debugScreen };
uint8_t screenCount = sizeof(screens) / sizeof(screens[0]);

void updateMetrics() {
  Serial.println("UPDATING MeTRICS");
  lastUpdate = millis();
}

void setup() {
  Serial.begin(115200);

  initTft();
  initTouch();
  initWifi();
  initNtp();
  initInflux();

  tft.fillScreen(TFT_BLACK);

  // Sun is static & screen buffer does not reset each frame
  tft.setSwapBytes(!tft.getSwapBytes());

  btnL.initButtonUL(
    &tft,
    tft.width() - 105,
    tft.height() - 30,
    50,
    30,
    TFT_WHITE,
    TFT_WHITE,
    TFT_BLACK,
    const_cast<char *>("<-"),
    1);

  btnL.setLabelDatum(0, 0, MC_DATUM);
  btnL.setButtonAction([]() {
    Serial.println("BUTTON LEFT PRESSED");
    if (transitioning) {
      // ignore if transitioning, i don't want to deal with this edge case
      return;
    }
    transitioning = true;
    if (screenIndex - 1 == -1) {  // Wrap around
      screenIndex = screenCount - 1;
      return;
    }
    screenIndex--;
  });
  btnL.drawButton();

  btnR.initButtonUL(
    &tft,
    tft.width() - 50,
    tft.height() - 30,
    50,
    30,
    TFT_WHITE,
    TFT_WHITE,
    TFT_BLACK,
    const_cast<char *>("->"),
    1);

  btnR.setLabelDatum(0, 0, MC_DATUM);
  btnR.setButtonAction([]() {
    Serial.println("BUTTON RIGHT PRESSED");
    if (transitioning) {
      // ignore if transitioning, i don't want to deal with this edge case
      return;
    }
    transitioning = true;
    if (screenIndex + 1 == screenCount) {  // Wrap around
      screenIndex = 0;
      return;
    }
    screenIndex++;
  });
  btnR.drawButton();
}

void drawControls() {
  tft.setCursor(0, 30, 2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  int pos[2] = { 0, 0 };
  ft6236_pos(pos);
  tft.print("Touch X: ");
  tft.print(pos[0]);
  tft.print(" Y: ");
  tft.print(pos[1]);
  tft.println("    ");

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
      btn->action();
      btn->drawButton(true);
    } else {
      btn->drawButton();
    }
  }
}

void loop() {
  lastRender = millis();

  if (transitioning) {
    transitioning = false;
    tft.fillScreen(TFT_BLACK);
  }
  drawControls();
  screens[screenIndex]();

  if (millis() - lastUpdate > metricUpdateInterval) {
    updateMetrics();
  }
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
    Serial.print(" - ");
    Serial.println(error);
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
    Serial.println(
      F("\n[ WIFI ] ERROR: Could not connect to wifi, rebooting..."));
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

void initInflux() {
  tft.print(F("[ INFLUX ] Connecting..."));
  bool influxOk = client.validateConnection();
  if (influxOk) {
    tft.println(F("done!"));
    Serial.print(F("[ INFLUX ] Connected to: "));
    Serial.println(client.getServerUrl());
  } else {
    Serial.print(F("[ INFLUX ] Connection failed: "));
    Serial.println(client.getLastErrorMessage());
    ESP.restart();
  }
}
