#ifndef _PNG_HELPER_C_
#define _PNG_HELPER_C_

#include "ff.h" // FS functions and declarations.
#include <setjmp.h>
#include <png.h>
#define PNG_READ_SUPPORTED

#include "HAL.h"
#include "Driver.h"
#include "LCD_1in28.h"
#include "Debug.h"
#include "PNGHelper.h"

void CustomReadData(png_structrp pngPointer, png_bytep data, size_t length) {
    SHOWDEBUG(".");
    UINT bytesRead;
    f_read((FIL*)png_get_io_ptr(pngPointer), data, length, &bytesRead);
}

static void ShowError(png_structp pngPointer, const char *message)
{
    SHOWDEBUG("Error from libpng: %s\n", message);
}

void DisplayPng(FIL *file) {
    SHOWDEBUG("Creating read structure\n");
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, ShowError, NULL);

    if (png_ptr == NULL) {
        SHOWDEBUG("png_create_read_struct error\n");
        return;
    }

    SHOWDEBUG("Allocating memory for image information\n");
    png_infop info_ptr = png_create_info_struct(png_ptr);

    if (info_ptr == NULL) {
        SHOWDEBUG("png_create_info_struct error\n");
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        return;
    }

    SHOWDEBUG("Setting up the custom read function\n");
    png_set_read_fn(png_ptr, file, CustomReadData);

    // State the error handler must clean up. Declared before setjmp and marked
    // volatile so their values survive a longjmp (C99 7.13.2.1).
    volatile png_bytep row_pointers = NULL;
    volatile bool lcdSelected = false;
    SHOWDEBUG("Setting up LongJump\n");

    if (setjmp(png_jmpbuf(png_ptr)) != 0) {
        // Any libpng error from here on jumps back to this point.
        SHOWDEBUG("We got a LongJump from a libpng error, cleaning up\n");

        if (row_pointers != NULL) {
            png_free(png_ptr, row_pointers);
        }

        if (lcdSelected) {
            DigitalWrite(LCD_CS_PIN, 1);
        }

        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return;
    }

    SHOWDEBUG("Reading info\n");
    /* png_read_info gets information about PNG file before the first IDAT (image data chunk). REQUIRED. */
    png_read_info(png_ptr, info_ptr);

    SHOWDEBUG("\nParsing image info\n");
    png_uint_32 width, height;
    int bit_depth, color_type, interlace_type;
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, NULL, NULL);
    SHOWDEBUG("PNG info: width: %d, height: %d, bit_depth: %d\n", width, height, bit_depth);

    int col, row;
    int maxCol = width > LCD.WIDTH ? LCD.WIDTH : width;     // won't print outside display.
    int maxRow = height > LCD.HEIGHT ? LCD.HEIGHT : height; // won't print outside display.

    // ####### LCDDisplayTexture #######
    SHOWDEBUG("Set display area\n");
    LCDSetDisplayArea(0, 0, maxCol, maxRow);
    DigitalWrite(LCD_DC_PIN, 1);
    DigitalWrite(LCD_CS_PIN, 0);
    lcdSelected = true;

    int num_palette = 0;
    png_colorp palette = NULL;

    if (color_type == PNG_COLOR_TYPE_PALETTE) {
        png_get_PLTE(png_ptr, info_ptr, &palette, &num_palette);
    }

    for (row = 0; row < maxRow; row++) {
        png_bytep rowBuffer = (png_bytep)png_malloc(png_ptr, png_get_rowbytes(png_ptr, info_ptr));
        row_pointers = rowBuffer; // track for cleanup before any call that may longjmp
        png_read_rows(png_ptr, &rowBuffer, NULL, 1);

        for (col = 0; col < maxCol; col++) {
            png_byte red, green, blue;

            if ((color_type == PNG_COLOR_TYPE_PALETTE) && (palette != NULL)) {
                red = palette[row_pointers[col]].red;
                green = palette[row_pointers[col]].green;
                blue = palette[row_pointers[col]].blue;
            } else {
                // if the image is paletted but we don't have a palette, display as grayscale using palette index.
                png_bytep pixel = &row_pointers[col];
                red = *(pixel++);
                green = *(pixel++);
                blue = *(pixel++);
            }

            /* The LCD uses RGB565 16-bits format: RRRRRGGG GGGBBBBB */
            SPIWriteByte((red & 0b11111000) | ((green & 0b11100000) >> 5));
            SPIWriteByte(((green & 0b00011100) << 3) | ((blue & 0b11111000) >> 3));
        }

        png_free(png_ptr, rowBuffer);
        row_pointers = NULL;
    }

    DigitalWrite(LCD_CS_PIN, 1);
    lcdSelected = false;
    DriverSendCommand(0x29); /* Ensure display ON */
    // ####### LCDDisplayTexture #######

    SHOWDEBUG("Done! Destroying read struct\n");
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
}

#endif