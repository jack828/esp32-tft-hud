#pragma once
#include <TFT_eSPI.h>
#include <functional>

class ScreenManager {
private:
  typedef std::function<void(void)> ScreenFunction;

  ScreenFunction screens[6];
  uint8_t screenCount = 0;
  uint8_t currentScreen = 0;
  unsigned long lastScreenChange = 0;
  const unsigned long SCREEN_CHANGE_INTERVAL = 60 * 1000; // 60 seconds
  bool autoRotate = true;

  TFT_eSPI& tft;

public:
  ScreenManager(TFT_eSPI& tft);

  void registerScreen(ScreenFunction screen);
  void nextScreen();
  void prevScreen();
  void render();
  void checkAutoRotate();
};
