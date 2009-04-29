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

double sinc(double x)
{
  return x == 0.0 ? 1.0 : sin(x)/x;
}

void simple_lowpass_fir_create(int n, double f, double coeffs[])
{
  int i;
  for(i=0;i<=n;++i)
    coeffs[i] = 2*f*sinc(PI*f*(2*i-n));
}

void bilinear_butterworth_order2_lowpass_iir__create(double f, double coeffsa[], double coeffsb[])
{
  f = 2*tan(PI*f);

  coeffsa[0] = 2*(f*f-4)/((f+2*sqrt(2))*f+4);
  coeffsa[1] = ((f-2*sqrt(2))*f+4)/((f+2*sqrt(2))*f+4);
  coeffsb[0] = f*f/((f+2*sqrt(2))*f+4);
  coeffsb[1] = f*f/((f+2*sqrt(2))*f+4);
  coeffsb[2] = f*f/((f+2*sqrt(2))*f+4);
}

void fir_filter(int n, double coeffs[], frame_t histbuff[], int amount, frame_t inbuff[], frame_t outbuff[])
{
  int i,j;
  double temp;

  for(i=0;i<amount;++i)
  {
    temp = 0.0;

    for(j=0;j<n-i;++j)
      temp += histbuff[j+i]*coeffs[j];

    for(;j<=n;++j)
      temp += inbuff[i-n+j]*coeffs[j];

    outbuff[i] = temp;
  }

  for(i=0;i<n-amount;++i)
    histbuff[i] = histbuff[amount+i];

  for(;i<n;++i)
    histbuff[i] = inbuff[amount-n+i];
}

void iir_filter_df1(int nb, double coeffsb[], frame_t histbuffb[], int na, double coeffsa[], frame_t histbuffa[], int amount, frame_t inbuff[], frame_t outbuff[])
{
  int i,j;
  double temp;

  for(i=0;i<amount;++i)
  {
    temp = 0.0;

    for(j=0;j<na-i;++j)
      temp -= histbuffa[j+i]*coeffsa[j];

    for(;j<na;++j)
      temp -= outbuff[i-na+j]*coeffsa[j];

    for(j=0;j<nb-i;++j)
      temp += histbuffb[j+i]*coeffsb[j];

    for(;j<=nb;++j)
      temp += inbuff[i-nb+j]*coeffsb[j];

    outbuff[i] = temp;
  }

  for(i=0;i<na-amount;++i)
    histbuffa[i] = histbuffa[amount+i];

  for(;i<na;++i)
    histbuffa[i] = outbuff[amount-na+i];

  for(i=0;i<nb-amount;++i)
    histbuffb[i] = histbuffb[amount+i];

  for(;i<nb;++i)
    histbuffb[i] = inbuff[amount-nb+i];
}


void iir_filter_df2(int na, double coeffsa[], int nb, double coeffsb[], frame_t histbuff[], int amount, frame_t inbuff[], frame_t outbuff[])
{
  int i,j;
  double temp;

  for(i=0;i<amount;++i)
  {
    temp = 0.0;

    for(j=0;j<na-i;++j)
      temp -= histbuff[j+i]*coeffsa[j];

    for(;j<na;++j)
      temp -= inbuff[i-na+j]*coeffsa[j];

    inbuff[i] += temp;

    temp = 0.0;

    for(j=0;j<nb-i;++j)
      temp += histbuff[j+i]*coeffsb[j];

    for(;j<=nb;++j)
      temp += inbuff[i-nb+j]*coeffsb[j];

    outbuff[i] = temp;
  }

  for(i=0;i<MAX(na,nb)-amount;++i)
    histbuff[i] = histbuff[amount+i];

  for(;i<MAX(na,nb);++i)
    histbuff[i] = inbuff[amount-na+i];
}


void iir_filter_pure(int n, double coeffs[], frame_t histbuff[], int amount, frame_t inbuff[], frame_t outbuff[])
{
  int i,j;
  double temp;

  for(i=0;i<amount;++i)
  {
    temp = 0.0;

    for(j=0;j<n-i;++j)
      temp -= histbuff[j+i]*coeffs[j];

    for(;j<n;++j)
      temp -= outbuff[i-n+j]*coeffs[j];

    temp += inbuff[i];

    outbuff[i] = temp;
  }

  for(i=0;i<n-amount;++i)
    histbuff[i] = histbuff[amount+i];

  for(;i<n;++i)
    histbuff[i] = outbuff[amount-n+i];
}
