

/*
 * Authors: Ossi Saresoja, Finland 2009
 *          Tomi Jylhä-Ollila, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <math.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#include <math_common.h>
#include <Filter.h>


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


int binom(int n, int k)
{
    return (k == 0 || k == n) ? 1 : binom(n - 1, k - 1) + binom(n - 1, k);
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
//    static int created = 0;
//    fprintf(stderr, "  %d \n", ++created);

    double a0   = 1.0;
    double fna0 = 1.0;
    f = tan(PI * f);
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
        coeffsb[i] = binom(n, i) * fna0;
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
 
    for(i = 0; i < n; ++i)
    {
        coeffsa[i] /= a0;
    }
 
    fna0 = powi(f, n) * e * (1 << (n - 1)) / a0;
 
    for(i = 0; i <= n; ++i)
    {
        coeffsb[i] = binom(n, i) * fna0;
    }
    return;
}


/*
void invert(int n, double* coeffs)
{
  int i;
  
  for(i=!(n % 2);i<n;i+=2)
    coeffs[i] = -coeffs[i];
}
*/


#define DPROD(histbuff, sourcebuff, coeffs, n, i, acc, oper) \
    if (true)                                                \
    {                                                        \
        int j = 0;                                           \
        int k = 0;                                           \
        for (j = 0, k = (i); k < (n); ++j, ++k)              \
        {                                                    \
            (acc) oper (histbuff)[k] * (coeffs)[j];          \
        }                                                    \
        for (k -= (n); j < (n); ++j, ++k)                    \
        {                                                    \
            (acc) oper (sourcebuff)[k] * (coeffs)[j];        \
        }                                                    \
    } else (void)0


#define BUFFER(histbuff, sourcebuff, n, amount)                                               \
    if (true)                                                                                 \
    {                                                                                         \
        if((amount)<(n))                                                                      \
        {                                                                                     \
            memmove((histbuff), (histbuff) + (amount), ((n) - (amount)) * sizeof(kqt_frame)); \
            memcpy((histbuff) + (n) - (amount), (sourcebuff), (amount) * sizeof(kqt_frame));  \
        }                                                                                     \
        else                                                                                  \
        {                                                                                     \
            memcpy((histbuff), (sourcebuff) + (amount) - (n), (n) * sizeof(kqt_frame));       \
        }                                                                                     \
    } else (void)0


void fir_filter(int n,
                double* coeffs,
                kqt_frame* histbuff,
                int amount,
                kqt_frame* inbuff,
                kqt_frame* outbuff)
{
    double temp;
 
    for(int i = 0; i < amount; ++i)
    {
        temp = inbuff[i] * coeffs[n];
        DPROD(histbuff, inbuff, coeffs, n, i, temp, +=);
        outbuff[i] = temp;
    }
 
    BUFFER(histbuff, inbuff, n, amount);
 
    return;
}


void iir_filter_df1(int na,
                    int nb,
                    double* coeffsa,
                    double* coeffsb,
                    kqt_frame* histbuffa,
                    kqt_frame* histbuffb,
                    int amount,
                    kqt_frame* inbuff,
                    kqt_frame* outbuff)
{
    double temp;
    for (int i = 0; i < amount; ++i)
    {
        temp = inbuff[i] * coeffsb[nb];
        DPROD(histbuffa, outbuff, coeffsa, na, i, temp, -=);
        DPROD(histbuffb,  inbuff, coeffsb, nb, i, temp, +=);
        outbuff[i] = temp;
    }
    BUFFER(histbuffa, outbuff, na, amount);
    BUFFER(histbuffb,  inbuff, nb, amount);
    return;
}


void iir_filter_df2(int na,
                    int nb,
                    double* coeffsa,
                    double* coeffsb,
                    kqt_frame* histbuff,
                    int amount,
                    kqt_frame* inbuff,
                    kqt_frame* outbuff)
{
    double temp;
    for (int i = 0; i < amount; ++i)
    {
        temp = inbuff[i];
        DPROD(&histbuff[MAX(na, nb) - na], inbuff, coeffsa, na, i, temp, -=);
        inbuff[i] = temp;
        temp *= coeffsb[nb];
        DPROD(&histbuff[MAX(na, nb) - nb], inbuff, coeffsb, nb, i, temp, +=);
        outbuff[i] = temp;
    }
    BUFFER(histbuff, inbuff, MAX(na,nb), amount);
    return;
}


void iir_filter_pure(int n,
                     double* coeffs,
                     kqt_frame* histbuff,
                     int amount,
                     kqt_frame* inbuff,
                     kqt_frame* outbuff)
{
    double temp;
    for (int i = 0; i < amount; ++i)
    {
        temp = inbuff[i];
        DPROD(histbuff, outbuff, coeffs, n, i, temp, -=);
        outbuff[i] = temp;
    }
    BUFFER(histbuff, outbuff, n, amount);
    return;
}


