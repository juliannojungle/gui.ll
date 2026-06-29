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
#include "HAL.h"
#include "Driver.h"

static UINT32 pwmBacklightBrightnessLevel;

void DriverGPIOMode(UINT16 pin, UINT16 mode) {
    GPIOInit(pin);

    if(mode == 0 || mode == GPIO_IN) {
        GPIOSetDir(pin, GPIO_IN);
    } else {
        GPIOSetDir(pin, GPIO_OUT);
    }
}

void DriverGPIOInitialize(void) {
    DriverGPIOMode(LCD_RST_PIN, GPIO_OUT);
    DriverGPIOMode(LCD_DC_PIN, GPIO_OUT);
    DriverGPIOMode(LCD_CS_PIN, GPIO_OUT);
    DriverGPIOMode(LCD_BL_PIN, GPIO_OUT);

    DigitalWrite(LCD_CS_PIN, 1);
    DigitalWrite(LCD_DC_PIN, 0);
    DigitalWrite(LCD_BL_PIN, 1);
}

void DriverBacklightPWMInitialize(void) {
    GPIOSetFunction(LCD_BL_PIN, GPIO_FUNC_PWM);
    pwmBacklightBrightnessLevel = PWMGPIOToSliceNum(LCD_BL_PIN);
    PWMSetWrap(pwmBacklightBrightnessLevel, 100);
    PWMSetChannelLevel(pwmBacklightBrightnessLevel, PWM_CHAN_B, 1);
    PWMSetClockDivider(pwmBacklightBrightnessLevel, 50);
    PWMSetEnabled(pwmBacklightBrightnessLevel, true);
}

void DriverSetBacklightBrightness(UINT8 brightness) {
    if (brightness < 100) {
        PWMSetChannelLevel(pwmBacklightBrightnessLevel, 1, brightness);
    }
}

UINT8 DriverInitialize(void) {
    STDIOInitAll();

    // SPI Config
    SPIInit(10000 * 1000);
    GPIOSetFunction(LCD_CLK_PIN, GPIO_FUNC_SPI);
    GPIOSetFunction(LCD_MOSI_PIN, GPIO_FUNC_SPI);

    DriverGPIOInitialize();
    DriverBacklightPWMInitialize();
    return 0;
}

void DriverHardwareReset(void)
{
    DigitalWrite(LCD_RST_PIN, 1);
    Delay(100);
    DigitalWrite(LCD_RST_PIN, 0);
    Delay(100);
    DigitalWrite(LCD_RST_PIN, 1);
    Delay(100);
}

void DriverSendCommand(UINT8 Reg)
{
    DigitalWrite(LCD_DC_PIN, 0);
    DigitalWrite(LCD_CS_PIN, 0);
    SPIWriteByte(Reg);
    DigitalWrite(LCD_CS_PIN, 1);
}

void DriverSendData8Bit(UINT8 Data)
{
    DigitalWrite(LCD_DC_PIN, 1);
    DigitalWrite(LCD_CS_PIN, 0);
    SPIWriteByte(Data);
    DigitalWrite(LCD_CS_PIN, 1);
}

void DriverSendCommandData8Bit(UINT8 command, UINT8 data[], int dataSize)
{
    DriverSendCommand(command);
    for (int i = 0; i < dataSize; i++) {
        DriverSendData8Bit(data[i]);
    }
}

void DriverSendData16Bit(UINT16 Data)
{
    DigitalWrite(LCD_DC_PIN, 1);
    DigitalWrite(LCD_CS_PIN, 0);
    SPIWriteByte((Data >> 8) & 0xFF);
    SPIWriteByte(Data & 0xFF);
    DigitalWrite(LCD_CS_PIN, 1);
}

#endif
