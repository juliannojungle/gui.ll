#ifndef _HAL_C_
#define _HAL_C_

#include "HAL.h"

#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <stdio.h>

static spi_device_handle_t lcdSpiHandle = NULL;

void DigitalWrite(uint32_t pin, UINT8 value) {
    gpio_set_level((gpio_num_t)pin, value);
}

UINT8 DigitalRead(uint32_t pin) {
    return (UINT8)gpio_get_level((gpio_num_t)pin);
}

void SPIWriteByte(UINT8 value) {
    if (lcdSpiHandle == NULL) return;
    spi_transaction_t trans = {
        .length = 8,
        .tx_buffer = &value
    };
    spi_device_transmit(lcdSpiHandle, &trans);
}

void SPIWriteNByte(UINT8 pData[], UINT32 len) {
    if (lcdSpiHandle == NULL) return;
    spi_transaction_t trans = {
        .length = len * 8,
        .tx_buffer = pData
    };
    spi_device_transmit(lcdSpiHandle, &trans);
}

void GPIOInit(uint32_t pin) {
    gpio_reset_pin((gpio_num_t)pin);
}

void GPIOSetDir(uint32_t pin, uint32_t mode) {
    gpio_set_direction((gpio_num_t)pin, mode ? GPIO_MODE_OUTPUT : GPIO_MODE_INPUT);
}

void GPIOPullUp(uint32_t pin) {
    gpio_pullup_en((gpio_num_t)pin);
}

void Delay(uint32_t milliseconds) {
    vTaskDelay(pdMS_TO_TICKS(milliseconds));
}

void STDIOInitAll(void) {
    // ESP-IDF initializes stdio automatically via the console component.
}

void SPIInit(uint32_t speed) {
    // LCD SPI initialization for ESP32
    // Note: SD card SPI is handled separately in diskio.c
    spi_bus_config_t busCfg = {
        .mosi_io_num = LCD_MOSI_PIN,
        .miso_io_num = -1,  // LCD doesn't need MISO
        .sclk_io_num = LCD_CLK_PIN,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 240 * 240 * 2
    };

    spi_bus_initialize(LCD_SPI, &busCfg, SPI_DMA_CH_AUTO);

    spi_device_interface_config_t devCfg = {
        .clock_speed_hz = (int)speed,
        .mode = 0,
        .spics_io_num = -1,  // CS controlled manually
        .queue_size = 1
    };

    spi_bus_add_device(LCD_SPI, &devCfg, &lcdSpiHandle);
}

void GPIOSetFunction(uint32_t pin, uint32_t function) {
    // On ESP32, pin functions are set via peripheral-specific configs.
    // SPI pins are configured in SPIInit, I2C in I2CInit, PWM via LEDC.
    (void)pin;
    (void)function;
}

/* PWM compatibility using ESP32 LEDC peripheral */

static uint32_t pwmSlicePin = 0; // Store backlight pin for PWM

uint32_t PWMGPIOToSliceNum(uint32_t pin) {
    pwmSlicePin = pin;
    return 0; // ESP32 uses LEDC channels, not slices
}

void PWMSetWrap(uint32_t slice, uint32_t value) {
    (void)slice;
    (void)value;
    // LEDC duty resolution is set during channel config
}

void PWMSetChannelLevel(uint32_t slice, uint32_t channel, UINT16 level) {
    (void)slice;
    (void)channel;
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, level);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

void PWMSetClockDivider(uint32_t slice, float divider) {
    (void)slice;
    (void)divider;
    // Frequency is set during LEDC timer config
}

void PWMSetEnabled(uint32_t slice, bool enable) {
    (void)slice;
    if (enable) {
        ledc_timer_config_t timerCfg = {
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .timer_num = LEDC_TIMER_0,
            .duty_resolution = LEDC_TIMER_8_BIT,
            .freq_hz = 1000,
            .clk_cfg = LEDC_AUTO_CLK
        };
        ledc_timer_config(&timerCfg);

        ledc_channel_config_t chCfg = {
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .channel = LEDC_CHANNEL_0,
            .timer_sel = LEDC_TIMER_0,
            .intr_type = LEDC_INTR_DISABLE,
            .gpio_num = (int)pwmSlicePin,
            .duty = 0,
            .hpoint = 0
        };
        ledc_channel_config(&chCfg);
    }
}

#endif /* _HAL_C_ */
