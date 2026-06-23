#ifndef _HAL_C_
#define _HAL_C_

#include "pico/stdlib.h"
#include "pico/stdio_usb.h"
#include "hardware/spi.h"
#include "hardware/pwm.h"
#include "HALConfig.h"

#define UBYTE uint8_t
#define UWORD uint16_t
#define UDOUBLE uint32_t

void DigitalWrite(uint pin, UBYTE value) {
    gpio_put(pin, value);
}

UBYTE DigitalRead(uint pin) {
    return gpio_get(pin);
}

void SPIWriteByte(UBYTE value) {
    spi_write_blocking(LCD_SPI, &value, 1);
}

void SPIWriteNByte(UBYTE pData[], UDOUBLE len) {
    spi_write_blocking(LCD_SPI, pData, len);
}

void GPIOInit(uint pin) {
    gpio_init(pin);
}

void GPIOSetDir(uint pin, uint mode) {
    gpio_set_dir(pin, mode);
}

void GPIOPullUp(uint pin) {
    gpio_pull_up(pin);
}

void Delay(uint milliseconds) {
    sleep_ms(milliseconds);
}

void STDIOInitAll(void) {
    stdio_init_all();
    stdio_set_translate_crlf(&stdio_usb, false);
}

void SPIInit(uint speed) {
    spi_init(LCD_SPI, speed);
}

void GPIOSetFunction(uint pin, uint function) {
    gpio_set_function(pin, (uint)function);
}

uint PWMGPIOToSliceNum(uint pin) {
    return pwm_gpio_to_slice_num(pin);
}

void PWMSetWrap(uint slice, uint value) {
    pwm_set_wrap(slice, value);
}

void PWMSetChannelLevel(uint slice, uint channel, UWORD level) {
    pwm_set_chan_level(slice, (uint)channel, level);
}

void PWMSetClockDivider(uint slice, float divider) {
    pwm_set_clkdiv(slice, divider);
}

void PWMSetEnabled(uint slice, bool enable) {
    pwm_set_enabled(slice, enable);
}

#endif /* _HAL_C_ */