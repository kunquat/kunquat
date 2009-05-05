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

#define PI  3.14159265358979323846
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

void simple_lowpass_fir_create(int n, double f, double coeffs[])
{
  int i;
  for(i=0;i<=n;++i)
    coeffs[i] = 2*f*sinc(PI*f*(2*i-n));
}

void bilinear_butterworth_lowpass_iir__create(int n, double f, double coeffsa[], double coeffsb[])
{
  int i;
  double a0=1.0,fna0;

  f = 2*tan(PI*f);

  switch(n)
  {
  case 1:
    a0 = f+2;
    coeffsa[0] = f-2;
    break;
  case 2:
    a0 = (f+2*sqrt(2))*f+4;
    coeffsa[0] = (f-2*sqrt(2))*f+4;
    coeffsa[1] = 2*f*f-8;
    break;
  case 3:
    a0 = ((f+4)*f+8)*f+8;
    coeffsa[0] = ((f-4)*f+8)*f-8;
    coeffsa[1] = ((3*f-4)*f-8)*f+24;
    coeffsa[2] = ((3*f+4)*f-8)*f-24;
    break;
  case 4:
    a0 = (((f+2*sqrt(4+2*sqrt(2)))*f+8+4*sqrt(2))*f+8*sqrt(4+2*sqrt(2)))*f+16;
    coeffsa[0] = (((f-2*sqrt(4+2*sqrt(2)))*f+8+4*sqrt(2))*f-8*sqrt(4+2*sqrt(2)))*f+16;
    coeffsa[1] = ((4*f-4*sqrt(4+2*sqrt(2)))*f*f+16*sqrt(4+2*sqrt(2)))*f-64;
    coeffsa[2] = (6*f*f-16-8*sqrt(2))*f*f+96;
    coeffsa[3] = ((4*f+4*sqrt(4+2*sqrt(2)))*f*f-16*sqrt(4+2*sqrt(2)))*f-64;
  }

  for(i=0;i<n;++i)
    coeffsa[i] /= a0;

  fna0 = powi(f,n)/a0;

  for(i=0;i<=n;++i)
    coeffsb[i] = binom(n,i)*fna0;
}

void fir_filter(int n, double coeffs[], frame_t histbuff[], int amount, frame_t inbuff[], frame_t outbuff[])
{
  int i,j,k;
  double temp;

  for(i=0;i<amount;++i)
  {
    temp = 0.0;

    for(j=0,k=i;k<n;++j,++k)
      temp += histbuff[k]*coeffs[j];

    for(k-=n;j<=n;++j,++k)
      temp += inbuff[k]*coeffs[j];

    outbuff[i] = temp;
  }

  for(i=0,j=amount;j<n;++i,++j)
    histbuff[i] = histbuff[j];

  for(j=amount-i;i<n;++i,++j)
    histbuff[i] = inbuff[j];
}

void iir_filter_df1(int na, double coeffsa[], frame_t histbuffa[], int nb, double coeffsb[], frame_t histbuffb[], int amount, frame_t inbuff[], frame_t outbuff[])
{
  int i,j,k;
  double temp;

  for(i=0;i<amount;++i)
  {
    temp = 0.0;

    for(j=0,k=i;k<na;++j,++k)
      temp -= histbuffa[k]*coeffsa[j];

    for(k-=na;j<=na;++j,++k)
      temp -= outbuff[k]*coeffsa[j];

    for(j=0,k=i;k<nb;++j,++k)
      temp += histbuffb[k]*coeffsb[j];

    for(k-=nb;j<=nb;++j,++k)
      temp += inbuff[k]*coeffsb[j];

    outbuff[i] = temp;
  }

  for(i=0,j=amount;j<na;++i,++j)
    histbuffa[i] = histbuffa[j];

  for(j=amount-i;i<na;++i,++j)
    histbuffa[i] = outbuff[j];

  for(i=0,j=amount;j<nb;++i,++j)
    histbuffb[i] = histbuffb[j];

  for(j=amount-i;i<nb;++i,++j)
    histbuffb[i] = inbuff[j];
}


void iir_filter_df2(int na, double coeffsa[], int nb, double coeffsb[], frame_t histbuff[], int amount, frame_t inbuff[], frame_t outbuff[])
{
  int i,j,k;
  double temp;

  for(i=0;i<amount;++i)
  {
    temp = 0.0;

    for(j=0,k=i;k<na;++j,++k)
      temp -= histbuff[k]*coeffsa[j];

    for(k-=na;j<=na;++j,++k)
      temp -= inbuff[k]*coeffsa[j];

    inbuff[i] += temp;

    temp = 0.0;

    for(j=0,k=i;k<nb;++j,++k)
      temp += histbuff[k]*coeffsb[j];

    for(k-=nb;j<=nb;++j,++k)
      temp += inbuff[k]*coeffsb[j];

    outbuff[i] = temp;
  }

  for(i=0,j=amount;j<MAX(na,nb);++i,++j)
    histbuff[i] = histbuff[j];

  for(j=amount-i;i<MAX(na,nb);++i,++j)
    histbuff[i] = inbuff[j];
}


void iir_filter_pure(int n, double coeffs[], frame_t histbuff[], int amount, frame_t inbuff[], frame_t outbuff[])
{
  int i,j,k;
  double temp;

  for(i=0;i<amount;++i)
  {
    temp = 0.0;

    for(j=0,k=i;k<n;++j,++k)
      temp -= histbuff[k]*coeffs[j];

    for(k-=n;j<=n;++j,++k)
      temp -= outbuff[k]*coeffs[j];

    temp += inbuff[i];

    outbuff[i] = temp;
  }

  for(i=0,j=amount;j<n;++i,++j)
    histbuff[i] = histbuff[j];

  for(j=amount-i;i<n;++i,++j)
    histbuff[i] = outbuff[j];
}
