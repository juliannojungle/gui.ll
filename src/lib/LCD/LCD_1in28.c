/*****************************************************************************
* | File        :   LCD_1in28.c
* | Author      :   Waveshare team, Julianno F. C. Silva (@juliannojungle)
* | Function    :   Hardware underlying interface
* | Info        :
*                Used to shield the underlying layers of each master
*                and enhance portability
*----------------
* | This version:   V1.0
* | Date        :   2023-03-26
* | Info        :   Basic version
*
******************************************************************************/
#include "LCD_1in28.h"
#include "HALConfig.h"

LCD_ATTRIBUTES LCD;

static void LCDReset(void)
{
    DigitalWrite(LCD_RST_PIN, 1);
    Delay(100);
    DigitalWrite(LCD_RST_PIN, 0);
    Delay(100);
    DigitalWrite(LCD_RST_PIN, 1);
    Delay(100);
}

static void LCDSendCommand(UBYTE Reg)
{
    DigitalWrite(LCD_DC_PIN, 0);
    DigitalWrite(LCD_CS_PIN, 0);
    SPIWriteByte(Reg);
    DigitalWrite(LCD_CS_PIN, 1);
}

static void LCDSendData8Bit(UBYTE Data)
{
    DigitalWrite(LCD_DC_PIN, 1);
    DigitalWrite(LCD_CS_PIN, 0);
    SPIWriteByte(Data);
    DigitalWrite(LCD_CS_PIN, 1);
}

static void LCDSendCommandData8Bit(UBYTE command, UBYTE data[], int dataSize)
{
    LCDSendCommand(command);
    for (int i = 0; i < dataSize; i++) {
        LCDSendData8Bit(data[i]);
    }
}

static void LCDSendData16Bit(UWORD Data)
{
    DigitalWrite(LCD_DC_PIN, 1);
    DigitalWrite(LCD_CS_PIN, 0);
    SPIWriteByte((Data >> 8) & 0xFF);
    SPIWriteByte(Data & 0xFF);
    DigitalWrite(LCD_CS_PIN, 1);
}

