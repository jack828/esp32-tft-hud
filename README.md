# esp32-tft-hud

## What's it do?

This shows a bunch of metrics I want to see from my InfluxDB server in some pretty graphs + charts.

## The Board

Using [Makerfabs' ESP32 Touch with Camera (capacitive)](https://www.makerfabs.com/esp32-3.5-inch-tft-touch-capacitive-with-camera.html)

Which has:
  1. 3.5inch TFT ILI9488
  2. SD card Reader
  3. I2C Touch Screen (NS2009 or FT6236)
  4. OV2640 Camera

## Prerequisites

Install Bodmer's TFT_eSPI library and:
 - In `User_Setup_Select.h`
   - Comment out ALL setups except `#include <User_Setup.h>`
 - In `User_Setup.h`
   - Change the contents to the one in this repo.
 - In `TFT_eSPI.cpp`
   - Head to the bottom and move `#include <Extensions/Button.cpp>` out of the `#if TOUCH_CS` conditional

Install NTPClient

## Author

[Jack Burgess](https://jackburgess.dev)

## License

GNU GPLv3
