

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
#include <stdbool.h>
#include <math.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#include <math_common.h>
#include <Filter.h>
#include <xassert.h>


double sinc(double x)
{
    return x == 0.0 ? 1.0 : sin(x) / x;
}


void simple_lowpass_fir_create(int n, double f, double *coeffs)
{
    for (int i = 0; i <= n; ++i)
    {
        coeffs[i] = 2 * f * sinc(PI * f * (2 * i - n));
    }
    return;
}


void two_pole_lowpass_filter_create(double f,
                                    double q,
                                    double coeffs[2],
                                    double *a0)
{
    assert(f < 0.5);
    assert(f > 0);
    assert(q >= 0.5);
    assert(q <= 1000);
    assert(coeffs != NULL);
    assert(a0 != NULL);
    //    static int created = 0;
    //    fprintf(stderr, "  %d \n", ++created);

    f = 1.0 / tan(PI * f); //Prewarp
    double i2q = 1.0 / (2 * q);
    double a0_temp   = ((f + i2q) * (f + i2q) + (1.0 + i2q) * (1.0 - i2q))          ;
           coeffs[0] = ((f - i2q) * (f - i2q) + (1.0 + i2q) * (1.0 - i2q)) / a0_temp;
           coeffs[1] = 2 * (1.0 + f) * (1.0 - f) / a0_temp;
    *a0 = a0_temp;
    return;
}


void butterworth_lowpass_filter_create(int n,
                                       double f,
                                       double coeffs[n],
                                       double *a0)

{
    assert(n >= 1);
    assert(f < 0.5);
    assert(coeffs != NULL);
    assert(a0 != NULL);
    //    static int created = 0;
    //    fprintf(stderr, "  %d \n", ++created);

    f = 1.0 / tan(PI * f); //Prewarp
    double a0_tot  = 1.0;
    for(int i = 0; i < (n & ~((int)1)); i += 2)
    {
        double sini = sin(PI / (2 * n) * (i + 1));
        double cosi = cos(PI / (2 * n) * (i + 1));
        double a0_temp     = ((sini + f) * (sini + f) + cosi * cosi)          ;
               coeffs[i  ] = ((sini - f) * (sini - f) + cosi * cosi) / a0_temp;
               coeffs[i+1] = 2 * (1.0 + f) * (1.0 - f) / a0_temp;
        a0_tot *= a0_temp;
    }
    if(n & 1)
    {
        double a0_temp     = (1.0 + f)          ;
               coeffs[n-1] = (1.0 - f) / a0_temp;
        a0_tot *= a0_temp;
    }
    *a0 = a0_tot;
    return;
}


#define dprod2(histbuf, sourcebuf, coeffs, n, i, acc, oper)       \
    if (true)                                                     \
    {                                                             \
        int j = (i);                                              \
        int k = 0;                                                \
        dprod(histbuf, coeffs, j, k, n, n, acc, oper);            \
        j -= (n);                                                 \
        dprod(sourcebuf, coeffs, j, k, nframes, n, acc, oper);    \
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


double iir_filter_strict_cascade(int n,
                                 double coeffs[n],
                                 double buf[n],
                                 double var)
{
    for (int i = 0; i < (n & ~((int)1)); i += 2)
    {
        var -= coeffs[i] * buf[i] + coeffs[i+1] * buf[i+1];
        buf[i] = buf[i+1];
        buf[i+1] = var;
    }
    if (n & 1)
    {
        var -= coeffs[n-1] * buf[n-1];
        buf[n-1] = var;
    }
    return var;
}


double iir_filter_strict_transposed_cascade(int n,
                                            double coeffs[n],
                                            double buf[n],
                                            double var)
{
    for (int i = 0; i < (n & ~((int)1)); i += 2)
    {
        var += buf[i+1];
        buf[i+1] = buf[i] - coeffs[i+1] * var;
        buf[i] = -coeffs[i] * var;
    }
    if (n & 1)
    {
        var += buf[n-1];
        buf[n-1] = -coeffs[n-1] * var;
    }
    return var;
}


double dc_zero_filter(int n,
                      double buf[n],
                      double var)
{
    for (int i = 0; i < n; ++i)
    {
        double temp = buf[i];
        buf[i] = var;
        var -= temp;
    }
    return var;
}


double nq_zero_filter(int n,
                      double buf[n],
                      double var)
{
    for (int i = 0; i < n; ++i)
    {
        double temp = buf[i];
        buf[i] = var;
        var += temp;
    }
    return var;
}


double dc_pole_filter(int n,
                      double buf[n],
                      double var)
{
    for (int i = 0; i < n; ++i)
    {
        var += buf[i];
        buf[i] = var;
    }
    return var;
}


double nq_pole_filter(int n,
                      double buf[n],
                      double var)
{
    for (int i = 0; i < n; ++i)
    {
        var -= buf[i];
        buf[i] = var;
    }
    return var;
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


