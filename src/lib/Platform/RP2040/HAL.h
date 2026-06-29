#ifndef HAL_H
#define HAL_H

/* Public interface of the RP2040 HAL. Definitions live in HAL.c.
 * Pulls in the Pico SDK headers so the SDK constants used by the
 * platform-agnostic Driver.c (GPIO_IN/GPIO_OUT/GPIO_FUNC_SPI/
 * GPIO_FUNC_PWM/PWM_CHAN_B) and the `UINT32` type are visible. */
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "Types.h"
#include "HALConfig.h"

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
