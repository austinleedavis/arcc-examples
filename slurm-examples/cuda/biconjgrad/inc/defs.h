#ifndef __DEFS_H
#define __DEFS_H

#include<stdio.h>

#ifdef USE_DOUBLE_PREC
  #define REAL_TYPE double
  #define CG_ERROR_TOLERANCE 0.01
#else
  #define REAL_TYPE float
  #define CG_ERROR_TOLERANCE 1E-01
#endif

#define CUDA_DEBUG

#define INVALID_COLUMN -1

const int CPUvsGPUmemWarnings = 0;  // Flag for warning about possible GPU/CPU memory sync problems

#endif
