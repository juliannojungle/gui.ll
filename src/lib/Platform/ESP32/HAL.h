#ifndef HAL_H
#define HAL_H

/* Public interface of the ESP32 HAL. Definitions live in HAL.c.
 * The Pico-SDK-compatible constants below let the platform-agnostic Driver.c
 * compile unchanged on the ESP32 target. */
#include <stdint.h>
#include <stdbool.h>
#include "Types.h"
#include "HALConfig.h"

/* Compatibility defines for Driver.c (mirror the RP2040 Pico SDK constants) */
#define GPIO_IN         0
#define GPIO_OUT        1
#define GPIO_FUNC_SPI   0  // No-op on ESP32 (SPI pins configured in SPIInit)
#define GPIO_FUNC_PWM   0  // No-op on ESP32 (PWM configured via LEDC)
#define PWM_CHAN_B      1

void DigitalWrite(uint32_t pin, UINT8 value);
UINT8 DigitalRead(uint32_t pin);
void SPIWriteByte(UINT8 value);
void SPIWriteNByte(UINT8 pData[], UINT32 len);
void GPIOInit(uint32_t pin);
void GPIOSetDir(uint32_t pin, uint32_t mode);
void GPIOPullUp(uint32_t pin);
void Delay(uint32_t milliseconds);
void STDIOInitAll(void);
void SPIInit(uint32_t speed);
void GPIOSetFunction(uint32_t pin, uint32_t function);
uint32_t PWMGPIOToSliceNum(uint32_t pin);
void PWMSetWrap(uint32_t slice, uint32_t value);
void PWMSetChannelLevel(uint32_t slice, uint32_t channel, UINT16 level);
void PWMSetClockDivider(uint32_t slice, float divider);
void PWMSetEnabled(uint32_t slice, bool enable);

#endif /* HAL_H */
