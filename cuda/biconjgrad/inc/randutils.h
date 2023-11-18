#ifndef __RANDUTILS_H
#define __RANDUTILS_H

/**
 * This module provides some simple random generation convenience routines.
 **/

#include "defs.h"
#include "vector.h"
#include "ellmatrix.h"

void initRand(int seed=-1);

REAL_TYPE genRandomReal(REAL_TYPE min, REAL_TYPE max);

int genRandomInt(int min, int max);

void testRandGen(int sampleSize);

void randomizeVector(Vector &vector,REAL_TYPE min, REAL_TYPE max);

void randomizeSymmetricPDELLMatrix(ELLMatrix &matrix,REAL_TYPE min, REAL_TYPE max);


#endif
