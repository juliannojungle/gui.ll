#include "qr.h"

#include <string.h>

/*
 * QR Code versions supported by this implementation:
 *
 * Version 4-M:
 *   33 x 33 modules
 *   100 total codewords
 *   64 data codewords
 *   36 ECC codewords
 *   2 blocks
 *   18 ECC codewords/block
 *
 * Version 5-M:
 *   37 x 37 modules
 *   134 total codewords
 *   86 data codewords
 *   48 ECC codewords
 *   2 blocks
 *   24 ECC codewords/block
 *
 * Byte-mode capacity:
 *   Version 4-M: 62 bytes
 *   Version 5-M: 84 bytes
 */

/* --------------------------------------------------------------------------
 * Bit-packed bitmap
 * -------------------------------------------------------------------------- */

static inline void bitmap_set(uint8_t *bitmap, int index, bool value) {
    uint8_t mask = (uint8_t)(1u << (index & 7));

    if (value)
        bitmap[index >> 3] |= mask;
    else
        bitmap[index >> 3] &= (uint8_t)~mask;
}

static inline bool bitmap_get(const uint8_t *bitmap, int index) { return (bitmap[index >> 3] >> (index & 7)) & 1u; }

static inline int module_index(int size, int x, int y) { return y * size + x; }

static inline void module_set(uint8_t *bitmap, int size, int x, int y, bool value) {
    bitmap_set(bitmap, module_index(size, x, y), value);
}

static inline bool module_get(const uint8_t *bitmap, int size, int x, int y) {
    return bitmap_get(bitmap, module_index(size, x, y));
}

/*
 * Set a function module and mark it as reserved.
 */
static void function_module(uint8_t *matrix, uint8_t *function, int size, int x, int y, bool dark) {
    if (x < 0 || y < 0 || x >= size || y >= size)
        return;

    module_set(matrix, size, x, y, dark);
    module_set(function, size, x, y, true);
}

/* --------------------------------------------------------------------------
 * GF(256) / Reed-Solomon
 * -------------------------------------------------------------------------- */

/*
 * QR Codes use GF(256) with primitive polynomial:
 *
 *     x^8 + x^4 + x^3 + x^2 + 1
 *
 * which corresponds to 0x11D.
 *
 * The multiplication below uses the usual xtime optimization, so we don't
 * need 512-byte log/antilog tables.
 */

static uint8_t gf_multiply(uint8_t x, uint8_t y) {
    uint8_t result = 0;

    while (y != 0) {
        if (y & 1)
            result ^= x;

        y >>= 1;

        if (x & 0x80)
            x = (uint8_t)((x << 1) ^ 0x1D);
        else
            x <<= 1;
    }

    return result;
}

static uint8_t gf_power_of_two(int exponent) {
    uint8_t value = 1;

    while (exponent-- > 0)
        value = gf_multiply(value, 2);

    return value;
}

/*
 * Builds the Reed-Solomon generator polynomial.
 *
 * generator[0] ... generator[degree]
 */
static void rs_generator(int degree, uint8_t *generator) {
    memset(generator, 0, (size_t)degree + 1);

    generator[0] = 1;

    int length = 1;

    for (int i = 0; i < degree; i++) {
        uint8_t root = gf_power_of_two(i);

        for (int j = length; j > 0; j--) {
            generator[j] ^= gf_multiply(generator[j - 1], root);
        }

        length++;
    }
}

/*
 * Computes the Reed-Solomon remainder.
 */
static void rs_remainder(
    const uint8_t *data, int data_length, const uint8_t *generator, int degree, uint8_t *remainder) {
    memset(remainder, 0, (size_t)degree);

    for (int i = 0; i < data_length; i++) {
        uint8_t factor = (uint8_t)(data[i] ^ remainder[0]);

        memmove(remainder, remainder + 1, (size_t)degree - 1);

        remainder[degree - 1] = 0;

        for (int j = 0; j < degree; j++) {
            remainder[j] ^= gf_multiply(generator[j + 1], factor);
        }
    }
}

