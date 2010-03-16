

/*
 * Copyright 2009 Tomi Jylhä-Ollila, Ossi Saresoja
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

#define BINOM_MAX 9

extern const double  bt[BINOM_MAX * (BINOM_MAX + 1) / 2];
extern const double nbt[BINOM_MAX * (BINOM_MAX + 1) / 2];

extern const double*    binom[BINOM_MAX];
extern const double* negbinom[BINOM_MAX];


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


#define dprod(buf0, buf1, i0, i1, limit0, limit1, var, oper)		\
  if (true)								\
    {									\
      for (; ((i0) < (limit0)) && ((i1) < (limit1)); ++(i0), ++(i0))	\
        {								\
	  (var) oper (buf0)[i0] * (buf1)[i1];				\
        }								\
    } else (void)0


#define cyclic_dprod(n, coeffs, bsize, buf, k, var, oper)		\
  if (true)								\
    {									\
      int filter_i = 0;							\
      int filter_j = (k) + ((bsize) - (n));				\
      dprod((coeffs), (buf), filter_i, filter_j, (n), (bsize), (var), oper); \
      filter_j -= (bsize);						\
      dprod((coeffs), (buf), filter_i, filter_j, (n), (bsize), (var), oper); \
    } else (void)0


#define fir_step(n, coeffs, bsize, buf, k, in, out)			\
  if (true)								\
    {									\
      (out) = (coeffs)[n] * (in);					\
      cyclic_dprod((n), (coeffs), (bsize), (buf), (k), (var), +=);	\
    } else (void)0


#define iir_step(n, coeffs, bsize, buf, k, in, out)			\
  if (true)								\
    {									\
      (out) = (in);							\
      cyclic_dprod((n), (coeffs), (bsize), (buf), (k), (var), -=);	\
    } else (void)0


#define fir_filter(n, coeffs, buf, k, in, out)			 \
  if (true)							 \
    {								 \
      fir_step((n),  (coeffs), (n),  (buf), (k), (in), (out));	 \
      (buf)[k]  = (in);						 \
      (k) = ((k) + 1) % (n);					 \
    } else (void)0


#define iir_filter_df1(n, coeffsa, coeffsb, bufa, bufb, k, in, out)	\
  if (true)								\
    {									\
      fir_step((n), (coeffsb), (n), (bufb), (k), (in), (out));		\
      (bufb)[k] = (in);							\
      iir_step((n), (coeffsa), (n), (bufa), (k), (in), (out));		\
      (bufa)[k] = (out);						\
      (k) = ((k) + 1) % (n);						\
    } else (void)0


#define iir_filter_df2(n, coeffsa, coeffsb, buf, k, in, out)		\
  if (true)                                                             \
    {									\
      iir_step((n), (coeffsa), (n),  (buf), (k), (var));		\
      (in) = (out);							\
      fir_step((n), (coeffsb), (n),  (buf), (k), (var));		\
      (buf)[k]  = (in);							\
      (k) = ((k) + 1) % (n);						\
    } else (void)0


#define iir_filter_strict(n, coeffs, buf, k, in, out)		\
  if (true)							\
    {								\
      iir_step((n),  (coeffs), (n),  (buf), (k), (in), (out));	\
      (buf)[k]  = (out);					\
      (k) = ((k) + 1) % (n);					\
    } else (void)0


#define power_law_filter(n, buf, in, out)			\
  if (true)							\
    {								\
      if ((n) > 0)						\
	{							\
	  (out) = (in);						\
	  for (int filter_i = 0; filter_i < (n); ++filter_i)	\
	    {							\
	      (out) += (buf)[filter_i];				\
	      (buf)[filter_i] = (out);				\
	    }							\
	}							\
      else							\
	{							\
	  for (int filter_i = 0; filter_i < (n); ++filter_i)	\
	    {							\
	      (out) = (in) - (buf)[filter_i];			\
	      (buf)[filter_i] = (in);				\
	      (in) = (out);					\
	    }							\
	}							\
    } else (void)0


void iir_filter_df1_old(int na,
                    int nb,
                    double* coeffsa,
                    double* coeffsb,
                    kqt_frame* histbufa,
                    kqt_frame* histbufb,
                    int nframes,
                    kqt_frame* inbuf,
                    kqt_frame* outbuf);


#endif // K_FILTER_H
