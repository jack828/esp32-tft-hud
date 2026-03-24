#ifndef __DEFINITIONS_H__
#define __DEFINITIONS_H__

// I2C
#define I2C_SDA 26
#define I2C_SCL 27

// SDcard
#define SD_MOSI 13
#define SD_MISO 12
#define SD_CLK 14

#define LCD_CS 15
#define SD_CS 4

#define SPI_ON_SD digitalWrite(SD_CS, LOW)
#define SPI_OFF_SD digitalWrite(SD_CS, HIGH)

#endif