/* --------------------------------------------------------------------------
 * Version parameters
 * -------------------------------------------------------------------------- */

static int qr_data_codewords(int version) { return version == 4 ? 64 : 86; }

static int qr_ecc_codewords_per_block(int version) { return version == 4 ? 18 : 24; }

static int qr_total_codewords(int version) { return version == 4 ? 100 : 134; }

/*
 * Both versions we support have exactly two equal-sized blocks.
 *
 * Version 4:
 *   64 / 2 = 32 data bytes per block
 *
 * Version 5:
 *   86 / 2 = 43 data bytes per block
 */
static void qr_add_ecc_and_interleave(const uint8_t *data, int version, uint8_t *result) {
    const int data_codewords = qr_data_codewords(version);
    const int ecc_codewords = qr_ecc_codewords_per_block(version);

    const int block_count = 2;
    const int data_per_block = data_codewords / block_count;

    uint8_t generator[25];
    uint8_t remainder[24];
    uint8_t block[43];

    rs_generator(ecc_codewords, generator);

    /*
     * Data blocks are interleaved first.
     */
    for (int block_index = 0; block_index < block_count; block_index++) {
        const uint8_t *block_data = data + block_index * data_per_block;

        memcpy(block, block_data, (size_t)data_per_block);

        rs_remainder(block, data_per_block, generator, ecc_codewords, remainder);

        for (int i = 0; i < data_per_block; i++) {
            result[i * block_count + block_index] = block[i];
        }

        for (int i = 0; i < ecc_codewords; i++) {
            result[data_codewords + i * block_count + block_index] = remainder[i];
        }
    }
}

/* --------------------------------------------------------------------------
 * Function patterns
 * -------------------------------------------------------------------------- */

static void qr_finder_pattern(uint8_t *matrix, uint8_t *function, int size, int center_x, int center_y) {
    for (int dy = -4; dy <= 4; dy++) {
        for (int dx = -4; dx <= 4; dx++) {
            int ax = dx < 0 ? -dx : dx;
            int ay = dy < 0 ? -dy : dy;

            int distance = ax > ay ? ax : ay;

            bool dark = distance != 2 && distance != 4;

            function_module(matrix, function, size, center_x + dx, center_y + dy, dark);
        }
    }
}

static void qr_alignment_pattern(uint8_t *matrix, uint8_t *function, int size, int center_x, int center_y) {
    for (int dy = -2; dy <= 2; dy++) {
        for (int dx = -2; dx <= 2; dx++) {
            int ax = dx < 0 ? -dx : dx;
            int ay = dy < 0 ? -dy : dy;

            int distance = ax > ay ? ax : ay;

            function_module(matrix, function, size, center_x + dx, center_y + dy, distance != 1);
        }
    }
}

static void qr_function_patterns(uint8_t *matrix, uint8_t *function, int size, int version) {
    memset(matrix, 0, QR_BITMAP_BYTES);
    memset(function, 0, QR_BITMAP_BYTES);

    /*
     * Timing patterns.
     */
    for (int i = 0; i < size; i++) {
        function_module(matrix, function, size, 6, i, (i & 1) == 0);

        function_module(matrix, function, size, i, 6, (i & 1) == 0);
    }

    /*
     * Finder patterns.
     */
    qr_finder_pattern(matrix, function, size, 3, 3);

    qr_finder_pattern(matrix, function, size, size - 4, 3);

    qr_finder_pattern(matrix, function, size, 3, size - 4);

    /*
     * Alignment pattern.
     *
     * Version 4: center at 26
     * Version 5: center at 30
     *
     * Only the bottom-right alignment pattern exists for these versions,
     * because the other three positions overlap finder patterns.
     */
    int alignment = version == 4 ? 26 : 30;

    qr_alignment_pattern(matrix, function, size, alignment, alignment);

    /*
     * Reserve the two format-information areas.
     */
    for (int i = 0; i <= 5; i++)
        function_module(matrix, function, size, 8, i, false);

    function_module(matrix, function, size, 8, 7, false);

    function_module(matrix, function, size, 8, 8, false);

    function_module(matrix, function, size, 7, 8, false);

    for (int i = 9; i < 15; i++) {
        function_module(matrix, function, size, 14 - i, 8, false);
    }

    for (int i = 0; i < 8; i++) {
        function_module(matrix, function, size, size - 1 - i, 8, false);
    }

    for (int i = 8; i < 15; i++) {
        function_module(matrix, function, size, 8, size - 15 + i, false);
    }

    /*
     * The permanently dark module.
     */
    function_module(matrix, function, size, 8, size - 8, true);
}

