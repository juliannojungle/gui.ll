#ifndef DRIVER_H
#define DRIVER_H

/* Public interface of the GC9A01 low-level driver. Definitions live in Driver.c. */
#include "Types.h"

UBYTE DriverInitialize(void);
void DriverSetBacklightBrightness(UINT brightness);
void DriverHardwareReset(void);
void DriverSendCommand(UBYTE Reg);
void DriverSendData8Bit(UBYTE Data);
void DriverSendCommandData8Bit(UBYTE command, UBYTE data[], int dataSize);
void DriverSendData16Bit(UWORD Data);

#endif /* DRIVER_H */
