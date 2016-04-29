

/*
 * Author: Tomi Jylhä-Ollila, Finland 2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <mathnum/irfft.h>

#include <debug/assert.h>
#include <mathnum/common.h>

#include <math.h>
#include <stdint.h>
#include <stdlib.h>


static uint32_t reverse_bits(uint32_t n, int bit_count)
{
    assert(bit_count > 0);
    assert(bit_count <= 32);

    n = ((n & 0xaaaaaaaaUL) >> 1) | ((n & 0x55555555UL) << 1);
    n = ((n & 0xccccccccUL) >> 2) | ((n & 0x33333333UL) << 2);
    n = ((n & 0xf0f0f0f0UL) >> 4) | ((n & 0x0f0f0f0fUL) << 4);
    n = ((n & 0xff00ff00UL) >> 8) | ((n & 0x00ff00ffUL) << 8);
    n = (n >> 16) | (n << 16);

    return n >> (32 - bit_count);
}


static int get_bit_count(size_t n)
{
    assert(n > 0);
    assert(is_p2(n));

    int count = 0;
    n >>= 1;
    while (n != 0)
    {
        ++count;
        n >>= 1;
    }

    return count;
}


static void bit_reversal_permute(float* data, size_t length)
{
    assert(is_p2(length));

    const int bit_count = get_bit_count(length);

    for (size_t i = 0; i < length; ++i)
    {
        const size_t rev_i = reverse_bits(i, bit_count);
        if (rev_i > i)
        {
            const float temp = data[i];
            data[i] = data[rev_i];
            data[rev_i] = temp;
        }
    }

    return;
}


void irfft(float* data, size_t length)
{
    assert(data != NULL);
    assert(length > 0);
    assert(is_p2(length));

    const int bit_count = get_bit_count(length);
    assert(bit_count > 0);
    assert(bit_count < 32);

    for (int i = bit_count; i > 0; --i)
    {
        const size_t p_i = 1 << i;
        const size_t p_im1 = p_i >> 1;
        const size_t q_i = length / p_i;

        for (size_t b = 0; b < q_i; ++b)
        {
            const size_t ix0 = b * p_i;
            const size_t ix1 = b * p_i + p_im1;
            const float x0 = data[ix0] + data[ix1];
            const float x1 = data[ix0] - data[ix1];
            data[ix0] = x0;
            data[ix1] = x1;
        }

        const size_t p_im2 = p_im1 >> 1;
        for (size_t a = 1; a < p_im2; ++a)
        {
            for (size_t b = 0; b < q_i; ++b)
            {
                const size_t bp_i = b * p_i;

                const float z0_re = data[bp_i + a];
                const float z0_im = data[bp_i + p_i - a];
                const float z1_re = data[bp_i + p_im1 - a];
                const float z1_im = -data[bp_i + p_im1 + a];

                const float t0_re = z0_re + z1_re;
                const float t0_im = z0_im + z1_im;
                const float t1_re = z0_re - z1_re;
                const float t1_im = z0_im - z1_im;
                data[bp_i + a] = t0_re;
                data[bp_i + p_im1 - a] = t0_im;

                // TODO: add cache for complex roots of unity
                const double u = -2.0 * PI * a / (double)p_i;
                const float Wa_p_i_re = cos(u);
                const float Wa_p_i_im = sin(u);

                const float rot_t1_re = Wa_p_i_re * t1_re - Wa_p_i_im * t1_im;
                const float rot_t1_im = Wa_p_i_im * t1_re + Wa_p_i_re * t1_im;

                data[bp_i + p_im1 + a] = rot_t1_re;
                data[bp_i + p_i - a] = rot_t1_im;
            }
        }
    }

    bit_reversal_permute(data, length);

    return;
}


