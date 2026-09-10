#ifndef QR_H
#define QR_H

#include <stdbool.h>
#include <stdint.h>

#define QR_MAX_URL_LENGTH 80

#define QR_VERSION_4_SIZE 33
#define QR_VERSION_5_SIZE 37
#define QR_MAX_SIZE QR_VERSION_5_SIZE

#define QR_BITMAP_BYTES ((QR_MAX_SIZE * QR_MAX_SIZE + 7) / 8)

typedef struct {
    uint8_t data[QR_BITMAP_BYTES];
    uint8_t size;
} QRCode;

/*
 * Generates a QR Code for the supplied URL.
 *
 * Supported:
 *   - ASCII
 *   - up to 80 characters
 *   - QR version 4 or 5
 *   - error correction level M
 *
 * Returns false if the URL is NULL or too long.
 */
bool qr_generate(QRCode *qr, const char *url);

/*
 * Returns the state of one QR module.
 *
 * true  = black
 * false = white
 */
bool qr_get_module(const QRCode *qr, uint8_t x, uint8_t y);

#endif