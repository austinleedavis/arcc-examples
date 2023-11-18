#include "globalsingleton.h"

static bool isInitialized = false;
static cudaStream_t sStreamTable[10];
static int threadsPerBlock=1024;   // Number of threads per block
static int blocksPerGrid = 8; // Number of blocks in a grid

void GlobalSingleton::initialize() {
  if (!isInitialized) {
    for (int idx=0; idx<10; idx++)
      sStreamTable[idx] = NULL;
    isInitialized=true;
  }
}

void GlobalSingleton::getCudaStream(int deviceIdx, cudaStream_t &stream) {
  if (sStreamTable[deviceIdx] == NULL) {
    cudaError_t error = cudaStreamCreate(&stream);
    sStreamTable[deviceIdx] = stream;
    if (error != cudaSuccess) {
      printf("CUDA ERROR: Could not create cuda stream on device %d.  Msg: %s \n", deviceIdx, cudaGetErrorString(error));
      exit(1);
    }
  }

  else  
    stream = sStreamTable[deviceIdx];
}



void GlobalSingleton::configureThreadsAndBlocks(int bpg, int tpg) {
  if ( (tpg > 0) && (tpg <= 1024) )
    threadsPerBlock = tpg;
    
  if (bpg > 0) {
    int blockFraction = (bpg/threadsPerBlock) + 1;
    blocksPerGrid = ( blockFraction > 4 ? blockFraction : 4);
  }
  
}


int GlobalSingleton::getBlocksPerGrid() {
  return blocksPerGrid;
}

int GlobalSingleton::getThreadsPerBlock() {
  return threadsPerBlock;
}

