#include "vector.h"
#include "kernelFunctions.h"
#include "globalsingleton.h"

#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<memory>


typedef GPUMemInterface<REAL_TYPE> *interface_ptr_t;

/**
 * Create the vector data structures, initializing all data elements to
 * defVal (which defaults to 0.0).  Memory is allocated for storage on both
 * GPU and CPU, but GPU values are NOT initialized.
 **/
Vector::Vector(const int inSize, const int inNumDevices, const REAL_TYPE inDefVal) {
  size = inSize;
  if (size > inNumDevices) numDevices = inNumDevices;
  else                     numDevices = 1;

  dirtyCPU = 1;
  dirtyGPU = 0;

//  data = new interface_ptr_t[numDevices];
  for (int idx=0; idx<numDevices; idx++) {
    data[idx] = new GPUMemInterface<REAL_TYPE>(size,idx,1);
    for (int jdx=0; jdx<size; jdx++) 
      data[idx]->setHostValue(jdx,inDefVal);
  }
}


/**
 * Create the a new vector data structure, copying size and values stored in
 * CPU memory from another vector.  Memory is allocated for storage on both
 * GPU and CPU, but GPU values are NOT initialized.
 **/
Vector::Vector(const Vector &x) {
  if ( (x.dirtyGPU) && CPUvsGPUmemWarnings )
    printf("WARNING:  Attempting to clone vector using CPU memory contents, but the source vector has had GPU operations since the last GPU copy.\n");
    
  size = x.size;
  numDevices = x.numDevices;
  dirtyCPU = 1;
  dirtyGPU = 0;

//  data = new interface_ptr_t[numDevices];
  for (int idx=0; idx<numDevices; idx++) {
    data[idx] = new GPUMemInterface<REAL_TYPE>(size,idx);
    for (int jdx=0; jdx<size; jdx++) {
      REAL_TYPE value = x.getValue(jdx);
      data[idx]->setHostValue(jdx,value);
    }
  }
}


/**
 * Destroy the vector data structure, freeing both CPU and GPU memory used
 * and setting internal member values to 0 or NULL.
 **/
Vector::~Vector() {
  for (int idx=0; idx<numDevices; idx++)
    if (data[idx] != NULL) {
      delete data[idx];
      data[idx] = NULL;
    }
    
//  delete [] data;
  
  numDevices = 0;
  size = 0;
  dirtyCPU = 1;
  dirtyGPU = 1;
}




/**
 * Copy values from CPU memory structures, y=x.
 **/
int Vector::copy(const Vector &x) {
  if (size != x.size) return 0;

  if ( (x.dirtyGPU) && CPUvsGPUmemWarnings )
    printf("WARNING:  Attempting to copy vector content from CPU memory, but the source vector has had GPU operations since the last GPU copy.\n");

  dirtyCPU = 1;
  dirtyGPU = 0;
  
  for (int idx=0; idx<size; idx++) 
    setValue(idx,x.getValue(idx));
    
  return 1;
}



/**
 *  Copy vector values stored in the CPU memory structure to the GPU memory structure.
 **/
void Vector::copyToGPU() {
  for (int idx=0; idx<numDevices; idx++)
    data[idx]->copyToGPU();

  dirtyCPU = 0;
  dirtyGPU = 0;
}


/**
 *  Copy vector values stored in the GPU memory structure to the CPU memory structure.
 **/
void Vector::copyFromGPU() {
  for (int idx=0; idx<numDevices; idx++)
    data[idx]->copyFromGPU();

  dirtyCPU = 0;
  dirtyGPU = 0;
}


/**
 * Pretty-print the vector.
 **/
void Vector::print(const char *prefix) const {
  if ( (dirtyGPU)  && CPUvsGPUmemWarnings )
    printf("WARNING:  Printing vector contents from CPU memory, but there have been GPU operations since last GPU memory copy.\n");
  
  printf("%s[ ",prefix);
  for (int idx=0; idx<size; idx++) {
    REAL_TYPE value = getValue(idx);
    if (value == 0.0) printf("0 ");
    else printf("%0.4lf ",value);
  }
  printf(" ]\n");
}


/**
 * Get the vector value at the specified position (from CPU memory)
 **/
REAL_TYPE Vector::getValue(const int idx) const {
  if ( (dirtyGPU) && CPUvsGPUmemWarnings )
    printf("WARNING:  Getting value form Vector CPU memory, but there have been GPU operations since last GPU memory copy.\n");

  REAL_TYPE value = 0.0;

  if ( (idx >= 0) && (idx < size) )
    // Since the vector is copied to all devices, just take the one from the first device
    value = data[0]->getHostValue(idx);  
  return value;
}


/**
 * Set the vector value at the specified position (in CPU memory)
 **/
