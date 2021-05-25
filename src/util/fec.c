#include <stdint.h>

#include "util/macros.h"

#include "fec.h"

// Encoding table. Map each 4-bit value to 8 bits. Values
// have been chosen to produce a mostly DC-Free encoding.
static const uint8_t symbols[16] = {
    0x0f, 0x18, 0x24, 0x33, 0x42, 0x55, 0x69, 0x7e,
    0x81, 0x96, 0xaa, 0xbd, 0xcc, 0xdb, 0xe7, 0xf0};

// Decoding table; 8-bit symbol to 4-bit code (packed table)
static const uint8_t codes[128] = {
    0x08, 0x40, 0x20, 0x00, 0x10, 0x00, 0x00, 0x00,
    0x11, 0x13, 0x15, 0x91, 0x11, 0x11, 0x11, 0x10,
    0x22, 0x23, 0x22, 0x22, 0x26, 0xa2, 0x22, 0x20,
    0x33, 0x33, 0x23, 0x33, 0x13, 0x33, 0x3b, 0x73,
    0x44, 0x44, 0x45, 0x44, 0x46, 0x44, 0xc4, 0x40,
    0x55, 0x45, 0x55, 0x55, 0x15, 0x5d, 0x55, 0x75,
    0x66, 0x46, 0x26, 0x6e, 0x66, 0x66, 0x66, 0x76,
    0xf7, 0x73, 0x75, 0x77, 0x76, 0x77, 0x77, 0x77,
    0x88, 0x88, 0x88, 0x98, 0x88, 0xa8, 0xc8, 0x80,
    0x98, 0x99, 0x99, 0x99, 0x19, 0x9d, 0x9b, 0x99,
    0xa8, 0xaa, 0x2a, 0xae, 0xaa, 0xaa, 0xab, 0xaa,
    0xfb, 0xb3, 0xbb, 0x9b, 0xbb, 0xab, 0xbb, 0xbb,
    0xc8, 0x4c, 0xcc, 0xce, 0xcc, 0xcd, 0xcc, 0xcc,
    0xfd, 0xdd, 0xd5, 0x9d, 0xdd, 0xdd, 0xcd, 0xdd,
    0xfe, 0xee, 0xee, 0xee, 0xe6, 0xae, 0xce, 0xee,
    0xff, 0xff, 0xff, 0xfe, 0xff, 0xfd, 0xfb, 0x7f};

static uint8_t fec_encode_nibble(uint8_t b)
{
    assert(b < ARRAY_COUNT(symbols));
    return symbols[b];
}

static uint8_t fec_decode_byte(uint8_t b)
{
    uint8_t d = codes[b >> 1];
    return ((b & 0x01) ? (d & 0x0f) : (d >> 4));
}

size_t fec_encode(const void *data, size_t size, void *output, size_t output_size)
{
    size_t total = FEC_ENCODED_SIZE(size);
    if (output_size < total)
    {
        return 0;
    }
    const uint8_t *input = data;
    uint8_t *ptr = output;
    for (int ii = 0; ii < size; ii++)
    {
        ptr[2 * ii] = fec_encode_nibble(input[ii] >> 4);
        ptr[2 * ii + 1] = fec_encode_nibble(input[ii] & 0xf);
    }
    return total;
}

size_t fec_decode(const void *data, size_t size, void *output, size_t output_size)
{
    size_t total = FEC_DECODED_SIZE(size);
    if (output_size < total)
    {
        return 0;
    }
    const uint8_t *input = data;
    uint8_t *ptr = output;
    for (int ii = 0; ii < size; ii += 2)
    {
        ptr[ii / 2] = fec_decode_byte(input[ii]) << 4;
        ptr[ii / 2] |= fec_decode_byte(input[ii + 1]);
    }
    return total;
}