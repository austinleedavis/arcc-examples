#ifndef __GPUMEMINTERFACE_H
#define __GPUMEMINTERFACE_H

#include<stdio.h>
#include<stdlib.h>
#include<cuda_runtime_api.h>

#include "defs.h"
#include "globalsingleton.h"

template <class T>
class GPUMemInterface {
public:
  GPUMemInterface(int inSize, int inDeviceIdx, int pinned=0)  {
    GlobalSingleton globals;
    
    size = inSize;
    deviceIdx = inDeviceIdx;
    numBytes = size * sizeof(T);

//    printf("DBG:  Creating GPUMemInterface with sizeof(T)=%d, size=%d, and deviceIdx=%d\n",sizeof(T),size,deviceIdx);

    error = cudaSetDevice(deviceIdx);
    if (error != cudaSuccess) {
      printf("CUDA ERROR: Could not make device %d active.  Msg: %s \n", deviceIdx, cudaGetErrorString(error));
      exit(1);
    }

    globals.getCudaStream(deviceIdx,stream);
    
    error = cudaMalloc((void**)&gpuData, numBytes );
    if (error != cudaSuccess) {
      printf("CUDA ERROR: Could not allocate memory on device %d.  Msg: %s \n", deviceIdx, cudaGetErrorString(error));
      exit(1);
    }
    
    if (pinned)
      cudaHostAlloc((void**)&hostData, numBytes, 
		    /*cudaHostAllocPinned |*/ cudaHostAllocMapped | cudaHostAllocPortable);
    else
      cudaMallocHost((void**)&hostData, numBytes);
  }  

  ~GPUMemInterface()  {
    if (deviceIdx < 0)  return;
    
    cudaSetDevice(deviceIdx);
    streamSync();

    if (gpuData != NULL) {
      cudaFree(gpuData);
      gpuData = NULL;
    }
    
    if (hostData != NULL) {
      cudaFreeHost(hostData);
      hostData = NULL;
    }
    
    if (stream != NULL) {
//      cudaStreamDestroy(stream);
      stream = NULL;
    }

    deviceIdx = -1;
  }
  
  void streamSync()  {
    //Wait for all operations to finish
    error = cudaStreamSynchronize(stream);
    if (error != cudaSuccess) {
      printf("CUDA ERROR: Could not synchronize cuda stream on device %d.  Msg: %s \n", deviceIdx, cudaGetErrorString(error));
      exit(1);
    }
  }

  void copyToGPU()  {
    cudaSetDevice(deviceIdx);
    error = cudaMemcpy(gpuData, hostData, numBytes, cudaMemcpyHostToDevice);
    // RPW-NOTE:  This should be done asyncrhonously, but for some reason that isn't working for me right now
    //  error = cudaMemcpyAsync(gpuData, hostData, numBytes, cudaMemcpyHostToDevice, stream);
    if (error != cudaSuccess) {
      printf("CUDA ERROR: Could copy host data to device %d.  Msg: %s \n", deviceIdx,cudaGetErrorString(error));
      exit(1);
    }
  }

  void copyFromGPU()  {
    cudaSetDevice(deviceIdx);
    error = cudaMemcpy(hostData, gpuData, numBytes, cudaMemcpyDeviceToHost);
    // RPW-NOTE:  This should be done asyncrhonously, but for some reason that isn't working for me right now
    //  error = cudaMemcpyAsync(hostData, gpuData, numBytes, cudaMemcpyDeviceToHost, stream);
    if (error != cudaSuccess) {
      printf("CUDA ERROR: Could copy device %d data to host.  Msg: %s \n", deviceIdx, cudaGetErrorString(error));
      exit(1);
    }
  }

  // Accessors
  int getNumBytes() {return numBytes;}
  int getSize() {return size;}  
  T* getGPUDataPtr() {return gpuData;}  // needed for device kernel calls
  T* getHostDataPtr() {return hostData;}  // needed for device kernel calls
  cudaStream_t getStream() {return stream;}     // needed for device kernel calls

  T getHostValue(int idx) {
    if ( (idx < 0) || (idx >= size) ) return (T)0;   // should be nan!!!
    else                              return hostData[idx];
  }

  void setHostValue(int idx, T value) {
    if ( (idx >= 0) && (idx < size) ) hostData[idx] = value;
  }

    
protected:
  cudaError_t error;
  int size;
  int deviceIdx;
  int numBytes;
  T *hostData;
  T *gpuData;
  cudaStream_t stream;
};




#endif
