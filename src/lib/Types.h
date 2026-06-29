#ifndef TYPES_H
#define TYPES_H

/* Shared, platform-agnostic scalar aliases used across the GUI/driver/LCD code.
 * Previously these were #define macros inside each platform's HAL.c and only
 * worked because of the unity build (HAL.c was always included first). With
 * separate compilation they live here so every translation unit can see them. */
#include <stdint.h>

typedef uint8_t  UINT8; // unsigned char
typedef uint16_t UINT16; // unsigned short int
typedef uint32_t UINT32; // unsigned int

#endif /* TYPES_H */
