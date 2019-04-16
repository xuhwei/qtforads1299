/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * envolope.cpp
 *
 * Code generation for function 'envolope'
 *
 */

/* Include files */
#include "rt_nonfinite.h"
#include "envolope.h"
#include "ifft.h"
#include "fft.h"

/* Function Declarations */
static double rt_hypotd_snf(double u0, double u1);

/* Function Definitions */
static double rt_hypotd_snf(double u0, double u1)
{
  double y;
  double a;
  double b;
  a = std::abs(u0);
  b = std::abs(u1);
  if (a < b) {
    a /= b;
    y = b * std::sqrt(a * a + 1.0);
  } else if (a > b) {
    b /= a;
    y = a * std::sqrt(b * b + 1.0);
  } else if (rtIsNaN(b)) {
    y = b;
  } else {
    y = a * 1.4142135623730951;
  }

  return y;
}

void envolope(const double datain[100], double, double data_hilbert[100])
{
  creal_T dcv0[100];
  creal_T x[100];
  int i;
  fft(datain, dcv0);
  memcpy(&x[0], &dcv0[0], 100U * sizeof(creal_T));
  for (i = 0; i < 49; i++) {
    x[i + 1].re *= 2.0;
    x[i + 1].im *= 2.0;
    x[i + 51].re = 0.0;
    x[i + 51].im = 0.0;
  }

  ifft(x, dcv0);
  for (i = 0; i < 100; i++) {
    x[i] = dcv0[i];
    data_hilbert[i] = rt_hypotd_snf(x[i].re, x[i].im);
  }

  /* { */
  /* function [data_hilbert,data_wavelet] = envolope(datain, Fs) */
  /*  */
  /* data_hilbert = abs(hilbert(datain)) */
  /*  */
  /* ev= abs(cwt(datain,'amor',Fs)) */
  /* [m,n]=find(ev == max(max(ev))) */
  /* data_wavelet = ev(m,:) */
  /* } */
}

/* End of code generation (envolope.cpp) */
