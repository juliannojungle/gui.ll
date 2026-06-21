#ifndef HAL_CONFIG_H
#define HAL_CONFIG_H

#include <stdbool.h>

#ifndef SD_SPI_SCLK
#define SD_SPI_SCLK 42
#endif
#ifndef SD_SPI_MOSI
#define SD_SPI_MOSI 41
#endif
#ifndef SD_SPI_MISO
#define SD_SPI_MISO 46
#endif
#ifndef SD_SPI_CS
#define SD_SPI_CS 45
#endif

/* SPI host for the SD card. ESP32-S3 routes any GPIO to any SPI host via the
 * GPIO matrix (SPI1_HOST is reserved for flash; SPI2_HOST/SPI3_HOST are free). */
#ifndef SD_SPI
#define SD_SPI SPI2_HOST
#endif

#ifndef SD_SPI_BAUDRATE
#define SD_SPI_BAUDRATE (25 * 1000 * 1000)
#endif

#ifndef SD_DETECT_PIN
#define SD_DETECT_PIN 39  // H1 pin 12 — card detect switch (LOW = card present, pull-up assumed)
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
#define LCD_BL_PIN 40
#endif

/* SPI host for the LCD. ESP32-S3 routes any GPIO to any SPI host via the GPIO
 * matrix, so the host isn't fixed by the pins. Kept separate from SD_SPI
 * (SPI2_HOST) so LCD and SD card can operate on independent buses. */
#ifndef LCD_SPI
#define LCD_SPI SPI3_HOST
#endif

#endif /* HAL_CONFIG_H */
