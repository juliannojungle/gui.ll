#ifndef HAL_CONFIG_H
#define HAL_CONFIG_H

#include <stdbool.h>

#ifndef SD_SPI_SCLK
#define SD_SPI_SCLK 18//2
#endif
#ifndef SD_SPI_MOSI
#define SD_SPI_MOSI 19//3
#endif
#ifndef SD_SPI_MISO
#define SD_SPI_MISO 16//0
#endif
#ifndef SD_SPI_CS
#define SD_SPI_CS 17//1
#endif

/* SPI peripheral instance — MUST match the SD_SPI_* pins above.
 * RP2040: GP0-7 / GP16-23 are spi0; GP8-15 / GP26-28 are spi1. */
#ifndef SD_SPI
#define SD_SPI spi0
#endif

#ifndef SD_SPI_BAUDRATE
#define SD_SPI_BAUDRATE (25 * 1000 * 1000)
#endif

#ifndef SD_DETECT_PIN
#define SD_DETECT_PIN 20//5   // H1 pin 12 — card detect switch (LOW = card present, pull-up assumed)
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

/* SPI peripheral instance for the LCD — MUST match the LCD_* pins above.
 * RP2040: GP0-7 / GP16-23 are spi0; GP8-15 / GP26-28 are spi1. */
#ifndef LCD_SPI
#define LCD_SPI spi1
#endif

bool Platform_SDCard_Init(void);

#endif /* HAL_CONFIG_H */