/* --------------------------------------------------------------------------
 * Data placement
 * -------------------------------------------------------------------------- */

static void qr_draw_codewords(
    uint8_t *matrix, const uint8_t *function, int size, const uint8_t *codewords, int codeword_count) {
    int bit_index = 0;

    /*
     * QR data is written in vertical zig-zag pairs of columns.
     */
    for (int right = size - 1; right >= 1; right -= 2) {
        /*
         * Column 6 is occupied by the timing pattern.
         */
        if (right == 6)
            right = 5;

        bool upward = ((right + 1) & 2) == 0;

        for (int vertical = 0; vertical < size; vertical++) {
            int y = upward ? size - 1 - vertical : vertical;

            for (int j = 0; j < 2; j++) {
                int x = right - j;

                if (!module_get(function, size, x, y)) {
                    bool dark = false;

                    if (bit_index < codeword_count * 8) {
                        dark = (codewords[bit_index >> 3] >> (7 - (bit_index & 7))) & 1;
                    }

                    module_set(matrix, size, x, y, dark);

                    bit_index++;
                }
            }
        }
    }
}

/* --------------------------------------------------------------------------
 * Format information
 * -------------------------------------------------------------------------- */

/*
 * ECC = M -> format ECC bits = 00.
 */
static int qr_format_bits(int mask) {
    int data = mask;

    int remainder = data;

    for (int i = 0; i < 10; i++) {
        remainder = (remainder << 1) ^ ((remainder >> 9) * 0x537);
    }

    return ((data << 10) | remainder) ^ 0x5412;
}

static void qr_draw_format_bits(uint8_t *matrix, int size, int mask) {
    int bits = qr_format_bits(mask);

    /*
     * First copy.
     */
    for (int i = 0; i <= 5; i++) {
        module_set(matrix, size, 8, i, (bits >> i) & 1);
    }

    module_set(matrix, size, 8, 7, (bits >> 6) & 1);

    module_set(matrix, size, 8, 8, (bits >> 7) & 1);

    module_set(matrix, size, 7, 8, (bits >> 8) & 1);

    for (int i = 9; i < 15; i++) {
        module_set(matrix, size, 14 - i, 8, (bits >> i) & 1);
    }

    /*
     * Second copy.
     */
    for (int i = 0; i < 8; i++) {
        module_set(matrix, size, size - 1 - i, 8, (bits >> i) & 1);
    }

    for (int i = 8; i < 15; i++) {
        module_set(matrix, size, 8, size - 15 + i, (bits >> i) & 1);
    }

    /*
     * Permanently dark module.
     */
    module_set(matrix, size, 8, size - 8, true);
}

/* --------------------------------------------------------------------------
 * Masking
 * -------------------------------------------------------------------------- */

static bool qr_mask(int mask, int x, int y) {
    switch (mask) {
    case 0:
        return ((x + y) & 1) == 0;

    case 1:
        return (y & 1) == 0;

    case 2:
        return x % 3 == 0;

    case 3:
        return (x + y) % 3 == 0;

    case 4:
        return ((y / 2) + (x / 3)) % 2 == 0;

    case 5: {
        int product = x * y;
        return (product % 2 + product % 3) == 0;
    }

    case 6: {
        int product = x * y;
        return ((product % 2 + product % 3) & 1) == 0;
    }

    default: {
        int product = x * y;
        return (((x + y) & 1) + (product % 3)) % 2 == 0;
    }
    }
}

