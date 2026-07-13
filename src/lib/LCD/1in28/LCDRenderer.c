#ifndef __LCD_RENDERER_
#define __LCD_RENDERER_

#include <png.h>
#include "LCDSetup.h"
#include "LCDRenderer.h"
#include "HALConfig.h"
#include "HAL.h"
#include "Driver.h"
#include "Debug.h"

void LCDSetDisplayArea(UINT16 xStart, UINT16 yStart, UINT16 xEnd, UINT16 yEnd)
{
    DriverSendCommandData8Bit(0x2A, (UINT8[]){0x00, xStart, 0x00, xEnd-1}, 4); //set the X coordinates
    DriverSendCommandData8Bit(0x2B, (UINT8[]){0x00, yStart, 0x00, yEnd-1}, 4); //set the Y coordinates
    DriverSendCommand(0X2C);
}

void LCDClear(UINT16 fillColor)
{
    UINT16 j;
    UINT16 texture[LCD.WIDTH*LCD.HEIGHT];
    fillColor = ((fillColor<<8)&0xff00)|(fillColor>>8);

    for (j = 0; j < LCD.HEIGHT*LCD.WIDTH; j++) {
        texture[j] = fillColor;
    }

    LCDSetDisplayArea(0, 0, LCD.WIDTH, LCD.HEIGHT);
    DigitalWrite(LCD_DC_PIN, 1);
    DigitalWrite(LCD_CS_PIN, 0);

    for (j = 0; j < LCD.HEIGHT; j++) {
        SPIWriteNByte((uint8_t *)&texture[j*LCD.WIDTH], LCD.WIDTH*2);
    }

    DigitalWrite(LCD_CS_PIN, 1);
}

void LCDRenderTexture(UINT8 *texture)
{
    UINT16 j;
    LCDSetDisplayArea(0, 0, LCD.WIDTH, LCD.HEIGHT); /* full screen */
    DigitalWrite(LCD_DC_PIN, 1);
    DigitalWrite(LCD_CS_PIN, 0);

    for (j = 0; j < LCD.HEIGHT; j++) {
        SPIWriteNByte(&texture[j * LCD.WIDTH * 2], LCD.WIDTH * 2);
    }

    DigitalWrite(LCD_CS_PIN, 1);
    DriverSendCommand(0x29); /* recover from DISPLAY OFF mode */
}

void LCDRenderTextureInArea(UINT16 xStart, UINT16 yStart, UINT16 xEnd, UINT16 yEnd, UINT8 *texture)
{
    UINT32 addr = 0;
    UINT16 j;
    LCDSetDisplayArea(xStart, yStart, xEnd , yEnd);
    DigitalWrite(LCD_DC_PIN, 1);
    DigitalWrite(LCD_CS_PIN, 0);

    for (j = yStart; j < yEnd - 1; j++) {
        addr = (xStart + j * LCD.WIDTH) * 2;
        SPIWriteNByte(&texture[addr], (xEnd - xStart) * 2);
    }

    DigitalWrite(LCD_CS_PIN, 1);
}

void LCDRenderPoint(UINT16 x, UINT16 y, UINT16 color)
{
    LCDSetDisplayArea(x,y,x,y);
    DriverSendData16Bit(color);
}

void PngCustomReadData(png_structrp pngPointer, png_bytep data, size_t length) {
    SHOWDEBUG(".");
    UINT bytesRead;
    f_read((FIL*)png_get_io_ptr(pngPointer), data, length, &bytesRead);
}

static void PngShowError(png_structp pngPointer, const char *message)
{
    SHOWDEBUG("Error from libpng: %s\n", message);
}

