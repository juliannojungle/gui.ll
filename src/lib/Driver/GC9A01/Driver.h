#ifndef DRIVER_H
#define DRIVER_H

/* Public interface of the GC9A01 low-level driver. Definitions live in Driver.c. */
#include "Types.h"

UINT8 DriverInitialize(void);
void DriverSetBacklightBrightness(UINT8 brightness);
void DriverHardwareReset(void);
void DriverSendCommand(UINT8 Reg);
void DriverSendData8Bit(UINT8 Data);
void DriverSendCommandData8Bit(UINT8 command, UINT8 data[], int dataSize);
void DriverSendData16Bit(UINT16 Data);

#endif /* DRIVER_H */
