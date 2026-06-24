#include "HAL.h"
#include "RTC.h" // rtc for file's timestamp.

#include "LCDSetup.h"
#include "LCDRenderer.h"
#include "Canvas.h"
// #include "fonts.h"
// #include <stdlib.h>
// #include <stdio.h>

#include "FileHelper.h"
#include "PNGHelper.h"

void app_entry(void) {
    STDIOInitAll();
    time_init();
    // Delay(3000); // give us time to start serial monitor

    /* LCD Init */
    LCDInitialize();
    LCDClear(BLACK);

/*
    UDOUBLE imageSize = LCD.HEIGHT * LCD.WIDTH * 2;
    UWORD *texture = (UWORD *) malloc(imageSize); // Allocate framebuffer

    if (texture == NULL) {
        exit(EXIT_FAILURE);
    }

    CanvasNewImage((UBYTE *)texture, LCD.WIDTH, LCD.HEIGHT, ROTATE_0, BLACK);
    CanvasSetScale(65);
    int count = 0;

    while (true) {
        CanvasClear(WHITE);
        CanvasDrawText(count, 130, "Hello, World!", &Font20, WHITE, BLACK);
        LCDDisplayTexture(texture);

        count = (count < 256) ? count + 1 : 0;
        printf("%d\n", count);
        Delay(100);
    }
*/

    FIL file;

    if (MountSdCard()
        && SelectActiveDrive()
        && OpenFile(&file, "01.png")) {
        DisplayPng(&file);
        CloseFile(&file);
    }

    UnMountSdCard();

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
