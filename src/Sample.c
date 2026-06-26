#include "HAL.h"
#include "RTC.h"
#include "LCDSetup.h"
#include "LCDRenderer.h"
#include "FileHelper.h"
#include "Canvas.h"
#include "fonts.h"
#include <stdlib.h>
#include <stdio.h>

void app_entry(void) {
    STDIOInitAll();
    RTCInitialize();
    LCDInitialize();
    LCDClear(BLACK);
    // Delay(3000); // give us time to start serial monitor

    UDOUBLE imageSize = LCD.HEIGHT * LCD.WIDTH * 2;
    UWORD *texture = (UWORD *) malloc(imageSize);
    if (texture == NULL)
        exit(EXIT_FAILURE);
    CanvasNewImage((UBYTE *)texture, LCD.WIDTH, LCD.HEIGHT, ROTATE_0);
    CanvasSetScale(65);

    FIL file;
    if (MountSdCard() && SelectActiveDrive() && OpenFile(&file, "01.png")) {
        CanvasDrawPng(&file);
        CloseFile(&file);
    }
    UnMountSdCard();

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
