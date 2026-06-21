#ifndef _SD_HW_CONFIG_H_
#define _SD_HW_CONFIG_H_

#include "SDConfig.h"
#include "HALConfig.h"

/* -----------------------------------------------------------------------
 * Default SPI and SD card configuration for ESP32.
 * Users can expand these arrays to support multiple SPIs / SD cards.
 * ----------------------------------------------------------------------- */

static spi_t spis[] = {  // One for each SPI.
    {
        .host_id = 2,           // SPI2_HOST
        .miso_gpio = SD_SPI_MISO,
        .mosi_gpio = SD_SPI_MOSI,
        .sck_gpio = SD_SPI_SCLK,
        .baud_rate = SD_SPI_BAUDRATE
    }
};

static SdCard sd_cards[] = {  // One for each SD card.
    {
        .pcName = "0:",             // Name used to mount device
        .type = SD_IF_SPI,
        .spi_if.spi = &spis[0],    // Pointer to the SPI driver
        .spi_if.ss_gpio = SD_SPI_CS,
        .use_card_detect = true,
        .card_detect_gpio = SD_DETECT_PIN,
        .card_detected_true = 0    // LOW = card present (normally-open switch closes to GND, pull-up on SD_DETECT_PIN)
    }
};

/* BEGIN no-OS-FatFS-compatible implementations */

size_t sd_get_num(void) {
    return count_of(sd_cards);
}

SdCard *sd_get_by_num(size_t num) {
    if (num < sd_get_num()) {
        return &sd_cards[num];
    } else {
        return NULL;
    }
}

/* END no-OS-FatFS-compatible implementations */

#endif /* _SD_HW_CONFIG_H_ */
