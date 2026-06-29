#ifndef _HAL_C_
#define _HAL_C_

#include "HAL.h"
#include "pico/stdio_usb.h"

void DigitalWrite(UINT32 pin, UINT8 value) {
    gpio_put(pin, value);
}

UINT8 DigitalRead(UINT32 pin) {
    return gpio_get(pin);
}

void SPIWriteByte(UINT8 value) {
    spi_write_blocking(LCD_SPI, &value, 1);
}

void SPIWriteNByte(UINT8 pData[], UINT32 len) {
    spi_write_blocking(LCD_SPI, pData, len);
}

void GPIOInit(UINT32 pin) {
    gpio_init(pin);
}

void GPIOSetDir(UINT32 pin, UINT32 mode) {
    gpio_set_dir(pin, mode);
}

void GPIOPullUp(UINT32 pin) {
    gpio_pull_up(pin);
}

void Delay(UINT32 milliseconds) {
    sleep_ms(milliseconds);
}

void STDIOInitAll(void) {
    stdio_init_all();
    stdio_set_translate_crlf(&stdio_usb, false);
}

void SPIInit(UINT32 speed) {
    spi_init(LCD_SPI, speed);
}

void GPIOSetFunction(UINT32 pin, UINT32 function) {
    gpio_set_function(pin, (UINT32)function);
}

UINT32 PWMGPIOToSliceNum(UINT32 pin) {
    return pwm_gpio_to_slice_num(pin);
}

void PWMSetWrap(UINT32 slice, UINT32 value) {
    pwm_set_wrap(slice, value);
}

void PWMSetChannelLevel(UINT32 slice, UINT32 channel, UINT16 level) {
    pwm_set_chan_level(slice, (UINT32)channel, level);
}

void PWMSetClockDivider(UINT32 slice, float divider) {
    pwm_set_clkdiv(slice, divider);
}

void PWMSetEnabled(UINT32 slice, bool enable) {
    pwm_set_enabled(slice, enable);
}

#endif /* _HAL_C_ */