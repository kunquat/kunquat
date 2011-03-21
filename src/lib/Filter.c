

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


void simple_lowpass_fir_create(int n, double f, double *coeffs)
{
    for (int i = 0; i <= n; ++i)
    {
        coeffs[i] = 2 * f * sinc(PI * f * (2 * i - n));
    }
    return;
}


void one_pole_filter_create(double f,
			    int bandform,
			    double coeffs[1],
			    double *mul)

{
    assert(f < 0.5);
    assert(coeffs != NULL);
    assert(mul != NULL);
    //    static int created = 0;
    //    fprintf(stderr, "  %d \n", ++created);

    double sinf  = sin(PI * f);
    double cosf  = cos(PI * f);
    double a0        =  sinf + cosf      ;
           coeffs[0] = (sinf - cosf) / a0;
    *mul = (bandform == 0) ? sinf /a0 : cosf / a0;
    return;
}


void two_pole_bandpass_filter_create(double f1,
				     double f2,
				     double coeffs[2],
				     double *mul)
{
    assert(f1 < f2);
    assert(f2 < 0.5);
    assert(coeffs != NULL);
    assert(mul != NULL);
    //    static int created = 0;
    //    fprintf(stderr, "  %d \n", ++created);

    double sinm = sin(PI * (f2 - f1));
    double cosm = cos(PI * (f2 - f1));
    double cosp = cos(PI * (f2 + f1));
    double a0        =  cosm + sinm      ;
           coeffs[0] = (cosm - sinm) / a0;
           coeffs[1] =   - 2 * cosp  / a0;
	   *mul = sinm / a0;
    return;
}


void two_pole_filter_create(double f,
			    double q,
			    int bandform,
			    double coeffs[2],
			    double *mul)
{
    assert(f < 0.5);
    assert(q >= 0.5);
    assert(q <= 1000);
    assert(coeffs != NULL);
    assert(mul != NULL);
    //    static int created = 0;
    //    fprintf(stderr, "  %d \n", ++created);

    double q2    = 2 * q;
    double sinf  = sin(    PI * f);
    double cosf  = cos(    PI * f);
    double sin2f = sin(2 * PI * f);
    double cos2f = cos(2 * PI * f);
    double a0        =   1 + sin2f / q2      ;
           coeffs[0] =  (1 - sin2f / q2) / a0;
           coeffs[1] = - 2 * cos2f       / a0;
	   *mul = (bandform == 0) ? sinf * sinf / a0 : cosf * cosf / a0;
    return;
}


void four_pole_bandpass_filter_create(double f1,
				      double f2,
				      double q,
				      double coeffs[4],
				      double *mul)
{
    assert(f1 < f2);
    assert(f2 < 0.5);
    assert(q >= 0.5);
    assert(q <= 1000);
    assert(coeffs != NULL);
    assert(mul != NULL);
    //    static int created = 0;
    //    fprintf(stderr, "  %d \n", ++created);

    double q2    = 2 * q;
    double sinf  = sin(    PI * f);
    double cosf  = cos(    PI * f);
    double sin2f = sin(2 * PI * f);
    double cos2f = cos(2 * PI * f);
    double a0_temp   = (1.0 + f * f + f / q)          ;
           coeffs[0] = (1.0 + f * f - f / q) / a0_temp;
           coeffs[1] = (1.0 + f * f) * 2     / a0_temp;

           a0_temp   = (1.0 + f * f + f / q)          ;
           coeffs[2] = (1.0 + f * f - f / q) / a0_temp;
           coeffs[3] = (1.0 + f * f) * 2     / a0_temp;
	   *mul = ;
    return;
}


void butterworth_filter_create(int n,
			       double f,
			       int bandform,
			       double coeffs[n],
			       double *mul)

