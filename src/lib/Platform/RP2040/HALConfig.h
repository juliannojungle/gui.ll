#ifndef HAL_CONFIG_H
#define HAL_CONFIG_H

#include <stdbool.h>

#ifndef SD_SPI_SCLK
#define SD_SPI_SCLK 2
#endif
#ifndef SD_SPI_MOSI
#define SD_SPI_MOSI 3
#endif
#ifndef SD_SPI_MISO
#define SD_SPI_MISO 0
#endif
#ifndef SD_SPI_CS
#define SD_SPI_CS 1
#endif

#ifndef SD_SPI_BAUDRATE
#define SD_SPI_BAUDRATE (25 * 1000 * 1000)
#endif

#ifndef LCD_DC_PIN
#define LCD_DC_PIN 8
#endif
#ifndef LCD_CS_PIN
#define LCD_CS_PIN 9
#endif
#ifndef LCD_CLK_PIN
#define LCD_CLK_PIN 10
#endif
#ifndef LCD_MOSI_PIN
#define LCD_MOSI_PIN 11
#endif
#ifndef LCD_RST_PIN
#define LCD_RST_PIN 12
#endif
#ifndef LCD_BL_PIN
#define LCD_BL_PIN 25
#endif

bool Platform_SDCard_Init(void);

#endif /* HAL_CONFIG_H */
