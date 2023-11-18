#ifndef __ELLMATRIX_H
#define __ELLMATRIX_H

/**
 * This module provides functionality for manipulating a sparse, ELL formatted
 * matrix data structure, including GPU-based operations.  The structure consists
 * of two 2D structures, one for storing column indices and one for storing
 * matrix values.  Each of these has as many rows as the real, mathematical matrix
 * has, but only has numCollsPerRow number of columns ... and is most efficient
 * when most rows have roughly the same number of non-zero values in them.
 **/

#include "defs.h"
#include "gpumeminterface.h"
#include "vector.h"

#include<cuda_runtime_api.h>

class ELLMatrix {
protected:
  cudaError_t error;
  int numDevices;
  int size;              // number of elements in data nd indicies
  int numRows;           // number of rows in the mathematical matrix
  int numCols;           // number of columns in the mathematical matrix
  int numColsPerRow;     // maximum number of elements in any row
  GPUMemInterface<int> *indices[10];
  GPUMemInterface<REAL_TYPE> *data[10];
  
public:
  int dirtyCPU;          // flag indicating that CPU memory contents have changed
  int dirtyGPU;          // flag indicating that GPU memory contents have changed

  ELLMatrix(int inNumRows, int inNumCols, int inNumColsPerRow, int inNumDevices);
  ~ELLMatrix();
 
  int getNumRows() {return numRows;}		
  int getNumCols() {return numCols;}		
  int getNumColsPerRow() {return numColsPerRow;}		
  int getSize() {return size;}		
  		
  void copyToGPU();
  void copyFromGPU();

  REAL_TYPE getValue(int row, int col) const;
  int setValue(int row, int col, REAL_TYPE value);
  void rawSetValue(int rawIdx, int col, REAL_TYPE value);
  
  void print(int reduceZeros=1) const;
  void printInSparseFormat() const;
  void printRaw(const char *matrixName) const;
				
  void multMatAbyVecX(Vector &x, Vector &y,int useCPUMultiplier=0);
};

#endif
