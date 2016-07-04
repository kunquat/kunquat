

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2016
 *         Ossi Saresoja, Finland 2010-2013
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_FILTER_H
#define KQT_FILTER_H


#include <debug/assert.h>

#include <stdlib.h>


/* #define dprod(buf0, buf1, i0, i1, limit0, limit1, var, oper)            \ */
/*     if (true)                                                           \ */
/*     {                                                                   \ */
/*         for (; ((i0) < (limit0)) && ((i1) < (limit1)); ++(i0), ++(i1))  \ */
/*         {                                                               \ */
/*             (var) oper (buf0)[i0] * (buf1)[i1];                         \ */
/*         }                                                               \ */
/*     } else (void)0 */


/* #define cyclic_dprod(n, coeffs, bsize, buf, k, var, oper)                      \ */
/*     if (true)                                                                  \ */
/*     {                                                                          \ */
/*         int filter_i = 0;                                                      \ */
/*         int filter_j = (k) + ((bsize) - (n));                                  \ */
/*         dprod((coeffs), (buf), filter_i, filter_j, (n), (bsize), (var), oper); \ */
/*         filter_j -= (bsize);                                                   \ */
/*         dprod((coeffs), (buf), filter_i, filter_j, (n), (bsize), (var), oper); \ */
/*     } else (void)0 */


void one_pole_filter_create(double f, int bandform, double coeffs[1], double *mul);


void two_pole_bandpass_filter_create(
        double f1, double f2, double coeffs[2], double* mul);


void two_pole_filter_create(
        double f, double q, int bandform, double coeffs[2], double* mul);


void four_pole_bandpass_filter_create(
        double f1, double f2, double q, double coeffs[4], double* mul);


void butterworth_filter_create(
        int n, double f, int bandform, double coeffs[n], double* mul);


void butterworth_bandpass_filter_create(
        int n, double f1, double f2, double coeffs[2*n], double* mul);


double iir_filter_strict_cascade(
        int n, const double coeffs[n], double buf[n], double var);


static inline double iir_filter_strict_cascade_even_order(
        int n, const double coeffs[n], double buf[n], double var)
{
    rassert((n & 1) == 0);
    rassert(coeffs != NULL);
    rassert(buf != NULL);

    for (int i = 0; i < (n & ~((int)1)); i += 2)
    {
        var -= coeffs[i  ] * buf[i  ] +
               coeffs[i+1] * buf[i+1];
        buf[i  ] = buf[i+1];
        buf[i+1] = var;
    }

    return var;
}


double iir_filter_strict_transposed_cascade(
        int n, const double coeffs[n], double buf[n], double var);


double dc_zero_filter(int n, double buf[n], double var);


static inline double nq_zero_filter(int n, double buf[n], double var)
{
    rassert(buf != NULL);

    for (int i = 0; i < n; ++i)
    {
        const double temp = buf[i];
        buf[i] = var;
        var += temp;
    }

    return var;
}


double dc_nq_zero_filter(int n, double buf[2*n], double var, int* s);


double dc_pole_filter(int n, double buf[n], double var);


double nq_pole_filter(int n, double buf[n], double var);


#endif // KQT_FILTER_H


