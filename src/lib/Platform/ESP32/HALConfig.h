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

#ifndef SD_SPI_BAUDRATE
#define SD_SPI_BAUDRATE (25 * 1000 * 1000)
#endif

bool Platform_SDCard_Init(void);

#endif /* HAL_CONFIG_H */
