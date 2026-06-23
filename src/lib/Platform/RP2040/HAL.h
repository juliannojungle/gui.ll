#ifndef HAL_H
#define HAL_H

/* Public interface of the RP2040 HAL. Definitions live in HAL.c.
 * Pulls in the Pico SDK headers so the SDK constants used by the
 * platform-agnostic Driver.c (GPIO_IN/GPIO_OUT/GPIO_FUNC_SPI/
 * GPIO_FUNC_PWM/PWM_CHAN_B) and the `uint` type are visible. */
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "Types.h"
#include "HALConfig.h"

void DigitalWrite(uint pin, UBYTE value);
UBYTE DigitalRead(uint pin);
void SPIWriteByte(UBYTE value);
void SPIWriteNByte(UBYTE pData[], UDOUBLE len);
void GPIOInit(uint pin);
void GPIOSetDir(uint pin, uint mode);
void GPIOPullUp(uint pin);
void Delay(uint milliseconds);
void STDIOInitAll(void);
void SPIInit(uint speed);
void GPIOSetFunction(uint pin, uint function);
uint PWMGPIOToSliceNum(uint pin);
void PWMSetWrap(uint slice, uint value);
void PWMSetChannelLevel(uint slice, uint channel, UWORD level);
void PWMSetClockDivider(uint slice, float divider);
void PWMSetEnabled(uint slice, bool enable);

#endif /* HAL_H */
