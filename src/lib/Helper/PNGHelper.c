#ifndef _PNG_HELPER_C_
#define _PNG_HELPER_C_

#include "ff.h" // FS functions and declarations.
#include <setjmp.h>
#include <png.h>
#define PNG_READ_SUPPORTED

#include "Driver.c"
#include "HAL.c"
#include "LCD_1in28.c"
#include "font20.c"
#include "Canvas.h"
#include "Debug.h"

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

    SHOWDEBUG("Setting up LongJump\n");

    if (setjmp(png_jmpbuf(png_ptr)) == 0) {
        SHOWDEBUG("LongJump set\n");
    } else {
        SHOWDEBUG("We got a LongJump, destroying read struct\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return;
    }

    // The call to png_read_info() gives us all of the information from the
    // PNG file before the first IDAT (image data chunk). REQUIRED.
    SHOWDEBUG("Reading info\n");
    png_read_info(png_ptr, info_ptr);

    SHOWDEBUG("\nParsing image info\n");
    png_uint_32 width, height;
    int bit_depth, color_type, interlace_type;
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, NULL, NULL);
    SHOWDEBUG("PNG info: width: %d, height: %d, bit_depth: %d\n", width, height, bit_depth);
    SHOWDEBUG("Initialize display\n");

    /* LCD Init */
    if (LCDInitialize(HORIZONTAL) != 0) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return;
    }

    LCDClear(BLACK);

    png_bytep row_pointers = NULL;
    int col, row;
    int maxCol = width > LCD.WIDTH ? LCD.WIDTH : width;     // won't print outside display.
    int maxRow = height > LCD.HEIGHT ? LCD.HEIGHT : height; // won't print outside display.

    // ####### LCDDisplayTexture #######
    LCDSetDisplayArea(0, 0, maxCol, maxRow);
    DigitalWrite(LCD_DC_PIN, 1);
    DigitalWrite(LCD_CS_PIN, 0);

    int num_palette = 0;
    png_colorp palette = NULL;

    if (color_type == PNG_COLOR_TYPE_PALETTE) {
        png_get_PLTE(png_ptr, info_ptr, &palette, &num_palette);
    }

    for (row = 0; row < maxRow; row++) {
        row_pointers = (png_bytep)png_malloc(png_ptr, png_get_rowbytes(png_ptr, info_ptr));
        png_read_rows(png_ptr, &row_pointers, NULL, 1);

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

        png_free(png_ptr, row_pointers);
        row_pointers = NULL;
    }

    DigitalWrite(LCD_CS_PIN, 1);
    DriverSendCommand(0x29);
    // ####### LCDDisplayTexture #######

    /* Turn backlight on */
    DriverGPIOMode(LCD_BL_PIN, GPIO_OUT);
    DigitalWrite(LCD_CS_PIN, 1);
    DigitalWrite(LCD_DC_PIN, 0);
    DigitalWrite(LCD_BL_PIN, 1);

    SHOWDEBUG("Done! Destroying read struct\n");
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
}

#endif