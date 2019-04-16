/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * fft1.h
 *
 * Code generation for function 'fft1'
 *
 */

#ifndef FFT1_H
#define FFT1_H

/* Include files */
#include <cmath>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "rt_nonfinite.h"
#include "rtwtypes.h"
#include "envolope_types.h"

/* Function Declarations */
extern void b_r2br_r2dit_trig(const creal_T x[256], const double costab[129],
  const double sintab[129], creal_T y[256]);
extern void r2br_r2dit_trig(const creal_T x[199], const double costab[129],
  const double sintab[129], creal_T y[256]);
extern void r2br_r2dit_trig_impl(const creal_T x[100], int xoffInit, const
  double costab[129], const double sintab[129], creal_T y[256]);

#endif

/* End of code generation (fft1.h) */
