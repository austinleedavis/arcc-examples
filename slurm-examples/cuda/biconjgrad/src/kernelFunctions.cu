#include "kernelFunctions.h"
#include "globalsingleton.h"

void checkCUDAError(const char *msg, int syncFlag) {
  if (syncFlag)
    // block until the device has completed                                                                                                    
    cudaThreadSynchronize();

  cudaError_t error = cudaGetLastError();

  if (cudaSuccess != error) {
    fprintf(stderr,"CUDA ERROR: %s: %s\n", msg, cudaGetErrorString(error));
    exit(1);
  }
}



/**
 *  A convenience function to be used on the GPU device to validate
 *  requested vector indices.
 **/
__device__ int gpuIsValidVectorIdx(const int idx, const int n) {
  return ( (idx >= 0) && (idx < n) );
}



//-----------------------------------------------------------------------------------------

/**
 *  Perform the GPU-based operation y=ax+y, where x and y are vectors
 *  and a is a scalar.  The routine uses only the GPU-based memory
 *  for vector values and so assumes the caller will copy CPU data
 *  into the GPU before calling this and copy results back when
 *  they are ready to do so.
 **/
__global__ void gpuSumAXplusThisKernel(const int n, 
				    const REAL_TYPE a, 
				    const REAL_TYPE *x, 
				    REAL_TYPE *y) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x; 
    if (gpuIsValidVectorIdx(idx,n))
      y[idx] += a*x[idx];
}

//extern "C" 
void launch_SumAXplusThisKernel(int BLOCK_N, int THREAD_N, const cudaStream_t &s, 
				const int n, const REAL_TYPE a, const REAL_TYPE *x, REAL_TYPE *y) {
  gpuSumAXplusThisKernel<<<BLOCK_N, THREAD_N, 0, s>>>(n,a,x,y);
}




//-----------------------------------------------------------------------------------------

/**
 *  Perform the GPU-based operation z=x+y, where x, y, and z are
 *  vectors.  The routine uses only the GPU-based memory for vector
 *  values and so assumes the caller will copy CPU data into the GPU
 *  before calling this and copy results back when they are ready to
 *  do so.
 **/
__global__ void gpuSumXplusYKernel(const int n, 
				   const REAL_TYPE *x, 
				   const REAL_TYPE *y, 
				   REAL_TYPE *z) {
  int idx = blockIdx.x * blockDim.x + threadIdx.x; 
  if (gpuIsValidVectorIdx(idx,n))
     z[idx] = x[idx] + y[idx];
}

//extern "C" 
void launch_SumXplusYKernel(int BLOCK_N, int THREAD_N, const cudaStream_t &s, 
			    const int n, const REAL_TYPE *x, const REAL_TYPE *y, REAL_TYPE *z) {
  gpuSumXplusYKernel<<<BLOCK_N, THREAD_N, 0, s>>>(n,x,y,z);
}




//-----------------------------------------------------------------------------------------

/**
 *  Perform the GPU-based operation y=x, where x and y are
 *  vectors.  The routine uses only the GPU-based memory for vector
 *  values and so assumes the caller will copy CPU data into the GPU
 *  before calling this and copy results back when they are ready to
 *  do so.
 **/
__global__ void gpuYeqXKernel(const int n, 
			      const REAL_TYPE *x, 
			      REAL_TYPE *y) {
  int idx = blockIdx.x * blockDim.x + threadIdx.x; 
  if (gpuIsValidVectorIdx(idx,n))
     y[idx] = x[idx];
}

//extern "C" 
void launch_YeqXKernel(int BLOCK_N, int THREAD_N, const cudaStream_t &s, 
			    const int n, const REAL_TYPE *x, REAL_TYPE *y) {
  gpuYeqXKernel<<<BLOCK_N, THREAD_N, 0, s>>>(n,x,y);
}



//-----------------------------------------------------------------------------------------

/**
 * Sum all values in an array on the GPU.  CAUTION:  This assumes only
 * one thread of one block is being used.  It's here simply as a convenience
 * for the GPU-based dot product routine to avoid copying the accumulated 
 * results back into CPU memory.
 **/
__global__ void gpuAccumulateKernel(const int n, const REAL_TYPE *x, REAL_TYPE *total) {
  total[0]=0.0;
  for (int idx=0.0; idx<n; idx++)
    total[0] += x[idx];
}

//extern "C" 
void launch_AccumulateKernel(int BLOCK_N, int THREAD_N, const cudaStream_t &s, 
			    const int n, const REAL_TYPE *x, REAL_TYPE *total) {
  gpuAccumulateKernel<<<BLOCK_N, THREAD_N, 0, s>>>(n,x,total);
}



/**
 * Perform most of a dot product operation on the GPU using only GPU-based
 * memory structures.  This routine will result in a reduced set of summed
 * values in z, which is merely an accumulator array of length equal to
 * the number of blocks in the grid.
 **/