static void qr_apply_mask(uint8_t *matrix, const uint8_t *function, int size, int mask) {
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            if (!module_get(function, size, x, y)) {
                if (qr_mask(mask, x, y)) {
                    bool value = module_get(matrix, size, x, y);

                    module_set(matrix, size, x, y, !value);
                }
            }
        }
    }
}

/* --------------------------------------------------------------------------
 * Mask penalty
 * -------------------------------------------------------------------------- */

static int qr_penalty(const uint8_t *matrix, int size) {
    int penalty = 0;

    /*
     * Rule 1:
     * Five or more consecutive modules of the same color.
     */
    for (int y = 0; y < size; y++) {
        int run = 1;
        bool color = module_get(matrix, size, 0, y);

        for (int x = 1; x < size; x++) {
            bool next = module_get(matrix, size, x, y);

            if (next == color) {
                run++;
            } else {
                if (run >= 5)
                    penalty += 3 + run - 5;

                color = next;
                run = 1;
            }
        }

        if (run >= 5)
            penalty += 3 + run - 5;
    }

    for (int x = 0; x < size; x++) {
        int run = 1;
        bool color = module_get(matrix, size, x, 0);

        for (int y = 1; y < size; y++) {
            bool next = module_get(matrix, size, x, y);

            if (next == color) {
                run++;
            } else {
                if (run >= 5)
                    penalty += 3 + run - 5;

                color = next;
                run = 1;
            }
        }

        if (run >= 5)
            penalty += 3 + run - 5;
    }

    /*
     * Rule 2:
     * 2x2 blocks of equal color.
     */
    for (int y = 0; y < size - 1; y++) {
        for (int x = 0; x < size - 1; x++) {
            bool color = module_get(matrix, size, x, y);

            if (module_get(matrix, size, x + 1, y) == color && module_get(matrix, size, x, y + 1) == color &&
                module_get(matrix, size, x + 1, y + 1) == color) {
                penalty += 3;
            }
        }
    }

    /*
     * Rule 3:
     *
     * Finder-like pattern:
     *
     * 1011101
     *
     * surrounded by light modules.
     */
    for (int y = 0; y < size; y++) {
        for (int x = 0; x <= size - 11; x++) {
            bool pattern = !module_get(matrix, size, x + 0, y) && !module_get(matrix, size, x + 1, y) &&
                           module_get(matrix, size, x + 2, y) && module_get(matrix, size, x + 3, y) &&
                           module_get(matrix, size, x + 4, y) && !module_get(matrix, size, x + 5, y) &&
                           module_get(matrix, size, x + 6, y) && !module_get(matrix, size, x + 7, y) &&
                           !module_get(matrix, size, x + 8, y) && !module_get(matrix, size, x + 9, y) &&
                           !module_get(matrix, size, x + 10, y);

            if (pattern)
                penalty += 40;
        }
    }

    for (int x = 0; x < size; x++) {
        for (int y = 0; y <= size - 11; y++) {
            bool pattern = !module_get(matrix, size, x, y + 0) && !module_get(matrix, size, x, y + 1) &&
                           module_get(matrix, size, x, y + 2) && module_get(matrix, size, x, y + 3) &&
                           module_get(matrix, size, x, y + 4) && !module_get(matrix, size, x, y + 5) &&
                           module_get(matrix, size, x, y + 6) && !module_get(matrix, size, x, y + 7) &&
                           !module_get(matrix, size, x, y + 8) && !module_get(matrix, size, x, y + 9) &&
                           !module_get(matrix, size, x, y + 10);

            if (pattern)
                penalty += 40;
        }
    }

    /*
     * Rule 4:
     * Percentage of dark modules.
     */
    int dark = 0;

    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            if (module_get(matrix, size, x, y))
                dark++;
        }
    }

    int percentage = (dark * 100) / (size * size);

    int deviation = percentage > 50 ? percentage - 50 : 50 - percentage;

    penalty += (deviation / 5) * 10;

    return penalty;
}

/* --------------------------------------------------------------------------
 * Public API
 * -------------------------------------------------------------------------- */

