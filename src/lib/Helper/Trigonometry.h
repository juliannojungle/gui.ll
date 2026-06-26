#ifndef __TRIGONOMETRY_H
#define __TRIGONOMETRY_H

#include "Types.h"

/* Fixed-point integer trigonometry. Values are read from precomputed Q16.16 LUTs
 * (no runtime <math.h>), so identical inputs yield bit-for-bit identical results on
 * every target regardless of its floating-point hardware. A value v represents the
 * real number v / TRIG_Q16_SCALE. The angle is in whole degrees, normalized mod 360. */
#define TRIG_Q16_SCALE (1 << 16)

int32_t TrigCosQ16(UWORD degrees);
int32_t TrigSinQ16(UWORD degrees);

#endif /* __TRIGONOMETRY_H */
