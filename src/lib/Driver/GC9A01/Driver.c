/*****************************************************************************
* | File          :   Driver.c
* | Author      :   
* | Function    :   Hardware underlying interface
* | Info        :
*----------------
* |    This version:   V1.0
* | Date        :   2021-03-16
* | Info        :   
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documnetation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of theex Software, and to permit persons to  whom the Software is
# furished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
******************************************************************************/
#ifndef _DRIVER_C_
#define _DRIVER_C_

#include "HALConfig.h"

uint slice_num;

void DriverGPIOInit(void) {
    GPIOSetDir(LCD_RST_PIN, GPIO_OUT);
    GPIOSetDir(LCD_DC_PIN, GPIO_OUT);
    GPIOSetDir(LCD_CS_PIN, GPIO_OUT);

    DigitalWrite(LCD_CS_PIN, 1);
    DigitalWrite(LCD_DC_PIN, 0);
}

UBYTE DriverInit(void) {
    STDIOInitAll();

    // SPI Config
    SPIInit(10000 * 1000);
    GPIOSetFunction(LCD_CLK_PIN, GPIO_FUNC_SPI);
    GPIOSetFunction(LCD_MOSI_PIN, GPIO_FUNC_SPI);

    // GPIO Config
    DriverGPIOInit();

    // PWM Config (backlight)
    GPIOSetFunction(LCD_BL_PIN, GPIO_FUNC_PWM);
    slice_num = pwm_gpio_to_slice_num(LCD_BL_PIN);
    PWMSetWrap(slice_num, 100);
    PWMSetChannelLevel(slice_num, PWM_CHAN_B, 1);
    PWMSetClockDivider(slice_num, 50);
    PWMSetEnabled(slice_num, true);

    printf("DriverInit OK\r\n");
    return 0;
}

void DriverSetPWM(UBYTE value) {
    if (value > 100) {
        printf("DriverSetPWM error\r\n");
    } else {
        PWMSetChannelLevel(slice_num, 1, value);
    }
}

void DriverExit(void) {
}

#endif
