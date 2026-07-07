#include "HAL.h"
#include "LCDRenderer.h"
#include <stdlib.h>
#include <SDL2/SDL.h>

void Delay(UINT32 milliseconds) {
    UINT32 start = SDL_GetTicks();
    SDL_Event event;
    while (SDL_GetTicks() - start < milliseconds) {
        if (LCDRenderShouldClose())
            exit(0);
        SDL_Delay(1);
    }
}

void DigitalWrite(UINT32 pin, UINT8 value) { (void)pin; (void)value; }
UINT8 DigitalRead(UINT32 pin) { (void)pin; return 0; }
void SPIWriteByte(UINT8 value) { (void)value; }
void SPIWriteNByte(UINT8 pData[], UINT32 len) { (void)pData; (void)len; }
void GPIOInit(UINT32 pin) { (void)pin; }
void GPIOSetDir(UINT32 pin, UINT32 mode) { (void)pin; (void)mode; }
void GPIOPullUp(UINT32 pin) { (void)pin; }
void STDIOInitAll(void) {}
void SPIInit(UINT32 speed) { (void)speed; }
void GPIOSetFunction(UINT32 pin, UINT32 function) { (void)pin; (void)function; }
UINT32 PWMGPIOToSliceNum(UINT32 pin) { (void)pin; return 0; }
void PWMSetWrap(UINT32 slice, UINT32 value) { (void)slice; (void)value; }
void PWMSetChannelLevel(UINT32 slice, UINT32 channel, UINT16 level) { (void)slice; (void)channel; (void)level; }
void PWMSetClockDivider(UINT32 slice, float divider) { (void)slice; (void)divider; }
void PWMSetEnabled(UINT32 slice, bool enable) { (void)slice; (void)enable; }
