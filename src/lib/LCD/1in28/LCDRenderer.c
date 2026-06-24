#ifndef __LCD_RENDERER_
#define __LCD_RENDERER_

#include "LCDSetup.h"
#include "LCDRenderer.h"
#include "HALConfig.h"
#include "HAL.h"
#include "Driver.h"

void LCDSetDisplayArea(UWORD xStart, UWORD yStart, UWORD xEnd, UWORD yEnd)
{
    DriverSendCommandData8Bit(0x2A, (UBYTE[]){0x00, xStart, 0x00, xEnd-1}, 4); //set the X coordinates
    DriverSendCommandData8Bit(0x2B, (UBYTE[]){0x00, yStart, 0x00, yEnd-1}, 4); //set the Y coordinates
    DriverSendCommand(0X2C);
}

void LCDClear(UWORD fillColor)
{
    UWORD j;
    UWORD image[LCD.WIDTH*LCD.HEIGHT];
    fillColor = ((fillColor<<8)&0xff00)|(fillColor>>8);

    for (j = 0; j < LCD.HEIGHT*LCD.WIDTH; j++) {
        image[j] = fillColor;
    }

    LCDSetDisplayArea(0, 0, LCD.WIDTH, LCD.HEIGHT);
    DigitalWrite(LCD_DC_PIN, 1);
    DigitalWrite(LCD_CS_PIN, 0);

    for (j = 0; j < LCD.HEIGHT; j++) {
        SPIWriteNByte((uint8_t *)&image[j*LCD.WIDTH], LCD.WIDTH*2);
    }

    DigitalWrite(LCD_CS_PIN, 1);
}

void LCDDisplayTexture(UWORD *image)
{
    UWORD j;
    LCDSetDisplayArea(0, 0, LCD.WIDTH, LCD.HEIGHT); /* full screen */
    DigitalWrite(LCD_DC_PIN, 1);
    DigitalWrite(LCD_CS_PIN, 0);

    for (j = 0; j < LCD.HEIGHT; j++) {
        SPIWriteNByte((uint8_t *)&image[j*LCD.WIDTH], LCD.WIDTH*2);
    }

    DigitalWrite(LCD_CS_PIN, 1);
    DriverSendCommand(0x29); /* Ensure display ON */
}

void LCDDisplayTextureInArea(UWORD xStart, UWORD yStart, UWORD xEnd, UWORD yEnd, UWORD *image)
{
    UDOUBLE addr = 0;
    UWORD j;
    LCDSetDisplayArea(xStart, yStart, xEnd , yEnd);
    DigitalWrite(LCD_DC_PIN, 1);
    DigitalWrite(LCD_CS_PIN, 0);

    for (j = yStart; j < yEnd - 1; j++) {
        addr = xStart + j * LCD.WIDTH ;
        SPIWriteNByte((uint8_t *)&image[addr], (xEnd-xStart)*2);
    }

    DigitalWrite(LCD_CS_PIN, 1);
}

void LCDDisplayTexturePoint(UWORD x, UWORD y, UWORD color)
{
    LCDSetDisplayArea(x,y,x,y);
    DriverSendData16Bit(color);
}

#endif /* __LCD_RENDERER_ */
