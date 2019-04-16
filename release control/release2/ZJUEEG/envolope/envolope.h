/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * envolope.h
 *
 * Code generation for function 'envolope'
 *
 */

#ifndef ENVOLOPE_H
#define ENVOLOPE_H

/* Include files */
#include <cmath>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "rt_nonfinite.h"
#include "rtwtypes.h"
#include "envolope_types.h"

/* Function Declarations */
extern void envolope(const double datain[100], double Fs, double data_hilbert
                     [100]);

#endif

/* End of code generation (envolope.h) */
