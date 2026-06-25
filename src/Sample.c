#include "HAL.h"
#include "RTC.h" // rtc for file's timestamp.

#include "LCDSetup.h"
#include "LCDRenderer.h"
#include "Canvas.h"
#include "fonts.h"
#include <stdlib.h>
#include <stdio.h>

#include "FileHelper.h"

void app_entry(void) {
    STDIOInitAll();
    time_init();
    // Delay(3000); // give us time to start serial monitor

    /* LCD Init */
    LCDInitialize();
    LCDClear(BLACK);

    FIL file;
    if (MountSdCard() && SelectActiveDrive() && OpenFile(&file, "01.png")) {
        LCDRenderPng(&file);
        CloseFile(&file);
    }
    UnMountSdCard();

    UDOUBLE imageSize = LCD.HEIGHT * LCD.WIDTH * 2;
    UWORD *texture = (UWORD *) malloc(imageSize);
    if (texture == NULL)
        exit(EXIT_FAILURE);
    CanvasNewImage((UBYTE *)texture, LCD.WIDTH, LCD.HEIGHT, ROTATE_0, BLACK);
    CanvasSetScale(65);
    // CanvasClear(BLACK);
    CanvasDrawText(30, 110, "Hello, World!", &Font20, WHITE, BLACK);
    LCDRenderTexture(texture);

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
