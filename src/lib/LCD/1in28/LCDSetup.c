#ifndef __LCD_SETUP_C
#define __LCD_SETUP_C

#include <stdlib.h>
#include "LCDSetup.h"
#include "HAL.h"
#include "Driver.h"

LCD_ATTRIBUTES LCD;

static void LCDInitRegister(void)
{
    DriverSendCommand(0x11); /* Sleep mode OFF */
    Delay(120); /* Delay 120ms */
    DriverSendCommand(0xEF); /* Inter register enable 2 */
    DriverSendCommandData8Bit(0xEB, (UINT8[]){0x14}, 1);
    DriverSendCommand(0xFE); /* set inter_command HIGH: Inter register enable 1 */
    DriverSendCommand(0xEF); /* set inter_command HIGH: Inter register enable 2 */
    DriverSendCommandData8Bit(0xEB, (UINT8[]){0x14}, 1);
    DriverSendCommandData8Bit(0x84, (UINT8[]){0x40}, 1);
    DriverSendCommandData8Bit(0x85, (UINT8[]){0xFF}, 1);
    DriverSendCommandData8Bit(0x86, (UINT8[]){0xFF}, 1);
    DriverSendCommandData8Bit(0x87, (UINT8[]){0xFF}, 1);
    DriverSendCommandData8Bit(0x88, (UINT8[]){0x0A}, 1);
    DriverSendCommandData8Bit(0x89, (UINT8[]){0x21}, 1);
    DriverSendCommandData8Bit(0x8A, (UINT8[]){0x00}, 1);
    DriverSendCommandData8Bit(0x8B, (UINT8[]){0x80}, 1);
    DriverSendCommandData8Bit(0x8C, (UINT8[]){0x01}, 1);
    DriverSendCommandData8Bit(0x8D, (UINT8[]){0x01}, 1);
    DriverSendCommandData8Bit(0x8E, (UINT8[]){0xFF}, 1);
    DriverSendCommandData8Bit(0x8F, (UINT8[]){0xFF}, 1);
    DriverSendCommandData8Bit(0xB6, (UINT8[]){0x00, 0x00}, 2); /* Display function control */
    // DriverSendCommandData8Bit(0x36, (UINT8[]){0x48}, 1); /* Memory access control - defined on LCDSetAttributes */
    DriverSendCommandData8Bit(0x3A, (UINT8[]){0x05}, 1); /* Pixel format */
    DriverSendCommandData8Bit(0x90, (UINT8[]){0x08,0x08,0x08,0x08}, 4);
    DriverSendCommandData8Bit(0xBD, (UINT8[]){0x06}, 1);
    DriverSendCommandData8Bit(0xBC, (UINT8[]){0x00}, 1);
    DriverSendCommandData8Bit(0xFF, (UINT8[]){0x60, 0x01, 0x04}, 3);
    DriverSendCommandData8Bit(0xC3, (UINT8[]){0x13}, 1); /* Voltage regulator 1a */
    DriverSendCommandData8Bit(0xC4, (UINT8[]){0x13}, 1); /* Voltage regulator 1b */
    DriverSendCommandData8Bit(0xC9, (UINT8[]){0x22}, 1); /* Voltage regulator 2a */
    DriverSendCommandData8Bit(0xBE, (UINT8[]){0x11}, 1);
    DriverSendCommandData8Bit(0xE1, (UINT8[]){0x10, 0x0E}, 2);
    DriverSendCommandData8Bit(0xDF, (UINT8[]){0x21, 0x0c, 0x02}, 3);
    DriverSendCommandData8Bit(0xF0, (UINT8[]){0x45, 0x09, 0x08, 0x08, 0x26, 0x2A}, 6); /* Set gamma1 */
    DriverSendCommandData8Bit(0xF1, (UINT8[]){0x43, 0x70, 0x72, 0x36, 0x37, 0x6F}, 6); /* Set gamma2 */
    DriverSendCommandData8Bit(0xF2, (UINT8[]){0x45, 0x09, 0x08, 0x08, 0x26, 0x2A}, 6); /* Set gamma3 */
    DriverSendCommandData8Bit(0xF3, (UINT8[]){0x43, 0x70, 0x72, 0x36, 0x37, 0x6F}, 6); /* Set gamma4 */
    DriverSendCommandData8Bit(0xED, (UINT8[]){0x1B, 0x0B}, 2);
    DriverSendCommandData8Bit(0xAE, (UINT8[]){0x77}, 1);
    DriverSendCommandData8Bit(0xCD, (UINT8[]){0x63}, 1);
    DriverSendCommandData8Bit(0x70, (UINT8[]){0x07, 0x07, 0x04, 0x0E, 0x0F, 0x09, 0x07, 0x08, 0x03}, 9);
    DriverSendCommandData8Bit(0xE8, (UINT8[]){0x34}, 1); /* Frame rate */
    DriverSendCommandData8Bit(0x62, (UINT8[]){0x18, 0x0D, 0x71, 0xED, 0x70, 0x70, 0x18, 0x0F, 0x71, 0xEF, 0x70, 0x70}, 12);
    DriverSendCommandData8Bit(0x63, (UINT8[]){0x18, 0x11, 0x71, 0xF1, 0x70, 0x70, 0x18, 0x13, 0x71, 0xF3, 0x70, 0x70}, 12);
    DriverSendCommandData8Bit(0x64, (UINT8[]){0x28, 0x29, 0xF1, 0x01, 0xF1, 0x00, 0x07}, 7);
    DriverSendCommandData8Bit(0x66, (UINT8[]){0x3C, 0x00, 0xCD, 0x67, 0x45, 0x45, 0x10, 0x00, 0x00, 0x00}, 10);
    DriverSendCommandData8Bit(0x67, (UINT8[]){0x00, 0x3C, 0x00, 0x00, 0x00, 0x01, 0x54, 0x10, 0x32, 0x98}, 10);
    DriverSendCommandData8Bit(0x74, (UINT8[]){0x10, 0x85, 0x80, 0x00, 0x00, 0x4E, 0x00}, 7);
    DriverSendCommandData8Bit(0x98, (UINT8[]){0x3e, 0x07}, 2);
    DriverSendCommand(0x35); /* Tearing effect ON */
    DriverSendCommand(0x21); /* Display color inversion ON */
    DriverSendCommand(0x11); /* Sleep mode OFF */
    Delay(12); /* Delay 12ms */
    DriverSendCommand(0x29); /* Display ON */
    Delay(20); /* Delay 20ms */
}

static void LCDSetAttributes(UINT8 scanDirection)
{
    LCD.SCAN_DIR = scanDirection;
    UINT8 memoryAccessReg = 0x00;

    if(scanDirection == HORIZONTAL) {
        LCD.HEIGHT = LCD_WIDTH;
        LCD.WIDTH = LCD_HEIGHT;
        memoryAccessReg = 0x48;
    } else {
        LCD.HEIGHT = LCD_HEIGHT;
        LCD.WIDTH = LCD_WIDTH;
        memoryAccessReg = 0x24;
    }

    /* set scan and refresh direction + color channel order RGB (not using BGR here)*/
    DriverSendCommandData8Bit(0x36, (UINT8[]){memoryAccessReg}, 1);
}

int LCDInitialize()
{
    int initResult = DriverInitialize();

    if (initResult != EXIT_SUCCESS)
        return initResult;

    DriverHardwareReset();
    LCDSetAttributes(HORIZONTAL);
    LCDInitRegister();
    return EXIT_SUCCESS;
}

#endif