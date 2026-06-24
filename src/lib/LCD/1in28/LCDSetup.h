#ifndef __LCD_SETUP_H
#define __LCD_SETUP_H

#include "Types.h"

#define LCD_HEIGHT 240
#define LCD_WIDTH 240

#define HORIZONTAL 0
#define VERTICAL 1

typedef struct{
    UWORD WIDTH;
    UWORD HEIGHT;
    UBYTE SCAN_DIR;
}LCD_ATTRIBUTES;

/* The single LCD attribute instance is defined in LCDSetup.c. */
extern LCD_ATTRIBUTES LCD;

/* Public LCD API — definitions in LCDSetup.c */
int LCDInitialize();

#endif