void LCDRenderPng(FIL *file) {
    SHOWDEBUG("Creating read structure\n");
    png_structp pngPointer = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, PngShowError, NULL);

    if (pngPointer == NULL) {
        SHOWDEBUG("png_create_read_struct error\n");
        return;
    }

    SHOWDEBUG("Allocating memory for texture information\n");
    png_infop infoPointer = png_create_info_struct(pngPointer);

    if (infoPointer == NULL) {
        SHOWDEBUG("png_create_info_struct error\n");
        png_destroy_read_struct(&pngPointer, NULL, NULL);
        return;
    }

    SHOWDEBUG("Setting up the custom read function\n");
    png_set_read_fn(pngPointer, file, PngCustomReadData);

    /* Declared volatile before setjmp so their values survive a longjmp (C99 7.13.2.1) and can be cleaned up. */
    volatile png_bytep rowPointers = NULL;
    volatile bool lcdSelected = false;
    SHOWDEBUG("Setting up LongJump\n");

    if (setjmp(png_jmpbuf(pngPointer)) != 0) {
        /* Any libpng error from here on jumps back to this point. */
        SHOWDEBUG("LongJump from a libpng error. Cleaning up.\n");

        if (rowPointers != NULL)
            png_free(pngPointer, rowPointers);

        if (lcdSelected)
            DigitalWrite(LCD_CS_PIN, 1);

        png_destroy_read_struct(&pngPointer, &infoPointer, NULL);
        return;
    }

    SHOWDEBUG("Reading info\n");
    /* png_read_info gets information about PNG file before the first IDAT (texture data chunk). REQUIRED. */
    png_read_info(pngPointer, infoPointer);
    png_set_packing(pngPointer); /* expand 1/2/4-bit samples to 1 byte per pixel */
    png_read_update_info(pngPointer, infoPointer);

    SHOWDEBUG("\nParsing texture info\n");
    png_uint_32 width, height;
    int bitDepth, colorType, interlaceType;
    png_get_IHDR(pngPointer, infoPointer, &width, &height, &bitDepth, &colorType, &interlaceType, NULL, NULL);
    SHOWDEBUG("PNG info: width: %d, height: %d, bit_depth: %d\n", width, height, bitDepth);

    int col, row;
    int maxCol = width > LCD.WIDTH ? LCD.WIDTH : width; /* won't print outside display. */
    int maxRow = height > LCD.HEIGHT ? LCD.HEIGHT : height;

    LCDSetDisplayArea(0, 0, maxCol, maxRow);
    SHOWDEBUG("LCD SPI chip select\n");
    DigitalWrite(LCD_DC_PIN, 1);
    DigitalWrite(LCD_CS_PIN, 0);
    lcdSelected = true;
    int num_palette = 0;
    png_colorp palette = NULL;

    if (colorType == PNG_COLOR_TYPE_PALETTE)
        png_get_PLTE(pngPointer, infoPointer, &palette, &num_palette);

    for (row = 0; row < maxRow; row++) {
        png_bytep rowBuffer = (png_bytep)png_malloc(pngPointer, png_get_rowbytes(pngPointer, infoPointer));
        rowPointers = rowBuffer; // track for cleanup before any call that may longjmp
        png_read_rows(pngPointer, &rowBuffer, NULL, 1);

        for (col = 0; col < maxCol; col++) {
            png_byte red, green, blue;

            if ((colorType == PNG_COLOR_TYPE_PALETTE) && (palette != NULL)) {
                red = palette[rowPointers[col]].red;
                green = palette[rowPointers[col]].green;
                blue = palette[rowPointers[col]].blue;
            } else {
                /* if the texture is paletted but we don't have a palette, display as grayscale using palette index. */
                png_bytep pixel = &rowPointers[col];
                red = *(pixel++);
                green = *(pixel++);
                blue = *(pixel++);
            }

            /* The LCD uses RGB565 16-bits format: RRRRRGGG GGGBBBBB */
            SPIWriteByte((red & 0b11111000) | ((green & 0b11100000) >> 5));
            SPIWriteByte(((green & 0b00011100) << 3) | ((blue & 0b11111000) >> 3));
        }

        png_free(pngPointer, rowBuffer);
        rowPointers = NULL;
    }

    DigitalWrite(LCD_CS_PIN, 1);
    lcdSelected = false;
    SHOWDEBUG("LCD SPI released\n");
    DriverSendCommand(0x29); /* recover from DISPLAY OFF mode */
    png_destroy_read_struct(&pngPointer, &infoPointer, NULL);
}

bool LCDRenderShouldClose() {
    return false;
}

void LCDRenderClose() {}

#endif /* __LCD_RENDERER_ */
