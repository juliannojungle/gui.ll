#ifndef _HAL_C_
#define _HAL_C_

#include "HAL.h"
#include "pico/stdio_usb.h"

void DigitalWrite(UINT pin, UBYTE value) {
    gpio_put(pin, value);
}

UBYTE DigitalRead(UINT pin) {
    return gpio_get(pin);
}

void SPIWriteByte(UBYTE value) {
    spi_write_blocking(LCD_SPI, &value, 1);
}

void SPIWriteNByte(UBYTE pData[], UDOUBLE len) {
    spi_write_blocking(LCD_SPI, pData, len);
}

void GPIOInit(UINT pin) {
    gpio_init(pin);
}

void GPIOSetDir(UINT pin, UINT mode) {
    gpio_set_dir(pin, mode);
}

void GPIOPullUp(UINT pin) {
    gpio_pull_up(pin);
}

void Delay(UINT milliseconds) {
    sleep_ms(milliseconds);
}

void STDIOInitAll(void) {
    stdio_init_all();
    stdio_set_translate_crlf(&stdio_usb, false);
}

void SPIInit(UINT speed) {
    spi_init(LCD_SPI, speed);
}

void GPIOSetFunction(UINT pin, UINT function) {
    gpio_set_function(pin, (UINT)function);
}

UINT PWMGPIOToSliceNum(UINT pin) {
    return pwm_gpio_to_slice_num(pin);
}

void PWMSetWrap(UINT slice, UINT value) {
    pwm_set_wrap(slice, value);
}

void PWMSetChannelLevel(UINT slice, UINT channel, UWORD level) {
    pwm_set_chan_level(slice, (UINT)channel, level);
}

void PWMSetClockDivider(UINT slice, float divider) {
    pwm_set_clkdiv(slice, divider);
}

void PWMSetEnabled(UINT slice, bool enable) {
    pwm_set_enabled(slice, enable);
}

#endif /* _HAL_C_ */