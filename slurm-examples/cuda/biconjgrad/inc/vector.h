#ifndef __VECTOR_H
#define __VECTOR_H

/**
 * This module implements routines to manipulate a basic vector
 * data structure, including operations performed on the GPU.  The
 * structure contains data components both for vector storage in
 * CPU memory, as well as vector storage in GPU memory. 
 **/

#include "defs.h"
#include "gpumeminterface.h"

#include<cuda_runtime_api.h>


class Vector {
protected:
  cudaError_t error;
  int numDevices;        // number of GPU devices employed
  int size;              // number of elements in the vector
  GPUMemInterface<REAL_TYPE> *data[10];  // Should be dynamic ... but as long as there are fewer than 

public:
  int dirtyCPU;          // flag indicating that CPU memory contents have changed
  int dirtyGPU;          // flag indicating that GPU memory contents have changed

  Vector(const Vector &x);
  Vector(const int inSize, const int inNumDevices=1,const REAL_TYPE inDefVal=0.0);
  ~Vector();

  REAL_TYPE *getGPUDataPtr(int idx) {return data[idx]->getGPUDataPtr();}		
  int getSize() const {return size;}		

  int copy(const Vector &x);
  void copyGPUContents(const Vector &x);

  void copyToGPU();
  void copyFromGPU();

  int getNumDevices() {return numDevices;}
  REAL_TYPE getValue(const int idx) const;
  void setValue(const int idx, const REAL_TYPE value);
  void print(const char *prefix) const;

  void sumAXplusThis(const Vector &x, const REAL_TYPE a);
  void sumXplusY(const Vector &x, const Vector &y);
  REAL_TYPE dotWith(const Vector &x);
  void scale(const REAL_TYPE a);
  void equalsScaleX(const REAL_TYPE a, const Vector &x);
  void equalsXplusY(const Vector &x, const Vector &y);
  void mergeOnGPU();
};

#endif