static void LCDInitRegister(void)
{
    LCDSendCommand(0x11); /* Sleep mode OFF */
    Delay(120); /* Delay 120ms */
    LCDSendCommand(0xEF); /* Inter register enable 2 */
    LCDSendCommandData8Bit(0xEB, (UBYTE[]){0x14}, 1);
    LCDSendCommand(0xFE); /* set inter_command HIGH: Inter register enable 1 */
    LCDSendCommand(0xEF); /* set inter_command HIGH: Inter register enable 2 */
    LCDSendCommandData8Bit(0xEB, (UBYTE[]){0x14}, 1);
    LCDSendCommandData8Bit(0x84, (UBYTE[]){0x40}, 1);
    LCDSendCommandData8Bit(0x85, (UBYTE[]){0xFF}, 1);
    LCDSendCommandData8Bit(0x86, (UBYTE[]){0xFF}, 1);
    LCDSendCommandData8Bit(0x87, (UBYTE[]){0xFF}, 1);
    LCDSendCommandData8Bit(0x88, (UBYTE[]){0x0A}, 1);
    LCDSendCommandData8Bit(0x89, (UBYTE[]){0x21}, 1);
    LCDSendCommandData8Bit(0x8A, (UBYTE[]){0x00}, 1);
    LCDSendCommandData8Bit(0x8B, (UBYTE[]){0x80}, 1);
    LCDSendCommandData8Bit(0x8C, (UBYTE[]){0x01}, 1);
    LCDSendCommandData8Bit(0x8D, (UBYTE[]){0x01}, 1);
    LCDSendCommandData8Bit(0x8E, (UBYTE[]){0xFF}, 1);
    LCDSendCommandData8Bit(0x8F, (UBYTE[]){0xFF}, 1);
    LCDSendCommandData8Bit(0xB6, (UBYTE[]){0x00, 0x00}, 2); /* Display function control */
    // LCDSendCommandData8Bit(0x36, (UBYTE[]){0x48}, 1); /* Memory access control - defined on LCDSetAttributes */
    LCDSendCommandData8Bit(0x3A, (UBYTE[]){0x05}, 1); /* Pixel format */
    LCDSendCommandData8Bit(0x90, (UBYTE[]){0x08,0x08,0x08,0x08}, 4);
    LCDSendCommandData8Bit(0xBD, (UBYTE[]){0x06}, 1);
    LCDSendCommandData8Bit(0xBC, (UBYTE[]){0x00}, 1);
    LCDSendCommandData8Bit(0xFF, (UBYTE[]){0x60, 0x01, 0x04}, 3);
    LCDSendCommandData8Bit(0xC3, (UBYTE[]){0x13}, 1); /* Voltage regulator 1a */
    LCDSendCommandData8Bit(0xC4, (UBYTE[]){0x13}, 1); /* Voltage regulator 1b */
    LCDSendCommandData8Bit(0xC9, (UBYTE[]){0x22}, 1); /* Voltage regulator 2a */
    LCDSendCommandData8Bit(0xBE, (UBYTE[]){0x11}, 1);
    LCDSendCommandData8Bit(0xE1, (UBYTE[]){0x10, 0x0E}, 2);
    LCDSendCommandData8Bit(0xDF, (UBYTE[]){0x21, 0x0c, 0x02}, 3);
    LCDSendCommandData8Bit(0xF0, (UBYTE[]){0x45, 0x09, 0x08, 0x08, 0x26, 0x2A}, 6); /* Set gamma1 */
    LCDSendCommandData8Bit(0xF1, (UBYTE[]){0x43, 0x70, 0x72, 0x36, 0x37, 0x6F}, 6); /* Set gamma2 */
    LCDSendCommandData8Bit(0xF2, (UBYTE[]){0x45, 0x09, 0x08, 0x08, 0x26, 0x2A}, 6); /* Set gamma3 */
    LCDSendCommandData8Bit(0xF3, (UBYTE[]){0x43, 0x70, 0x72, 0x36, 0x37, 0x6F}, 6); /* Set gamma4 */
    LCDSendCommandData8Bit(0xED, (UBYTE[]){0x1B, 0x0B}, 2);
    LCDSendCommandData8Bit(0xAE, (UBYTE[]){0x77}, 1);
    LCDSendCommandData8Bit(0xCD, (UBYTE[]){0x63}, 1);
    LCDSendCommandData8Bit(0x70, (UBYTE[]){0x07, 0x07, 0x04, 0x0E, 0x0F, 0x09, 0x07, 0x08, 0x03}, 9);
    LCDSendCommandData8Bit(0xE8, (UBYTE[]){0x34}, 1); /* Frame rate */
    LCDSendCommandData8Bit(0x62, (UBYTE[]){0x18, 0x0D, 0x71, 0xED, 0x70, 0x70, 0x18, 0x0F, 0x71, 0xEF, 0x70, 0x70}, 12);
    LCDSendCommandData8Bit(0x63, (UBYTE[]){0x18, 0x11, 0x71, 0xF1, 0x70, 0x70, 0x18, 0x13, 0x71, 0xF3, 0x70, 0x70}, 12);
    LCDSendCommandData8Bit(0x64, (UBYTE[]){0x28, 0x29, 0xF1, 0x01, 0xF1, 0x00, 0x07}, 7);
    LCDSendCommandData8Bit(0x66, (UBYTE[]){0x3C, 0x00, 0xCD, 0x67, 0x45, 0x45, 0x10, 0x00, 0x00, 0x00}, 10);
    LCDSendCommandData8Bit(0x67, (UBYTE[]){0x00, 0x3C, 0x00, 0x00, 0x00, 0x01, 0x54, 0x10, 0x32, 0x98}, 10);
    LCDSendCommandData8Bit(0x74, (UBYTE[]){0x10, 0x85, 0x80, 0x00, 0x00, 0x4E, 0x00}, 7);
    LCDSendCommandData8Bit(0x98, (UBYTE[]){0x3e, 0x07}, 2);
    LCDSendCommand(0x35); /* Tearing effect ON */
    LCDSendCommand(0x21); /* Display color inversion ON */
    LCDSendCommand(0x11); /* Sleep mode OFF */
    Delay(12); /* Delay 12ms */
    LCDSendCommand(0x29); /* Display ON */
    Delay(20); /* Delay 20ms */
}

