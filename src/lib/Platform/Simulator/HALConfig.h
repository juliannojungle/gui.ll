#ifndef HAL_CONFIG_H
#define HAL_CONFIG_H

#include <stdbool.h>

#define LCD_DC_PIN   0
#define LCD_CS_PIN   1
#define LCD_CLK_PIN  2
#define LCD_MOSI_PIN 3
#define LCD_RST_PIN  4
#define LCD_BL_PIN   5

#define SD_SPI_CS       6
#define SD_SPI_SCLK     7
#define SD_SPI_MOSI     8
#define SD_SPI_MISO     9
#define SD_SPI_BAUDRATE 25000000
#define SD_DETECT_PIN   10

#define SD_DIRECTORY "sample/sdcard"

#endif /* HAL_CONFIG_H */