void Vector::setValue(const int idx, const REAL_TYPE value) {
  dirtyCPU = 1;
  
  if ( (idx >= 0) && (idx < size) )
    // We keep a copy of every value on every device
    for (int jdx=0; jdx<numDevices; jdx++) 
      data[jdx]->setHostValue(idx,value);
}


/**
 *  A CPU-based wrapper around the GPU-based operation y=ax+y, where x
 *  and y are vectors and a is a scalar.  The routine uses only the
 *  GPU-based memory for vector values and so assumes the caller will
 *  copy CPU data into the GPU before calling this and copy results
 *  back when they are ready to do so.
 **/
void Vector::sumAXplusThis(const Vector &x, const REAL_TYPE a) {
  if ( (x.dirtyCPU) && CPUvsGPUmemWarnings )
    printf("WARNING:  Executing x+ay=y GPU operation, but some contents of x have changed in CPU memory and have not been copied to the GPU.\n");

  if ( (dirtyCPU) && CPUvsGPUmemWarnings )
    printf("WARNING:  Executing x+ay=y GPU operation, but some contents of y have changed in CPU memory and have not been copied to the GPU.\n");
  
  for (int idx=0; idx<numDevices; idx++) {
    cudaSetDevice(idx);    
    launch_SumAXplusThisKernel(GlobalSingleton::getBlocksPerGrid(),GlobalSingleton::getThreadsPerBlock(),data[idx]->getStream(),
			       size, a, x.data[idx]->getGPUDataPtr(), 
			       data[idx]->getGPUDataPtr());
  }
  

  for (int idx=0; idx<numDevices; idx++) {
    cudaSetDevice(idx);
    data[idx]->streamSync();
  }
  
  #ifdef CUDA_DEBUG
    // check if kernel execution generated an error
    // Check for any CUDA errors
    checkCUDAError("gpuSumAXplusThisKernel invocation",1);
  #endif

  // Indicate that the contents of y changed in GPU memory but have
  // not yet been updated in CPU memory
  dirtyGPU = 1;
}



/**
 *  A CPU-based wrapper around the GPU-based operation z=x+y, where x,
 *  y, and z are vectors.  The routine uses only the GPU-based memory
 *  for vector values and so assumes the caller will copy CPU data
 *  into the GPU before calling this and copy results back when they
 *  are ready to do so.
 **/
void Vector::sumXplusY(const Vector &x, const Vector &y) {
  if ( (x.dirtyCPU) && CPUvsGPUmemWarnings )
    printf("WARNING:  Executing x+y=z GPU operation, but some contents of x have changed in CPU memory and have not been copied to the GPU.\n");

  if ( (y.dirtyCPU) && CPUvsGPUmemWarnings )
    printf("WARNING:  Executing x+y=z GPU operation, but some contents of y have changed in CPU memory and have not been copied to the GPU.\n");
  
  for (int idx=0; idx<numDevices; idx++) {
    cudaSetDevice(idx);    
    launch_SumXplusYKernel(GlobalSingleton::getBlocksPerGrid(),GlobalSingleton::getThreadsPerBlock(),data[idx]->getStream(), 
			   size, x.data[idx]->getGPUDataPtr(), 
			   y.data[idx]->getGPUDataPtr(), data[idx]->getGPUDataPtr());
  }
  
  for (int idx=0; idx<numDevices; idx++) {
    cudaSetDevice(idx);    
    data[idx]->streamSync();
  }

  #ifdef CUDA_DEBUG
    // check if kernel execution generated an error
    // Check for any CUDA errors
    checkCUDAError("gpuSumXplusYKernel invocation",1);
  #endif

  // Indicate that the contents of z changed in GPU memory but have
  // not yet been updated in CPU memory
  dirtyGPU = 1;
}



void Vector::copyGPUContents(const Vector &x) {
  if ( (x.dirtyCPU) && CPUvsGPUmemWarnings )
    printf("WARNING:  Copying contents vector directly in GPU memory, but some contents of source vector have changed in CPU memory and have not been copied to the GPU.\n");

  for (int idx=0; idx<numDevices; idx++) {
    cudaSetDevice(idx);    
    launch_YeqXKernel(GlobalSingleton::getBlocksPerGrid(),GlobalSingleton::getThreadsPerBlock(),data[idx]->getStream(),
		      size, x.data[idx]->getGPUDataPtr(), data[idx]->getGPUDataPtr());
  }
  
  for (int idx=0; idx<numDevices; idx++) {
    cudaSetDevice(idx);    
    data[idx]->streamSync();
  }
  
  #ifdef CUDA_DEBUG
    // check if kernel execution generated an error
    // Check for any CUDA errors
    checkCUDAError("gpuYeqXKernel invocation",1);
  #endif

  // Indicate that the contents of y changed in GPU memory but have
  // not yet been updated in CPU memory
  dirtyGPU = 1;
}


