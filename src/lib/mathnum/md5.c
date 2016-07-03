

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <mathnum/md5.h>

#include <debug/assert.h>
#include <mathnum/common.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


static void add_length(unsigned char* p, uint64_t len);

static void prepare_X(uint32_t* X, const unsigned char* p);
static void md5_rounds(
        uint32_t* X, uint32_t* aa, uint32_t* bb, uint32_t* cc, uint32_t* dd);
static uint32_t F(uint32_t x, uint32_t y, uint32_t z);
static uint32_t G(uint32_t x, uint32_t y, uint32_t z);
static uint32_t H(uint32_t x, uint32_t y, uint32_t z);
static uint32_t I(uint32_t x, uint32_t y, uint32_t z);

static uint32_t left_rotate(uint32_t value, int steps);


//#define PADDED_LEN ((MSG_LEN_MAX) + 1 + 8)


static const uint32_t T[] =
{
    0xd76aa478UL, 0xe8c7b756UL, 0x242070dbUL, 0xc1bdceeeUL,
    0xf57c0fafUL, 0x4787c62aUL, 0xa8304613UL, 0xfd469501UL,
    0x698098d8UL, 0x8b44f7afUL, 0xffff5bb1UL, 0x895cd7beUL,
    0x6b901122UL, 0xfd987193UL, 0xa679438eUL, 0x49b40821UL,
    0xf61e2562UL, 0xc040b340UL, 0x265e5a51UL, 0xe9b6c7aaUL,
    0xd62f105dUL, 0x2441453UL,  0xd8a1e681UL, 0xe7d3fbc8UL,
    0x21e1cde6UL, 0xc33707d6UL, 0xf4d50d87UL, 0x455a14edUL,
    0xa9e3e905UL, 0xfcefa3f8UL, 0x676f02d9UL, 0x8d2a4c8aUL,
    0xfffa3942UL, 0x8771f681UL, 0x6d9d6122UL, 0xfde5380cUL,
    0xa4beea44UL, 0x4bdecfa9UL, 0xf6bb4b60UL, 0xbebfbc70UL,
    0x289b7ec6UL, 0xeaa127faUL, 0xd4ef3085UL, 0x4881d05UL,
    0xd9d4d039UL, 0xe6db99e5UL, 0x1fa27cf8UL, 0xc4ac5665UL,
    0xf4292244UL, 0x432aff97UL, 0xab9423a7UL, 0xfc93a039UL,
    0x655b59c3UL, 0x8f0ccc92UL, 0xffeff47dUL, 0x85845dd1UL,
    0x6fa87e4fUL, 0xfe2ce6e0UL, 0xa3014314UL, 0x4e0811a1UL,
    0xf7537e82UL, 0xbd3af235UL, 0x2ad7d2bbUL, 0xeb86d391UL,
};


#define CHUNK_BITS 512
#define CHUNK_BYTES ((CHUNK_BITS) / 8)
#define LENGTH_POS ((CHUNK_BYTES) - 8)


void md5_str(const char* str, uint64_t* lower, uint64_t* upper)
{
    assert(str != NULL);
    assert(lower != NULL);
    assert(upper != NULL);

    md5(str, (int)strlen(str), lower, upper, true);

    return;
}


void md5(const char* seq, int len, uint64_t* lower, uint64_t* upper, bool complete)
{
    assert(seq != NULL);
    assert(len >= 0);
    assert(lower != NULL);
    assert(upper != NULL);

    const uint64_t a = 0x67452301ULL;
    const uint64_t b = 0xefcdab89ULL;
    const uint64_t c = 0x98badcfeULL;
    const uint64_t d = 0x10325476ULL;
    const uint64_t lower_init = a | (b << 32);
    const uint64_t upper_init = c | (d << 32);

    md5_with_state(seq, len, lower, upper, lower_init, upper_init, complete, 0);

    return;
}


void md5_with_state(
        const char* seq,
        int len,
        uint64_t* lower, uint64_t* upper,
        uint64_t lower_init, uint64_t upper_init,
        bool last,
        int prev_len)
{
    assert(seq != NULL);
    assert(len >= 0);
    assert(lower != NULL);
    assert(upper != NULL);
    assert(last || len % 64 == 0);
    assert(prev_len >= 0);

    int cur_len = len;
    len += prev_len;
    uint32_t a = (uint32_t)lower_init;
    uint32_t b = (uint32_t)(lower_init >> 32);
    uint32_t c = (uint32_t)upper_init;
    uint32_t d = (uint32_t)(upper_init >> 32);
    unsigned char padded[CHUNK_BYTES] = { 0 };
    uint32_t X[16] = { 0 };

    for (; cur_len >= 0; cur_len -= CHUNK_BYTES, seq += CHUNK_BYTES)
    {
        const unsigned char* p = (const unsigned char*)seq;
        if (cur_len < CHUNK_BYTES)
        {
            if (!last && cur_len == 0)
                break;

            memcpy((char*)padded, seq, (size_t)cur_len);
            padded[cur_len] = 0x80;
            if (cur_len < LENGTH_POS)
                add_length(padded + LENGTH_POS, (uint64_t)len);

            p = padded;
        }

        prepare_X(X, p);
        md5_rounds(X, &a, &b, &c, &d);
    }

    if (last && cur_len >= LENGTH_POS - CHUNK_BYTES)
    {
        memset((char*)padded, 0, CHUNK_BYTES);
        add_length(padded + LENGTH_POS, (uint64_t)len);
        prepare_X(X, padded);
        md5_rounds(X, &a, &b, &c, &d);
    }

    *lower = a | ((uint64_t)b << 32);
    *upper = c | ((uint64_t)d << 32);

    return;
}


