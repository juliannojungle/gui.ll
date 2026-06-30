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

    UINT32 imageSize = LCD.HEIGHT * LCD.WIDTH * 2;
    UINT16 *texture = (UINT16 *) malloc(imageSize);
    if (texture == NULL)
        exit(EXIT_FAILURE);
    CanvasNewTexture((UINT8 *)texture, LCD.WIDTH, LCD.HEIGHT, ROTATE_0);
    CanvasSetScale(65);

    FIL file;
    if (MountSdCard() && SelectActiveDrive() && OpenFile(&file, "01.png")) {
        CanvasDrawPng(&file);
        CloseFile(&file);
    }
    UnMountSdCard();

    CanvasDrawText(52, 112, "0123456789", &Font20, RGB_COLOR(252, 82, 0), TRANSPARENT);
    CanvasDrawCurvedText("ABCDEFGHIJKLMNOPQRSTUVW", 120, 120, 102, 181, TEXT_ORIENTATION_INWARDS, &Font20, RGB_COLOR(252, 82, 0), TRANSPARENT);
    CanvasDrawCurvedText("abcdefghijklmnopqrstuvw", 120, 120, 105, 172, TEXT_ORIENTATION_OUTWARDS, &Font20, RGB_COLOR(252, 82, 0), TRANSPARENT);
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