bool qr_generate(QRCode *qr, const char *url) {
    if (qr == NULL || url == NULL)
        return false;

    size_t length = strlen(url);

    if (length > QR_MAX_URL_LENGTH)
        return false;

    /*
     * Version 4-M can contain 62 bytes in byte mode.
     * Version 5-M can contain 84 bytes.
     */
    int version = length <= 62 ? 4 : 5;

    int size = version * 4 + 17;

    int data_codewords = qr_data_codewords(version);

    int capacity_bits = data_codewords * 8;

    /*
     * Byte mode:
     *
     *   0100             -> 4 bits
     *   character count  -> 8 bits
     *   data              -> length * 8
     */
    int required_bits = 4 + 8 + (int)length * 8;

    if (required_bits > capacity_bits)
        return false;

    /*
     * Build the data codewords.
     */
    uint8_t data[86];

    memset(data, 0, sizeof(data));

    int bit_position = 0;

    /*
     * Small local bit writer.
     */
#define QR_WRITE_BITS(value, count)                                                                                    \
    do {                                                                                                               \
        unsigned _value = (unsigned)(value);                                                                           \
        for (int _i = (count) - 1; _i >= 0; _i--) {                                                                    \
            data[bit_position >> 3] |= ((_value >> _i) & 1u) << (7 - (bit_position & 7));                              \
            bit_position++;                                                                                            \
        }                                                                                                              \
    } while (0)

    /*
     * Byte mode = 0100.
     */
    QR_WRITE_BITS(4, 4);

    /*
     * Character count.
     *
     * Versions 1-9 use an 8-bit byte-mode count.
     */
    QR_WRITE_BITS(length, 8);

    /*
     * URL bytes.
     */
    for (size_t i = 0; i < length; i++)
        QR_WRITE_BITS((uint8_t)url[i], 8);

    /*
     * Terminator.
     */
    int remaining = capacity_bits - bit_position;

    int terminator = remaining > 4 ? 4 : remaining;

    QR_WRITE_BITS(0, terminator);

    /*
     * Align to byte boundary.
     */
    while (bit_position & 7)
        QR_WRITE_BITS(0, 1);

    /*
     * Pad bytes:
     *
     * EC 11 EC 11 ...
     */
    uint8_t pad = 0xEC;

    while (bit_position < capacity_bits) {
        QR_WRITE_BITS(pad, 8);

        pad ^= 0xEC ^ 0x11;
    }

#undef QR_WRITE_BITS

    /*
     * Generate Reed-Solomon ECC and interleave the blocks.
     */
    uint8_t codewords[134];

    qr_add_ecc_and_interleave(data, version, codewords);

    /*
     * Build the function modules.
     *
     * 'matrix' contains the QR.
     * 'function' tells us which modules cannot be masked.
     */
    uint8_t base[QR_BITMAP_BYTES];
    uint8_t function[QR_BITMAP_BYTES];

    qr_function_patterns(base, function, size, version);

    /*
     * Insert data.
     */
    qr_draw_codewords(base, function, size, codewords, qr_total_codewords(version));

    /*
     * Try all eight masks.
     */
    uint8_t candidate[QR_BITMAP_BYTES];

    int best_mask = 0;
    int best_penalty = 0x7FFFFFFF;

    for (int mask = 0; mask < 8; mask++) {
        memcpy(candidate, base, QR_BITMAP_BYTES);

        qr_apply_mask(candidate, function, size, mask);

        qr_draw_format_bits(candidate, size, mask);

        int penalty = qr_penalty(candidate, size);

        if (penalty < best_penalty) {
            best_penalty = penalty;
            best_mask = mask;

            memcpy(qr->data, candidate, QR_BITMAP_BYTES);
        }
    }

    /*
     * qr->data already contains the best masked candidate.
     */
    (void)best_mask;

    qr->size = (uint8_t)size;

    return true;
}

bool qr_get_module(const QRCode *qr, uint8_t x, uint8_t y) {
    if (qr == NULL)
        return false;

    if (x >= qr->size || y >= qr->size)
        return false;

    return module_get(qr->data, qr->size, x, y);
}