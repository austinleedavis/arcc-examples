#ifndef __DATABUILDER_H
#define __DATABUILDER_H

/**
 * This module provides functionality for manipulating a sparse, ELL formatted
 * matrix data structure, including GPU-based operations.  The structure consists
 * of two 2D structures, one for storing column indices and one for storing
 * matrix values.  Each of these has as many rows as the real, mathematical matrix
 * has, but only has numCollsPerRow number of columns ... and is most efficient
 * when most rows have roughly the same number of non-zero values in them.
 **/

#include "defs.h"
#include "vector.h"
#include "ellmatrix.h"

void setDBNumberOfDevices(int numDevices);
int getDBNumberOfDevices();

int countValuesPerLine(const char *fileName, int printLevel=0);
ELLMatrix *readMatrixFromFile(const char *fileName, int systemSize, int &numValsRead, int printLevel=0);
ELLMatrix *readMatrixFromELLFile(const char *fileName, int &numValsRead, int printLevel=0);
Vector *readVectorFromFile(const char *fileName, int systemSize, int &numValsRead, int printLevel=0, int *indices=NULL);
int writeVectorToFile(const char *fileName, Vector &vector, int printLevel);
int readIndicesFromFile(const char *fileName, int systemSize, int *indices, int printLevel=0);


#endif
