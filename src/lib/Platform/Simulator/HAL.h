#ifndef HAL_H
#define HAL_H

#include <stdbool.h>
#include "Types.h"
#include "HALConfig.h"

#define GPIO_FUNC_SPI  1
#define GPIO_IN        0
#define GPIO_OUT       1
#define GPIO_FUNC_PWM  4
#define PWM_CHAN_B     1

void DigitalWrite(UINT32 pin, UINT8 value);
UINT8 DigitalRead(UINT32 pin);
void SPIWriteByte(UINT8 value);
void SPIWriteNByte(UINT8 pData[], UINT32 len);
void GPIOInit(UINT32 pin);
void GPIOSetDir(UINT32 pin, UINT32 mode);
void GPIOPullUp(UINT32 pin);
void Delay(UINT32 milliseconds);
void STDIOInitAll(void);
void SPIInit(UINT32 speed);
void GPIOSetFunction(UINT32 pin, UINT32 function);
UINT32 PWMGPIOToSliceNum(UINT32 pin);
void PWMSetWrap(UINT32 slice, UINT32 value);
void PWMSetChannelLevel(UINT32 slice, UINT32 channel, UINT16 level);
void PWMSetClockDivider(UINT32 slice, float divider);
void PWMSetEnabled(UINT32 slice, bool enable);

#endif /* HAL_H */
