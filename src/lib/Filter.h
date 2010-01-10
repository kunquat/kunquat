

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
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


#include <kunquat/frame.h>


void bilinear_butterworth_lowpass_filter_create(int n,
                                                double f,
                                                double q,
                                                double coeffsa[],
                                                double coeffsb[]);

void bilinear_chebyshev_t1_lowpass_filter_create(int n,
                                                 double f,
                                                 double e,
                                                 double* coeffsa,
                                                 double* coeffsb);

void iir_filter_df1(int na,
                    int nb,
                    double* coeffsa,
                    double* coeffsb,
                    kqt_frame* histbuffa,
                    kqt_frame* histbuffb,
                    int amount,
                    kqt_frame* inbuff,
                    kqt_frame* outbuff);


void iir_filter_df2(int na,
                    int nb,
                    double* coeffsa,
                    double* coeffsb,
                    kqt_frame* histbuff,
                    int amount,
                    kqt_frame* inbuff,
                    kqt_frame* outbuff);


#endif // K_FILTER_H


