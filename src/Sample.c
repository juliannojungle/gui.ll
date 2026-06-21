#include "HAL.c"
#include "RTC.h" // rtc for file's timestamp.

// #include "LCD_1in28.c"
// #include "font20.c"
// #include "GUI_Paint.c"

#include "FileHelper.c"
#include "PNGHelper.c"

void app_entry(void) {
    STDIOInitAll();
    time_init();

#if _DEBUG
    Delay(3000);
#endif

/*
    // LCD Init
    LCDInitialize(HORIZONTAL);
    LCDClear(WHITE);

    // Turn backlight on
    GPIOSetDir(LCD_BL_PIN, GPIO_OUT);
    DigitalWrite(LCD_CS_PIN, 1);
    DigitalWrite(LCD_DC_PIN, 0);
    DigitalWrite(LCD_BL_PIN, 1);

    // Allocate framebuffer
    UDOUBLE imageSize = LCD_1IN28_HEIGHT * LCD_1IN28_WIDTH * 2;
    UWORD *texture;

    if ((texture = (UWORD *) malloc(imageSize)) == NULL) {
        exit(0);
    }

    CanvasNewImage((UBYTE *)texture, LCD.WIDTH, LCD.HEIGHT, ROTATE_0, WHITE);
    CanvasSetScale(65);
    int count = 0;

    while (true) {
        CanvasClear(WHITE);
        CanvasDrawStringEN(count, count, "Hello, World!", &Font20, BLACK, WHITE);
        LCDDisplayTexture(texture);

        count = (count < 256) ? count + 1 : 0;
        // printf("%d\n", count);
        Delay(100);
    }
*/

    if (sd_get_num() > 0) {
        SdCard *sdcard = sd_get_by_num(0);
        FIL file;

        if (MountSdCard(sdcard)
            && SelectActiveDrive(sdcard)
            && OpenFile(sdcard, &file, "01.png")) {
            DisplayPng(&file);
            CloseFile(&file);
        }

        UnMountSdCard(sdcard);
    }

    while(true) {
    }
}

#ifdef ESP_PLATFORM
void app_main(void) {
    app_entry();
}
#else
int main(void) {
    app_entry();
    return 0;
}
#endif