/**
 * This is a CPU-based wrapper around the GPU-based dot product
 **/
REAL_TYPE Vector::dotWith(const Vector &x) {
  if ( (x.dirtyCPU) && CPUvsGPUmemWarnings )
    printf("WARNING:  Executing <x,y> GPU operation, but some contents of x have changed in CPU memory and have not been copied to the GPU.\n");

  if ( (dirtyCPU) && CPUvsGPUmemWarnings )
    printf("WARNING:  Executing <x,y> GPU operation, but some contents of y have changed in CPU memory and have not been copied to the GPU.\n");


  REAL_TYPE *deviceAccum;
  REAL_TYPE *deviceTotal;
  REAL_TYPE total;

  cudaSetDevice(0);    

  // Allocate temporary memory on the GPU for accumulating results 
  cudaMalloc( (void**)&deviceAccum, GlobalSingleton::getBlocksPerGrid() * sizeof(REAL_TYPE) );
  cudaMalloc( (void**)&deviceTotal, 1 * sizeof(REAL_TYPE) );

  // Perform *most* of the dot product, reducing sums into
  // an array only as large as the number of blocks in the grid  
  launch_DotProductKernel(GlobalSingleton::getBlocksPerGrid(),GlobalSingleton::getThreadsPerBlock(),data[0]->getStream(),
			  x.size, x.data[0]->getGPUDataPtr(), 
			  data[0]->getGPUDataPtr(), deviceAccum);

  #ifdef CUDA_DEBUG
    // check if kernel execution generated an error
    // Check for any CUDA errors
    checkCUDAError("gpuDotProductKernel invocation",1);
  #endif

  // Use a single thread to clean up the rest of the accumulation
  // to avoid copying all that back to the CPU
  launch_AccumulateKernel(1,1,data[0]->getStream(),
			  GlobalSingleton::getBlocksPerGrid(),deviceAccum,deviceTotal);

  #ifdef CUDA_DEBUG
    // check if kernel execution generated an error
    // Check for any CUDA errors
    checkCUDAError("gpuAccumulateKernel invocation",1);
  #endif

  // Copy back only the final total value
  cudaMemcpy(&total, deviceTotal, 1*sizeof(REAL_TYPE), cudaMemcpyDeviceToHost);

  cudaFree(deviceAccum);
  cudaFree(deviceTotal);
  
  return total;
}


/**
 * This is a CPU-based wrapper around the GPU-based operation to
 * scale a vector (in-place) ... x = ax.
 **/
void Vector::scale(const REAL_TYPE a) {
  if ( (dirtyCPU) && CPUvsGPUmemWarnings )
    printf("WARNING:  Executing ax GPU operation, but some contents of x have changed in CPU memory and have not been copied to the GPU.\n");

  for (int idx=0; idx<numDevices; idx++) {
    cudaSetDevice(idx);    
    launch_ScaleVectorKernel(GlobalSingleton::getBlocksPerGrid(), GlobalSingleton::getThreadsPerBlock(), data[idx]->getStream(),
			     size, data[idx]->getGPUDataPtr(), a);
  }

  #ifdef CUDA_DEBUG
    // check if kernel execution generated an error
    // Check for any CUDA errors
    checkCUDAError("gpuScaleVectorKernel invocation",1);
  #endif

  for (int idx=0; idx<numDevices; idx++) {
    cudaSetDevice(idx);    
    data[idx]->streamSync();
  }

  // Indicate that the contents of x changed in GPU memory but have
  // not yet been updated in CPU memory
  dirtyGPU = 1;
}


void Vector::equalsScaleX(const REAL_TYPE a, const Vector &x) {
  if ( (x.dirtyCPU) && CPUvsGPUmemWarnings )
    printf("WARNING:  Executing y=ax GPU operation, but some contents of x have changed in CPU memory and have not been copied to the GPU.\n");

  for (int idx=0; idx<numDevices; idx++) {
    cudaSetDevice(idx);    
    // printf("DBG:  GlobalSingleton::getBlocksPerGrid()=%d, GlobalSingleton::getThreadsPerBlock()=%d, stream=%d, size=%d, dev=%d\n",
    // 	   GlobalSingleton::getBlocksPerGrid(),GlobalSingleton::getThreadsPerBlock(),data[idx]->getStream(),size,idx);
    launch_EqualsScaleVectorKernel(GlobalSingleton::getBlocksPerGrid(), GlobalSingleton::getThreadsPerBlock(), data[idx]->getStream(),
		 	           size, data[idx]->getGPUDataPtr(), a, x.data[idx]->getGPUDataPtr());
  }

  #ifdef CUDA_DEBUG
    // check if kernel execution generated an error
    // Check for any CUDA errors
    checkCUDAError("gpuEqualsScaleVectorKernel invocation",1);
  #endif

  for (int idx=0; idx<numDevices; idx++) {
    cudaSetDevice(idx);    
    data[idx]->streamSync();
  }

  // Indicate that the contents of x changed in GPU memory but have
  // not yet been updated in CPU memory
  dirtyGPU = 1;
}


