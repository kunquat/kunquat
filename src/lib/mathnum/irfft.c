

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
    rassert(bit_count > 0);
    rassert(bit_count <= 31);

    n = (uint32_t)(((n & 0xaaaaaaaaUL) >> 1) | ((n & 0x55555555UL) << 1));
    n = (uint32_t)(((n & 0xccccccccUL) >> 2) | ((n & 0x33333333UL) << 2));
    n = (uint32_t)(((n & 0xf0f0f0f0UL) >> 4) | ((n & 0x0f0f0f0fUL) << 4));
    n = (uint32_t)(((n & 0xff00ff00UL) >> 8) | ((n & 0x00ff00ffUL) << 8));
    n = (uint32_t)((n >> 16) | (n << 16));

    return n >> (32 - bit_count);
}


static int get_bit_count(int32_t n)
{
    rassert(n > 0);
    rassert(is_p2(n));

    int count = 0;
    n >>= 1;
    while (n != 0)
    {
        ++count;
        n >>= 1;
    }

    return count;
}


static void bit_reversal_permute(float* data, int32_t length)
{
    rassert(is_p2(length));

    const int bit_count = get_bit_count(length);

    for (int32_t i = 0; i < length; ++i)
    {
        const int32_t rev_i = (int32_t)reverse_bits((uint32_t)i, bit_count);
        if (rev_i > i)
        {
            const float temp = data[i];
            data[i] = data[rev_i];
            data[rev_i] = temp;
        }
    }

    return;
}


void fill_Ws(float* Ws, int32_t tlength)
{
    rassert(Ws != NULL);
    rassert(tlength >= 4);
    rassert(is_p2(tlength));

    const int32_t Wlength = tlength / 4;

    for (int32_t i = 0; i < Wlength; ++i)
    {
        const double u = -2.0 * PI * i / (double)tlength;
        Ws[2 * i] = (float)cos(u);
        Ws[2 * i + 1] = (float)sin(u);
    }

    return;
}


void irfft(float* data, const float* Ws, int32_t length)
{
    rassert(data != NULL);
    rassert(Ws != NULL);
    rassert(length > 0);
    rassert(is_p2(length));

    const int bit_count = get_bit_count(length);
    rassert(bit_count > 0);
    rassert(bit_count < 31);

    // Algorithmically mostly based on the description in the LaTeX documentation
    // of GNU Scientific Library, http://www.gnu.org/software/gsl/
    for (int i = bit_count; i > 0; --i)
    {
        const int32_t p_i = 1 << i;
        const int32_t p_im1 = p_i >> 1;
        const int32_t q_i = length / p_i;

        for (int32_t b = 0; b < q_i; ++b)
        {
            const int32_t ix0 = b * p_i;
            const int32_t ix1 = b * p_i + p_im1;
            const float x0 = data[ix0] + data[ix1];
            const float x1 = data[ix0] - data[ix1];
            data[ix0] = x0;
            data[ix1] = x1;
        }

        const int32_t p_im2 = p_im1 >> 1;
        for (int32_t a = 1; a < p_im2; ++a)
        {
            for (int32_t b = 0; b < q_i; ++b)
            {
                const int32_t bp_i = b * p_i;

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

                const float Wa_p_i_re = Ws[2 * a * q_i];
                const float Wa_p_i_im = Ws[2 * a * q_i + 1];

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


