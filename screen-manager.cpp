#include "screen-manager.h"
#include <Arduino.h>

ScreenManager::ScreenManager() {}

void ScreenManager::registerScreen(ScreenFunction screen) {
  if (screenCount < 6) {
    screens[screenCount++] = screen;
  }
}

void ScreenManager::nextScreen() {
  currentScreen = (currentScreen + 1) % screenCount;
  lastScreenChange = millis();
}

void ScreenManager::prevScreen() {
  currentScreen = (currentScreen == 0) ? screenCount - 1 : currentScreen - 1;
  lastScreenChange = millis();
}

void ScreenManager::render() {
  if (screenCount > 0) {
    screens[currentScreen]();
  }
}

void ScreenManager::checkAutoRotate() {
  if (autoRotate && millis() - lastScreenChange > SCREEN_CHANGE_INTERVAL) {
    Serial.println("[ ScreenManager ] Rotating...");
    nextScreen();
  }
}