__global__ void gpuDotProductKernel(const int n, const REAL_TYPE *x, const REAL_TYPE *y, REAL_TYPE *z ) { 
  // Shared buffer for accumulating individual thread results
  __shared__ REAL_TYPE cache[1024];//GlobalSingleton::getThreadsPerBlock()]; 

  int idx = threadIdx.x + blockIdx.x * blockDim.x; 
  int cacheIndex = threadIdx.x;
  REAL_TYPE temp = 0; 

  while (idx < n) {
    temp += x[idx] * y[idx]; 
    idx += blockDim.x * gridDim.x;
  }

  // set the cache values
  cache[cacheIndex] = temp;

  // Make sure all threads have completed their accumulation
  // from their respective blocks before proceeding (i.e.,
  // the cache is filled)
  __syncthreads();

  // For reductions, threadsPerBlock must be a power of 2 
  // because of the following code 
  int i = blockDim.x/2; 
  while (i != 0) {
    if (cacheIndex < i) 
      cache[cacheIndex] += cache[cacheIndex + i];
    __syncthreads(); 
    i /= 2;
  }

  if (cacheIndex == 0) 
    z[blockIdx.x] = cache[0];
}

//extern "C" 
void launch_DotProductKernel(int BLOCK_N, int THREAD_N, const cudaStream_t &s, 
			     const int n, const REAL_TYPE *x, const REAL_TYPE *y, REAL_TYPE *z) {
  gpuDotProductKernel<<<BLOCK_N, THREAD_N>>>(n,x,y,z);
}




//-----------------------------------------------------------------------------------------

/**
 * This is a GPU-based operation to scale a vector (in-place):  x = ax
 **/
__global__ void gpuScaleVectorKernel(const int n,
				   REAL_TYPE *x, 
				   const REAL_TYPE a) {
  int idx = blockIdx.x * blockDim.x + threadIdx.x; 
  if (gpuIsValidVectorIdx(idx,n))
     x[idx] = a * x[idx] ;
}

void launch_ScaleVectorKernel(int BLOCK_N, int THREAD_N, const cudaStream_t &s, 
			    const int n, REAL_TYPE *x, const REAL_TYPE a) {
  gpuScaleVectorKernel<<<BLOCK_N, THREAD_N, 0, s>>>(n,x,a);
}



//-----------------------------------------------------------------------------------------

/**
 * The is a GPU-based operation to produce a new vector by scaling
 * another vector:  y = ax
 **/
__global__ void gpuEqualsScaleVectorKernel(const int n,
					   REAL_TYPE *y, 
					   const REAL_TYPE a,
					   const REAL_TYPE *x) {
  int idx = blockIdx.x * blockDim.x + threadIdx.x; 
  if (gpuIsValidVectorIdx(idx,n))
     y[idx] = a * x[idx] ;
}

void launch_EqualsScaleVectorKernel(int BLOCK_N, int THREAD_N, const cudaStream_t &s, 
				    const int n, REAL_TYPE *y, const REAL_TYPE a, const REAL_TYPE *x) {
  gpuEqualsScaleVectorKernel<<<BLOCK_N, THREAD_N, 0, s>>>(n,y,a,x);
}




//-----------------------------------------------------------------------------------------

/**
 * This is a GPU-based operation to produce a new vector by adding two other
 * vectors:  z = x + y
 **/
__global__ void gpuEqualsXplusYKernel(const int n,
				      REAL_TYPE *z, 
				      const REAL_TYPE *x, 
				      const REAL_TYPE *y) {
  int idx = blockIdx.x * blockDim.x + threadIdx.x; 
  if (gpuIsValidVectorIdx(idx,n))
     z[idx] = x[idx] + y[idx];
}

void launch_EqualsXplusYKernel(int BLOCK_N, int THREAD_N, const cudaStream_t &s, 
			       const int n, REAL_TYPE *z, const REAL_TYPE *x, const REAL_TYPE *y) {
  gpuEqualsXplusYKernel<<<BLOCK_N, THREAD_N, 0, s>>>(n,z,x,y);
}




//-----------------------------------------------------------------------------------------

/**
 * Use the GPU to perform the operation Ax=y, where A is a matrix,
 * and x and y are vectors.  This routine uses only the on-board
 * GPU memory and expects that data from the CPU storage has already
 * been copied.  It DOES NOT copy the GPU-stored results structure
 * back to the CPU memory.
 **/
__global__ void gpuMultMatAbyVecXKernel(int numRows, 
					int numCols, 
					int numColsPerRow,
					int whichDevice,
					int vectorOffsetPos,
                                        int numComp,
					const int *indices, 
					const REAL_TYPE *data,
					REAL_TYPE *x, 
					REAL_TYPE *y) {
  int row = blockDim.x * blockIdx.x + threadIdx.x;
  int compRow = row + vectorOffsetPos;	
 
  if (row < numComp) {
    y[compRow] = 0.0;

    for (int idx=0; idx<numColsPerRow; idx++) {
      int pos = row*numColsPerRow + idx;
      int col = indices[pos];
      REAL_TYPE value = data[pos];

      if (col != INVALID_COLUMN)
        y[compRow] += value * x[col];
    }
  }
}

extern "C" 
void launch_MultMatAbyVecXKernel(int BLOCK_N, int THREAD_N, const cudaStream_t &s, 
				 int numRows, int numCols, int numColsPerRow,
				 int whichDevice, int vectorOffsetPos, int numComp, const int *indices, 
				 const REAL_TYPE *data, REAL_TYPE *x, REAL_TYPE *y) {
  gpuMultMatAbyVecXKernel<<<BLOCK_N, THREAD_N, 0, s>>>(numRows,
                                                       numCols, 
						       numColsPerRow,
						       whichDevice,
						       vectorOffsetPos,
                                                       numComp,
						       indices, data,
						       x, y);
}