static void prepare_X(uint32_t* X, const unsigned char* p)
{
    assert(X != NULL);
    assert(p != NULL);

    for (int i = 0; i < 16; ++i)
    {
        X[i] = *p++;
        X[i] |= (uint32_t)*p++ << 8;
        X[i] |= (uint32_t)*p++ << 16;
        X[i] |= (uint32_t)*p++ << 24;
    }

    return;
}


static void md5_rounds(
        uint32_t* X, uint32_t* aa, uint32_t* bb, uint32_t* cc, uint32_t* dd)
{
    assert(X != NULL);
    assert(aa != NULL);
    assert(bb != NULL);
    assert(cc != NULL);
    assert(dd != NULL);

    uint32_t a = *aa;
    uint32_t b = *bb;
    uint32_t c = *cc;
    uint32_t d = *dd;

#define op4(r, k1, s1, i1, k2, s2, i2, k3, s3, i3, k4, s4, i4) \
        r(a, b, c, d, k1, s1, i1 - 1); r(d, a, b, c, k2, s2, i2 - 1); \
        r(c, d, a, b, k3, s3, i3 - 1); r(b, c, d, a, k4, s4, i4 - 1)

#define r1(a, b, c, d, k, s, i) \
        (a) = (b) + left_rotate(((a) + F((b),(c),(d)) + X[(k)] + T[(i)]), (s))
    op4(r1,  0,  7,  1,   1, 12,  2,   2, 17,  3,   3, 22,  4);
    op4(r1,  4,  7,  5,   5, 12,  6,   6, 17,  7,   7, 22,  8);
    op4(r1,  8,  7,  9,   9, 12, 10,  10, 17, 11,  11, 22, 12);
    op4(r1, 12,  7, 13,  13, 12, 14,  14, 17, 15,  15, 22, 16);
#undef r1

#define r2(a, b, c, d, k, s, i) \
        (a) = (b) + left_rotate(((a) + G((b),(c),(d)) + X[(k)] + T[(i)]), (s))
    op4(r2,  1,  5, 17,   6,  9, 18,  11, 14, 19,   0, 20, 20);
    op4(r2,  5,  5, 21,  10,  9, 22,  15, 14, 23,   4, 20, 24);
    op4(r2,  9,  5, 25,  14,  9, 26,   3, 14, 27,   8, 20, 28);
    op4(r2, 13,  5, 29,   2,  9, 30,   7, 14, 31,  12, 20, 32);
#undef r2

#define r3(a, b, c, d, k, s, i) \
        (a) = (b) + left_rotate(((a) + H((b),(c),(d)) + X[(k)] + T[(i)]), (s))
    op4(r3,  5,  4, 33,   8, 11, 34,  11, 16, 35,  14, 23, 36);
    op4(r3,  1,  4, 37,   4, 11, 38,   7, 16, 39,  10, 23, 40);
    op4(r3, 13,  4, 41,   0, 11, 42,   3, 16, 43,   6, 23, 44);
    op4(r3,  9,  4, 45,  12, 11, 46,  15, 16, 47,   2, 23, 48);
#undef r3

#define r4(a, b, c, d, k, s, i) \
        (a) = (b) + left_rotate(((a) + I((b),(c),(d)) + X[(k)] + T[(i)]), (s))
    op4(r4,  0,  6, 49,   7, 10, 50,  14, 15, 51,   5, 21, 52);
    op4(r4, 12,  6, 53,   3, 10, 54,  10, 15, 55,   1, 21, 56);
    op4(r4,  8,  6, 57,  15, 10, 58,   6, 15, 59,  13, 21, 60);
    op4(r4,  4,  6, 61,  11, 10, 62,   2, 15, 63,   9, 21, 64);
#undef r4

#undef op4

    *aa += a;
    *bb += b;
    *cc += c;
    *dd += d;

    return;
}


static void add_length(unsigned char* p, uint64_t len)
{
    assert(p != NULL);

    len *= 8;
    for (int i = 0; i < 8; ++i)
    {
        p[i] = len & 0xFF;
        len >>= 8;
    }

    return;
}


static uint32_t F(uint32_t x, uint32_t y, uint32_t z)
{
    return (x & y) | (~x & z);
}


static uint32_t G(uint32_t x, uint32_t y, uint32_t z)
{
    return (x & z) | (y & ~z);
}


static uint32_t H(uint32_t x, uint32_t y, uint32_t z)
{
    return x ^ y ^ z;
}


static uint32_t I(uint32_t x, uint32_t y, uint32_t z)
{
    return y ^ (x | ~z);
}


static uint32_t left_rotate(uint32_t value, int steps)
{
    assert(steps > 0);
    assert(steps < 32);

    return (value << steps) | (value >> (32 - steps));
}


