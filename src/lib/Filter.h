

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
 *         Ossi Saresoja, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_FILTER_H
#define K_FILTER_H


#include <frame.h>


#define dprod(buf0, buf1, i0, i1, limit0, limit1, var, oper)            \
    if (true)                                                           \
    {                                                                   \
        for (; ((i0) < (limit0)) && ((i1) < (limit1)); ++(i0), ++(i1))  \
        {                                                               \
            (var) oper (buf0)[i0] * (buf1)[i1];                         \
        }                                                               \
    } else (void)0


#define cyclic_dprod(n, coeffs, bsize, buf, k, var, oper)                      \
    if (true)                                                                  \
    {                                                                          \
        int filter_i = 0;                                                      \
        int filter_j = (k) + ((bsize) - (n));                                  \
        dprod((coeffs), (buf), filter_i, filter_j, (n), (bsize), (var), oper); \
        filter_j -= (bsize);                                                   \
        dprod((coeffs), (buf), filter_i, filter_j, (n), (bsize), (var), oper); \
    } else (void)0


void two_pole_lowpass_filter_create(double f,
                                    double q,
                                    double coeffs[2],
                                    double *a0);


void butterworth_lowpass_filter_create(int n,
                                       double f,
                                       double coeffs[n],
                                       double *a0);


double iir_filter_strict_cascade(int n,
                                 double coeffs[n],
                                 double buf[n],
                                 double var);


double iir_filter_strict_transposed_cascade(int n,
                                            double coeffs[n],
                                            double buf[n],
                                            double var);


double dc_zero_filter(int n,
                      double buf[n],
                      double var);


double nq_zero_filter(int n,
                      double buf[n],
                      double var);


double dc_pole_filter(int n,
                      double buf[n],
                      double var);


double nq_pole_filter(int n,
                      double buf[n],
                      double var);


#endif // K_FILTER_H