void Vector::equalsXplusY(const Vector &x, const Vector &y) {
  if ( (x.dirtyCPU || y.dirtyCPU) && CPUvsGPUmemWarnings )
    printf("WARNING:  Executing z=x+y GPU operation, but some contents of x or y have changed in CPU memory and have not been copied to the GPU.\n");

  for (int idx=0; idx<numDevices; idx++) {
    cudaSetDevice(idx);    
    launch_EqualsXplusYKernel(GlobalSingleton::getBlocksPerGrid(), GlobalSingleton::getThreadsPerBlock(), data[idx]->getStream(),
			      size, data[idx]->getGPUDataPtr(), x.data[idx]->getGPUDataPtr(), y.data[idx]->getGPUDataPtr());
  }

  #ifdef CUDA_DEBUG
    // check if kernel execution generated an error
    // Check for any CUDA errors
    checkCUDAError("gpuEqualsXplusYKernel invocation",1);
  #endif

  for (int idx=0; idx<numDevices; idx++) {
    cudaSetDevice(idx);    
    data[idx]->streamSync();
  }

  // Indicate that the contents of x changed in GPU memory but have
  // not yet been updated in CPU memory
  dirtyGPU = 1;
}




void Vector::mergeOnGPU() {
  if (numDevices == 1) return;  // We don't need to merge if there's only one device
  
  int rawResolution = (int) ceil((float)size/(float)numDevices);
  int lastRes = size - (numDevices-1)*rawResolution + 1;  
  if (lastRes > rawResolution) lastRes = rawResolution;
  
  copyFromGPU();
  
  int resolution = rawResolution;
  for (int fromDev=0; fromDev<numDevices; fromDev++) {
    if (fromDev == (numDevices-1)) resolution = lastRes;
    
    for (int toDev=0; toDev<numDevices; toDev++)
      if (fromDev != toDev) {
	// printf("\nDBG:  Attempting to copy from host memory for dev %d to dev %d\n",fromDev,toDev);
//        for (int idx=0; idx<data[toDev]->getSize(); idx++) {
//        printf("\nDBG:  Copying %d through %d, from dev %d to dev %d, res=%d, rawRes=%d\n", (fromDev*rawResolution), (fromDev*rawResolution+(resolution-1)), fromDev,toDev,resolution,rawResolution);
        for (int jdx=0; jdx<resolution; jdx++) {
          int idx = fromDev*rawResolution + jdx;
	  REAL_TYPE *fromHostPtr = data[fromDev]->getHostDataPtr();
	  REAL_TYPE *toHostPtr =  data[toDev]->getHostDataPtr();
          // printf("\nDBG:      fromDev=%d, res=%d, jdx=%d\n",fromDev,resolution,jdx);
          // printf("DBG:      fromHostPtr[%d] = %3.3lf\n",idx,fromHostPtr[idx]);
          // printf("DBG:      toHostPtr[%d] = %3.3lf\n",idx,toHostPtr[idx]);
	  toHostPtr[idx] = fromHostPtr[idx];
	}
	
        // // Compute the offset with which to jump into the array 
        // int compNumBytes = resolution*sizeof(REAL_TYPE);//data[fromDev]->getNumBytes();
	// int offset = fromDev * compNumBytes;// * sizeof(REAL_TYPE);

        // // Find the pointers to the interior components
	// REAL_TYPE *toGPUPtr = data[toDev]->getGPUDataPtr() + offset;
	// REAL_TYPE *fromGPUPtr = data[fromDev]->getGPUDataPtr() + offset;
	
        // // printf("DBG:  Attempting to merge offset %d of %d byes from dev %d to device %d\n",offset,compNumBytes,fromDev,toDev);
	
        // cudaSetDevice(toDev);
	// error = cudaMemcpyAsync(toGPUPtr, fromGPUPtr, compNumBytes, cudaMemcpyDeviceToDevice, data[fromDev]->getStream());
	// if (error != cudaSuccess) {
	//    printf("CUDA ERROR: Could not copy from device %d to device %d.  Msg: %s \n", fromDev,toDev,cudaGetErrorString(error));
	//    exit(1);
	// }
      }
  }

  copyToGPU();
}

