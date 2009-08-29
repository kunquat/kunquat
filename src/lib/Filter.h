

/*
 * Copyright 2009 Tomi Jylhä-Ollila
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


