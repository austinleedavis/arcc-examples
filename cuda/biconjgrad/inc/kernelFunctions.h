#ifndef __KERNELFUNCTIONS_H
#define __KERNELFUNCTIONS_H

#include "defs.h"


void checkCUDAError(const char *msg, int syncFlag);


#ifdef __cplusplus
 extern "C" {
#endif 
  
// extern "C"
void launch_SumAXplusThisKernel(int BLOCK_N, int THREAD_N, const cudaStream_t &s, 
				const int n, const REAL_TYPE a, const REAL_TYPE *x, REAL_TYPE *y);

//extern "C" 
void launch_SumXplusYKernel(int BLOCK_N, int THREAD_N, const cudaStream_t &s, 
			    const int n, const REAL_TYPE *x, const REAL_TYPE *y, REAL_TYPE *z);

//extern "C" 
void launch_YeqXKernel(int BLOCK_N, int THREAD_N, const cudaStream_t &s, 
		       const int n, const REAL_TYPE *x, REAL_TYPE *y);

//extern "C" 
void launch_AccumulateKernel(int BLOCK_N, int THREAD_N, const cudaStream_t &s, 
			     const int n, const REAL_TYPE *x, REAL_TYPE *total);

//extern "C" 
void launch_DotProductKernel(int BLOCK_N, int THREAD_N, const cudaStream_t &s, 
			     const int n, const REAL_TYPE *x, const REAL_TYPE *y, REAL_TYPE *z);

//extern "C" 
void launch_ScaleVectorKernel(int BLOCK_N, int THREAD_N, const cudaStream_t &s, 
			      const int n, REAL_TYPE *x, const REAL_TYPE a);

//extern "C" 
void launch_EqualsScaleVectorKernel(int BLOCK_N, int THREAD_N, const cudaStream_t &s, 
				    const int n, REAL_TYPE *y, const REAL_TYPE a, const REAL_TYPE *x);

//extern "C" 
void launch_EqualsXplusYKernel(int BLOCK_N, int THREAD_N, const cudaStream_t &s, 
			       const int n, REAL_TYPE *z, const REAL_TYPE *x, const REAL_TYPE *y);

//extern "C" 
void launch_MultMatAbyVecXKernel(int BLOCK_N, int THREAD_N, const cudaStream_t &s, 
				 int numRows, int numCols, int numColsPerRow,
				 int whichDevice, int vectorOffsetPos, int numComp, const int *indices, 
				 const REAL_TYPE *data, REAL_TYPE *x, REAL_TYPE *y);

#ifdef __cplusplus
 }
#endif 

#endif
