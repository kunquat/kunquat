/*
 * Copyright 2009 Ossi Saresoja
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

#include <math.h>
#include <stdarg.h>
#include <string.h>

#define PI 3.14159265358979323846
#define MAX(x,y) ((x)>(y) ? (x) : (y))
#define MIN(x,y) ((x)<(y) ? (x) : (y))

double sinc(double x)
{
  return x == 0.0 ? 1.0 : sin(x)/x;
}

double powi(double x, int n)
{
  double y = 1.0;

  while(n != 0)
  {
    if(n & 1)
      y *= x;
    n >>= 1;
    x *= x;
  }
  return y;
}

int binom(int n, int k)
{
  return (k==0 || k==n) ? 1 : binom(n-1,k-1) + binom(n-1,k);
}

double poly(double x, int n, ...)
{
  va_list k;
  int i;
  double y;

  va_start(k, n);

  y = va_arg(k, double);
  for (i=0;i<n;++i)
    y = y*x + va_arg(k, double);
  va_end(k);
  return y;
}

#define DPROD(histbuff, sourcebuff, coeffs, n, i, acc, j, k, oper) { \
for((j)=0,(k)=(i);(k)<(n);++(j),++(k))                               \
  (acc) oper (histbuff)[(k)]*(coeffs)[(j)];                          \
                                                                     \
for((k)-=(n);(j)<(n);++(j),++(k))                                    \
  (acc) oper (sourcebuff)[(k)]*(coeffs)[(j)];                        \
}

#define BUFFER(histbuff, sourcebuff, n, amount) {                                \
if((amount)<(n)){                                                                \
  memmove(&(histbuff)[0], &(histbuff)[(amount)], ((n)-(amount))*sizeof(double)); \
  memcpy(&(histbuff)[(n)-(amount)], &(sourcebuff)[0], (amount)*sizeof(double));  \
}                                                                                \
 else                                                                            \
  memcpy(&(histbuff)[0], &(sourcebuff)[(amount)-(n)], (n)*sizeof(double));       \
}

void simple_lowpass_fir_create(int n, double f, double coeffs[])
{
  int i;
  for(i=0;i<=n;++i)
    coeffs[i] = 2*f*sinc(PI*f*(2*i-n));
}

#define C1 1.41421356237309504880 //sqrt(2)
#define C2 2.61312592975275305571 //sqrt(4+2*sqrt(2)) or equivalenty sqrt(2+sqrt(2))+sqrt(2-sqrt(2))
#define C3 2.23606797749978969641 //sqrt(5)
#define C4 1.73205080756887729353 //sqrt(3)
#define C5 2.44948974278317809820 //sqrt(6)

void bilinear_butterworth_lowpass_filter_create(int n, double f, double coeffsa[], double coeffsb[])
{
  int i;
  double a0=1.0,fna0;

    f = tan(PI*f);

  switch(n)
  {
  case 1:
    coeffsa[0] = poly(f, n, 1.0, -1.0);
    a0         = poly(f, n, 1.0,  1.0);
    break;
  case 2:
    coeffsa[0] = poly(f, n, 1.0, -C1,  1.0);
    coeffsa[1] = poly(f, n, 2.0, 0.0, -2.0);
    a0         = poly(f, n, 1.0,  C1,  1.0);
    break;
  case 3:
    coeffsa[0] = poly(f, n, 1.0, -2.0,  2.0, -1.0);
    coeffsa[1] = poly(f, n, 3.0, -2.0, -2.0,  3.0);
    coeffsa[2] = poly(f, n, 3.0,  2.0, -2.0, -3.0);
    a0         = poly(f, n, 1.0,  2.0,  2.0,  1.0);
    break;
  case 4:
    coeffsa[0] = poly(f, n, 1.0, -  C2,  2.0+  C1, -  C2,  1.0);
    coeffsa[1] = poly(f, n, 4.0, -2*C2,       0.0,  2*C2, -4.0);
    coeffsa[2] = poly(f, n, 6.0,   0.0, -4.0-2*C1,   0.0,  6.0);
    coeffsa[3] = poly(f, n, 4.0,  2*C2,       0.0, -2*C2, -4.0);
    a0         = poly(f, n, 1.0,    C2,  2.0+  C1,    C2,  1.0);
    break;
  case 5:
    coeffsa[0] = poly(f, n,  1.0, -1.0-  C3,  3.0+  C3, -3.0-  C3,  1.0+  C3, - 1.0);
    coeffsa[1] = poly(f, n,  5.0, -3.0-3*C3,  3.0+  C3,  3.0+  C3, -3.0-3*C3,   5.0);
    coeffsa[2] = poly(f, n, 10.0, -2.0-2*C3, -6.0-2*C3,  6.0+2*C3,  2.0+2*C3, -10.0);
    coeffsa[3] = poly(f, n, 10.0,  2.0+2*C3, -6.0-2*C3, -6.0-2*C3,  2.0+2*C3,  10.0);
    coeffsa[4] = poly(f, n,  5.0,  3.0+3*C3,  3.0+  C3, -3.0-  C3, -3.0-3*C3, - 5.0);
    a0         = poly(f, n,  1.0,  1.0+  C3,  3.0+  C3,  3.0+  C3,  1.0+  C3,   1.0);
    break;
  case 6:
    coeffsa[0] = poly(f, n,  1.0, -  C1-  C5,   4.0+2*C4, -3*C1-2*C5,   4.0+2*C4,  -  C1-  C5,   1.0);
    coeffsa[1] = poly(f, n,  6.0, -4*C1-4*C5,   8.0+4*C4,        0.0, - 8.0-4*C4,   4*C1+4*C5, - 6.0);
    coeffsa[2] = poly(f, n, 15.0, -5*C1-5*C5, - 4.0-2*C4,  9*C1+6*C5, - 4.0-2*C4,  -5*C1-5*C5,  15.0);
    coeffsa[3] = poly(f, n, 20.0,        0.0, -16.0-8*C4,        0.0,  16.0+8*C4,         0.0, -20.0);
    coeffsa[4] = poly(f, n, 15.0,  5*C1+5*C5, - 4.0-2*C4, -9*C1-6*C5, - 4.0-2*C4,   5*C1+5*C5,  15.0);
    coeffsa[5] = poly(f, n,  6.0,  4*C1+4*C5,   8.0+4*C4,        0.0, - 8.0-4*C4,  -4*C1-4*C5, - 6.0);
    a0         = poly(f, n,  1.0,    C1+  C5,   4.0+2*C4,  3*C1+2*C5,   4.0+2*C4,     C1+  C5,   1.0);
  }

  for(i=0;i<n;++i)
    coeffsa[i] /= a0;

  fna0 = powi(f,n)/a0;

  for(i=0;i<=n;++i)
    coeffsb[i] = binom(n,i)*fna0;
}

void bilinear_butterworth_highpass_filter_create(int n, double f, double coeffsa[], double coeffsb[])
{
  int i;
  
  bilinear_butterworth_lowpass_filter_create(n, 1.0-f, coeffsa, coeffsb);

  for(i=0;i<n;i+=2)
    coeffsa[i] = -coeffsa[i];

  for(i=1;i<=n;+=2)
    coeffsb[i] = -coeffsb[i];
}

void fir_filter(int n, double coeffs[], frame_t histbuff[], int amount, frame_t inbuff[], frame_t outbuff[])
{
  int i,j,k;
  double temp;

  for(i=0;i<amount;++i)
  {
    temp = inbuff[i]*coeffs[n];

    DPROD(histbuff, inbuff, coeffs, n, i, temp, j, k, +=);

    outbuff[i] = temp;
  }

  BUFFER(histbuff, inbuff, n, amount);
}

void iir_filter_df1(int na, double coeffsa[], frame_t histbuffa[], int nb, double coeffsb[], frame_t histbuffb[], int amount, frame_t inbuff[], frame_t outbuff[])
{
  int i,j,k;
  double temp;

  for(i=0;i<amount;++i)
  {
    temp = inbuff[i]*coeffsb[nb];

    DPROD(histbuffa, outbuff, coeffsa, na, i, temp, j, k, -=);

    DPROD(histbuffb,  inbuff, coeffsb, nb, i, temp, j, k, +=);

    outbuff[i] = temp;
  }

  BUFFER(histbuffa, outbuff, na, amount);

  BUFFER(histbuffb,  inbuff, nb, amount);
}


void iir_filter_df2(int na, double coeffsa[], int nb, double coeffsb[], frame_t histbuff[], int amount, frame_t inbuff[], frame_t outbuff[])
{
  int i,j,k;
  double temp;

  for(i=0;i<amount;++i)
  {
    temp = inbuff[i];

    DPROD(histbuff, inbuff, coeffsa, na, i, temp, j, k, -=);

    inbuff[i] = temp;

    temp *= coeffsb[nb];

    DPROD(histbuff, inbuff, coeffsb, nb, i, temp, j, k, +=);

    outbuff[i] = temp;
  }

  BUFFER(histbuff, inbuff, MAX(na,nb), amount);
}


void iir_filter_pure(int n, double coeffs[], frame_t histbuff[], int amount, frame_t inbuff[], frame_t outbuff[])
{
  int i,j,k;
  double temp;

  for(i=0;i<amount;++i)
  {
    temp = inbuff[i];

    DPROD(histbuff, outbuff, coeffs, n, i, temp, j, k, -=);

    outbuff[i] = temp;
  }

  BUFFER(histbuff, outbuff, n, amount);
}