{
    assert(n >= 1);
    assert(f < 0.5);
    assert(coeffs != NULL);
    assert(mul != NULL);
    //    static int created = 0;
    //    fprintf(stderr, "  %d \n", ++created);

    double sinf  = sin(    PI * f);
    double cosf  = cos(    PI * f);
    double sin2f = sin(2 * PI * f);
    double cos2f = cos(2 * PI * f);
    double a0 = 1.0;
    for(int i = 0; i < (n & ~1); i += 2)
    {
        double sini = sin(PI / 2 / n * (i + 1));
	double a0_temp     =  1 + sini * sin2f           ;
               coeffs[i  ] = (1 - sini * sin2f) / a0_temp;
               coeffs[i+1] =       - 2 * cos2f  / a0_temp;
        a0 *= a0_temp;
    }
    if(n & 1)
    {
        double a0_temp     =  sinf + cosf           ;
               coeffs[n-1] = (sinf - cosf) / a0_temp;
        a0 *= a0_temp;
    }
    *mul = (bandform == 0) ? powi(sinf, n) /a0 : powi(cosf, n) / a0;
    return;
}

/*

q(s+1/s)=S
qs²-Ss+q=0

s=(S+-sqrt(S²-4q²))/2q=S/2q+-sqrt((S/2q)²-1)


1+S/Q+S²

S=(1/Q+-sqrt(1/Q²-4))/2=1/2Q+-sqrt((1/2Q)²-1)


p1=(s-S/2q-sqrt((S/2q)²-1))(s-S'/2q-sqrt((S'/2q)²-1))
=s²+s(-Re(S)/q-2Re(sqrt((S/2q)²-1)))+(1/4q²+Re(sqrt((1/2q)²-S²))/q+sqrt((1-2Re(S²))/8q⁴-1)


p2=(s-S/2q+sqrt((S/2q)²-1))(s-S'/2q+sqrt((S'/2q)²-1))
=s²+s(-Re(S)/q+2Re(sqrt((S/2q)²-1)))+(1/4q²-Re(sqrt((1/2q)²-S²))/q+sqrt((1-2Re(S²))/8q⁴-1)

*/

/* void four_pole_bandpass_filter_create(double f1, */
/* 				      double f2, */
/* 				      double q, */
/* 				      double coeffs[4], */
/* 				      double *a0) */
/* { */
/*   assert(f1 < 0.5); */
/*   assert(f2 < 0.5); */
/*   assert(f1 < f2); */
/*   assert(q >= 0.5); */
/*   assert(q <= 1000); */
/*   assert(coeffs != NULL); */
/*   assert(a0 != NULL); */
/*   //    static int created = 0; */
/*   //    fprintf(stderr, "  %d \n", ++created); */

/*   f1 = tan(PI * f1); //Prewarp */
/*   f2 = tan(PI * f2); //Prewarp */
/*   double ff = 2 * (f2 * f1); */
/*   double df = 2 * (f2 - f1); */
/*   double a0_temp   = ((ff * (ff + 2.0) + df * df + 1.0) + (ff + 1.0) * (df / q))          ;  */
/*          coeffs[3] = 2 * (2 * (1.0 - ff) + (ff * df - df / q))                   / a0_temp; */
/*          coeffs[2] = 2 * (ff * (3 * ff - 2.0) + 3.0 - df * df)                   / a0_temp; */
/*          coeffs[1] = 2 * (2 * (1.0 - ff) - (ff * df - df / q))                   / a0_temp; */
/*          coeffs[0] = ((ff * (ff + 2.0) + df * df + 1.0) - (ff + 1.0) * (df / q)) / a0_temp; */
/*   *a0 = a0_temp / (df * df); */
/*   return; */
/* } */


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
        var -= coeffs[i  ] * buf[i  ] + 
	       coeffs[i+1] * buf[i+1];
        buf[i  ] = buf[i+1];
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
        buf[i  ] =        - coeffs[i  ] * var;
    }
    if (n & 1)
    {
        var += buf[n-1];
        buf[n-1] =        - coeffs[n-1] * var;
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


double dc_nq_zero_filter(int n,
			 double buf[2*n],
			 double var,
			 int *s)
{
    var = dc_zero_filter(n, buf + (*s & n), var);
    *s = ~*s;
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


