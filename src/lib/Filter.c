

/*
 * Copyright 2009 Ossi Saresoja, Tomi Jylhä-Ollila
 *
 * This file is part of Kunquat.
 *
 * Kunquat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kunquat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kunquat.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <math.h>
#include <stdarg.h>
#include <string.h>

#include <math_common.h>
#include <Filter.h>

const double  bt[BINOM_MAX * (BINOM_MAX + 1) / 2] = { 1.0,
						      1.0,  1.0,
						      1.0,  2.0,   1.0,
						      1.0,  3.0,   3.0,   1.0,
						      1.0,  4.0,   6.0,   4.0,   1.0,
						      1.0,  5.0,  10.0,  10.0,   5.0,   1.0,
						      1.0,  6.0,  15.0,  20.0,  15.0,   6.0,   1.0,
						      1.0,  7.0,  21.0,  35.0,  35.0,  21.0,   7.0,  1.0,
						      1.0,  8.0,  28.0,  56.0,  70.0,  56.0,  28.0,  8.0, 1.0};


const double nbt[BINOM_MAX * (BINOM_MAX + 1) / 2] = { 1.0,
						     -1.0,  1.0,
						      1.0, -2.0,   1.0,
						     -1.0,  3.0, - 3.0,   1.0,
						      1.0, -4.0,   6.0, - 4.0,   1.0,
						     -1.0,  5.0, -10.0,  10.0, - 5.0,   1.0,
						      1.0, -6.0,  15.0, -20.0,  15.0, - 6.0,   1.0,
						     -1.0,  7.0, -21.0,  35.0, -35.0,  21.0, - 7.0,  1.0,
						      1.0, -8.0,  28.0, -56.0,  70.0, -56.0,  28.0, -8.0, 1.0};


const double*    binom[BINOM_MAX] = { bt     ,
				      bt + 1 ,
				      bt + 3 ,
				      bt + 6 ,
				      bt + 10,
				      bt + 15,
				      bt + 21,
				      bt + 28,
				      bt + 36};


const double* negbinom[BINOM_MAX] = {nbt     ,
				     nbt + 1 ,
				     nbt + 3 ,
				     nbt + 6 ,
				     nbt + 10,
				     nbt + 15,
				     nbt + 21,
				     nbt + 28,
				     nbt + 36};


double sinc(double x)
{
    return x == 0.0 ? 1.0 : sin(x) / x;
}


double powi(double x, int n)
{
    double ret = 1.0;
    while (n > 0)
    {
        if ((n & 1) != 0)
        {
            ret *= x;
        }
        n >>= 1;
        x *= x;
    }
    return ret;
}


double poly(double x, int n, ...)
{
    va_list args;
    double ret;
    va_start(args, n);
    ret = va_arg(args, double);
    for (int i = 0; i < n; ++i)
    {
        ret = ret * x + va_arg(args, double);
    }
    va_end(args);
    return ret;
}


void simple_lowpass_fir_create(int n, double f, double coeffs[])
{
    for (int i = 0; i <= n; ++i)
    {
        coeffs[i] = 2 * f * sinc(PI * f * (2 * i - n));
    }
    return;
}


#define C1 1.41421356237309504880 //sqrt(2)
#define C2 2.61312592975275305571 //sqrt(4+2*sqrt(2)) or equivalently sqrt(2+sqrt(2))+sqrt(2-sqrt(2))
#define C3 2.23606797749978969641 //sqrt(5)
#define C4 1.73205080756887729353 //sqrt(3)
#define C5 2.44948974278317809820 //sqrt(6)


void bilinear_butterworth_lowpass_filter_create(int n,
                                                double f,
                                                double q,
                                                double coeffsa[],
                                                double coeffsb[])
{
    assert(n >= 1);
    assert(n <= 6);
    assert(f < 0.5);
    assert(q >= 1.0);
    assert(q <= 1000);
    assert(coeffsa != NULL);
    assert(coeffsb != NULL);

    double a0   = 1.0;
    double fna0 = 1.0;
    f = tan(PI * f); //Prewarp
    switch(n)
    {
        case 1:
        {
            coeffsa[0] = poly(f, 1, 1.0, -1.0);
            a0         = poly(f, 1, 1.0,  1.0);
        }
        break;
        case 2:
        {
            coeffsa[0] = poly(f, 2, 1.0, -C1 / q,  1.0);
            coeffsa[1] = poly(f, 2, 2.0, 0.0, -2.0);
            a0         = poly(f, 2, 1.0,  C1 / q,  1.0);
        }
        break;
        case 3:
        {
            coeffsa[0] = poly(f, 3, 1.0, -2.0,  2.0, -1.0);
            coeffsa[1] = poly(f, 3, 3.0, -2.0, -2.0,  3.0);
            coeffsa[2] = poly(f, 3, 3.0,  2.0, -2.0, -3.0);
            a0         = poly(f, 3, 1.0,  2.0,  2.0,  1.0);
        }
        break;
        case 4:
        {
            coeffsa[0] = poly(f, 4, 1.0, -  C2,  2.0+  C1, -  C2,  1.0);
            coeffsa[1] = poly(f, 4, 4.0, -2*C2,       0.0,  2*C2, -4.0);
            coeffsa[2] = poly(f, 4, 6.0,   0.0, -4.0-2*C1,   0.0,  6.0);
            coeffsa[3] = poly(f, 4, 4.0,  2*C2,       0.0, -2*C2, -4.0);
            a0         = poly(f, 4, 1.0,    C2,  2.0+  C1,    C2,  1.0);
        }
        break;
        case 5:
        {
            coeffsa[0] = poly(f, 5,  1.0, -1.0-  C3,  3.0+  C3, -3.0-  C3,  1.0+  C3, - 1.0);
            coeffsa[1] = poly(f, 5,  5.0, -3.0-3*C3,  3.0+  C3,  3.0+  C3, -3.0-3*C3,   5.0);
            coeffsa[2] = poly(f, 5, 10.0, -2.0-2*C3, -6.0-2*C3,  6.0+2*C3,  2.0+2*C3, -10.0);
            coeffsa[3] = poly(f, 5, 10.0,  2.0+2*C3, -6.0-2*C3, -6.0-2*C3,  2.0+2*C3,  10.0);
            coeffsa[4] = poly(f, 5,  5.0,  3.0+3*C3,  3.0+  C3, -3.0-  C3, -3.0-3*C3, - 5.0);
            a0         = poly(f, 5,  1.0,  1.0+  C3,  3.0+  C3,  3.0+  C3,  1.0+  C3,   1.0);
        }
        break;
        case 6:
        {
            coeffsa[0] = poly(f, 6,  1.0, -  C1-  C5,   4.0+2*C4, -3*C1-2*C5,   4.0+2*C4,  -  C1-  C5,   1.0);
            coeffsa[1] = poly(f, 6,  6.0, -4*C1-4*C5,   8.0+4*C4,        0.0, - 8.0-4*C4,   4*C1+4*C5, - 6.0);
            coeffsa[2] = poly(f, 6, 15.0, -5*C1-5*C5, - 4.0-2*C4,  9*C1+6*C5, - 4.0-2*C4,  -5*C1-5*C5,  15.0);
            coeffsa[3] = poly(f, 6, 20.0,        0.0, -16.0-8*C4,        0.0,  16.0+8*C4,         0.0, -20.0);
            coeffsa[4] = poly(f, 6, 15.0,  5*C1+5*C5, - 4.0-2*C4, -9*C1-6*C5, - 4.0-2*C4,   5*C1+5*C5,  15.0);
            coeffsa[5] = poly(f, 6,  6.0,  4*C1+4*C5,   8.0+4*C4,        0.0, - 8.0-4*C4,  -4*C1-4*C5, - 6.0);
            a0         = poly(f, 6,  1.0,    C1+  C5,   4.0+2*C4,  3*C1+2*C5,   4.0+2*C4,     C1+  C5,   1.0);
        }
        break;
        default:
        {
            assert(false);
        }
    }
    for (int i = 0; i < n; ++i)
    {
        coeffsa[i] /= a0;
    }
    fna0 = powi(f, n) / a0;
    for (int i = 0; i <= n; ++i)
    {
        coeffsb[i] = binom[n][i] * fna0;
    }
    return;
}


void bilinear_chebyshev_t1_lowpass_filter_create(int n,
                                                 double f,
                                                 double e,
                                                 double* coeffsa,
                                                 double* coeffsb)
{
    int i;
    double a0 = 1.0;
    double fna0 = 0;
 
    double temp1 = pow((sqrt(1.0 + e * e) + 1.0) / 2, 1.0 / n);
    double temp2 = pow((sqrt(1.0 + e * e) - 1.0) / 2, 1.0 / n);
    double sh = temp1-temp2;
    double ch = temp1+temp2;
 
    f = tan(PI*f);
 
    switch(n)
    {
        case 1:
        {
            coeffsa[0] = poly(f, n, sh, -1.0);
            a0         = poly(f, n, sh,  1.0);
        }
        break;
        case 2:
        {
            coeffsa[0] = poly(f, n, (sh*sh+ch*ch)/2, -sh*C1,  1.0);
            coeffsa[1] = poly(f, n,  sh*sh+ch*ch   ,    0.0, -2.0);
            a0         = poly(f, n, (sh*sh+ch*ch)/2,  sh*C1,  1.0);
        }
        break;
        default:
        {
            assert(false);
        }
        break;
    }
 
    for (i = 0; i < n; ++i)
    {
        coeffsa[i] /= a0;
    }
 
    fna0 = powi(f, n) * e * (1 << (n - 1)) / a0;
 
    for (i = 0; i <= n; ++i)
    {
        coeffsb[i] = binom[n][i] * fna0;
    }
    return;
}


#define dprod2(histbuf, sourcebuf, coeffs, n, i, acc, oper)	\
  if (true)							\
    {								\
      int j = 0;						\
      int k = (i);						\
      dprod(histbuf, coeffs, j, k, n, n, acc, oper);		\
      dprod(sourcebuf, coeffs, j, k, nframes, n, acc, oper);	\
    } else (void)0


void buffer(kqt_frame* histbuf,
            kqt_frame* sourcebuf,
            int n,
            int nframes)
{
    if (nframes < n)
    {
        memmove(histbuf, histbuf + nframes, (n - nframes) * sizeof(kqt_frame));
        memcpy(histbuf + n - nframes, sourcebuf, nframes * sizeof(kqt_frame));
    }
    else
    {
        memcpy(histbuf, sourcebuf + nframes - n, n * sizeof(kqt_frame));
    }
    return;
}


#if 0
void fir_filter(int n,
                double* coeffs,
                kqt_frame* histbuf,
                int nframes,
                kqt_frame* inbuf,
                kqt_frame* outbuf)
{
    double temp;
 
    for (int i = 0; i < nframes; ++i)
    {
        temp = inbuf[i] * coeffs[n];
        dprod2(histbuf, inbuf, coeffs, n, i, temp, +=);
        outbuf[i] = temp;
    }
 
    buffer(histbuf, inbuf, n, nframes);
 
    return;
}
#endif


void iir_filter_df1_old(int na,
                    int nb,
                    double* coeffsa,
                    double* coeffsb,
                    kqt_frame* histbufa,
                    kqt_frame* histbufb,
                    int nframes,
                    kqt_frame* inbuf,
                    kqt_frame* outbuf)
{
    double temp;
    for (int i = 0; i < nframes; ++i)
    {
        temp = inbuf[i] * coeffsb[nb];
        dprod2(histbufa, outbuf, coeffsa, na, i, temp, -=);
        dprod2(histbufb,  inbuf, coeffsb, nb, i, temp, +=);
        outbuf[i] = temp;
    }
    buffer(histbufa, outbuf, na, nframes);
    buffer(histbufb,  inbuf, nb, nframes);
    return;
}


#if 0
void iir_filter_pure(int n,
                     double* coeffs,
                     kqt_frame* histbuf,
                     int nframes,
                     kqt_frame* inbuf,
                     kqt_frame* outbuf)
{
    double temp;
    for (int i = 0; i < nframes; ++i)
    {
        temp = inbuf[i];
        dprod2(histbuf, outbuf, coeffs, n, i, temp, -=);
        outbuf[i] = temp;
    }
    buffer(histbuf, outbuf, n, nframes);
    return;
}
#endif


