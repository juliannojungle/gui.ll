#include "HAL.h"
#include "RTC.h"
#include "LCDSetup.h"
#include "LCDRenderer.h"
#include "FileSystem.h"
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

    Texture texture = CanvasNewTexture(LCD.WIDTH, LCD.HEIGHT);
    if (texture.Data == NULL)
        exit(EXIT_FAILURE);

    FIL file;
    if (MountSdCard() && SelectActiveDrive() && OpenFile(&file, "01.png")) {
        CanvasDrawPng(texture, &file);
        CloseFile(&file);
    }
    UnMountSdCard();

    CanvasDrawText(texture, 52, 112, "0123456789", &Font20, RGB_COLOR(252, 82, 0), TRANSPARENT);
    CanvasDrawCurvedText(texture, "ABCDEFGHIJKLMNOPQRSTUVW", 120, 120, 102, 181, TEXT_ORIENTATION_INWARDS, &Font20, RGB_COLOR(252, 82, 0), TRANSPARENT);
    CanvasDrawCurvedText(texture, "abcdefghijklmnopqrstuvw", 120, 120, 105, 172, TEXT_ORIENTATION_OUTWARDS, &Font20, RGB_COLOR(252, 82, 0), TRANSPARENT);
    LCDRenderTexture(texture.Data);

    int cont = 0;
    while(true) {
        CanvasDrawNum(texture, 100, 100, cont, &Font20, 0, BLACK, WHITE);
        LCDRenderTexture(texture.Data);
        if (cont < 1000)
          cont++;
        else
          cont = 0;
        Delay(1000);
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