static void LCDSetAttributes(UBYTE Scan_dir)
{
    //Get the screen scan direction
    LCD.SCAN_DIR = Scan_dir;
    UBYTE MemoryAccessReg = 0x00;

    //Get GRAM and LCD width and height
    if(Scan_dir == HORIZONTAL) {
        LCD.HEIGHT = LCD_1IN28_WIDTH;
        LCD.WIDTH = LCD_1IN28_HEIGHT;
        MemoryAccessReg = 0x48;
    } else {
        LCD.HEIGHT = LCD_1IN28_HEIGHT;
        LCD.WIDTH = LCD_1IN28_WIDTH;
        MemoryAccessReg = 0x24;
    }

    // Set the read / write scan direction of the frame memory
    //MX, MY, RGB mode; 0x08 set RGB
    LCDSendCommandData8Bit(0x36, (UBYTE[]){MemoryAccessReg}, 1);
}

int LCDInitialize(UBYTE Scan_dir)
{
    int driverResult = DriverInit();

    if (driverResult != 0)
        return driverResult;

    DriverSetPWM(90);
    LCDReset(); //Hardware reset
    LCDSetAttributes(Scan_dir); //Set the resolution and scanning method of the screen
    LCDInitRegister(); //Set the initialization register
    return 0;
}

void LCDSetDisplayArea(UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD Yend)
{
    LCDSendCommandData8Bit(0x2A, (UBYTE[]){0x00, Xstart, 0x00, Xend-1}, 4); //set the X coordinates
    LCDSendCommandData8Bit(0x2B, (UBYTE[]){0x00, Ystart, 0x00, Yend-1}, 4); //set the Y coordinates
    LCDSendCommand(0X2C);
    // printf("%d %d\r\n",x,y);
}

void LCDClear(UWORD FillColor)
{
    UWORD j;
    UWORD Image[LCD.WIDTH*LCD.HEIGHT];
    FillColor = ((FillColor<<8)&0xff00)|(FillColor>>8);

    for (j = 0; j < LCD.HEIGHT*LCD.WIDTH; j++) {
        Image[j] = FillColor;
    }

    LCDSetDisplayArea(0, 0, LCD.WIDTH, LCD.HEIGHT);
    DigitalWrite(LCD_DC_PIN, 1);
    DigitalWrite(LCD_CS_PIN, 0);

    for (j = 0; j < LCD.HEIGHT; j++) {
        SPIWriteNByte((uint8_t *)&Image[j*LCD.WIDTH], LCD.WIDTH*2);
    }

    DigitalWrite(LCD_CS_PIN, 1);
}

void LCDDisplayImage(UWORD *Image) /* uses full display area available in LCD */
{
    UWORD j;
    LCDSetDisplayArea(0, 0, LCD.WIDTH, LCD.HEIGHT);
    DigitalWrite(LCD_DC_PIN, 1);
    DigitalWrite(LCD_CS_PIN, 0);

    for (j = 0; j < LCD.HEIGHT; j++) {
        SPIWriteNByte((uint8_t *)&Image[j*LCD.WIDTH], LCD.WIDTH*2);
    }

    DigitalWrite(LCD_CS_PIN, 1);
    LCDSendCommand(0x29);
}

void LCDDisplayImageInArea(UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD Yend, UWORD *Image)
{
    UDOUBLE Addr = 0;
    UWORD j;
    LCDSetDisplayArea(Xstart, Ystart, Xend , Yend);
    DigitalWrite(LCD_DC_PIN, 1);
    DigitalWrite(LCD_CS_PIN, 0);

    for (j = Ystart; j < Yend - 1; j++) {
        Addr = Xstart + j * LCD.WIDTH ;
        SPIWriteNByte((uint8_t *)&Image[Addr], (Xend-Xstart)*2);
    }

    DigitalWrite(LCD_CS_PIN, 1);
}

void LCDDisplayImagePoint(UWORD X, UWORD Y, UWORD Color)
{
    LCDSetDisplayArea(X,Y,X,Y);
    LCDSendData16Bit(Color);
}
