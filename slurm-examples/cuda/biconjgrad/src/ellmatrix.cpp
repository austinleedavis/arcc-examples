#include "ellmatrix.h"
#include "kernelFunctions.h"
#include "globalsingleton.h"

#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include<math.h>
  

/**
 * Create a matrix data structure in sparse, ELL format.  Allocate any necessary memory,
 * including the on-board GPU memory used in GPU routines.
 **/
ELLMatrix::ELLMatrix(int inNumRows, int inNumCols, int inNumColsPerRow, int inNumDevices) {
  int resolution;
  numRows = inNumRows;
  numCols = inNumCols;
  numColsPerRow = inNumColsPerRow;
  size = numRows * numColsPerRow;

  if (numRows < inNumDevices) {
    numDevices = 1;
    resolution = size;
  }
  
  else {
    numDevices = inNumDevices;
    resolution = (int) ceil((float)numRows / (float)numDevices);
  }
  
  // printf("DBG:  Creating matrix:\n");
  // printf("       size = %d\n",size);
  // printf("       rows = %d\n", numRows);
  // printf("       cols = %d\n", numCols);
  // printf("       cpr  = %d\n", numColsPerRow);
  // printf("       dev  = %d\n", numDevices);
  // printf("       res  = %d\n", resolution);

  dirtyCPU = 1;
  dirtyGPU = 0;

  for (int dev=0; dev<numDevices; dev++) {
    // Divide rows among devices ... but the last device may be a bit short
    int remaining = numRows*numColsPerRow - (dev * resolution);
    int numRowsForThisDevice = (remaining < resolution ? remaining : resolution);

    int compSize = numRowsForThisDevice*numColsPerRow;

    data[dev] = new GPUMemInterface<REAL_TYPE>(compSize, dev);
    indices[dev] = new GPUMemInterface<int>(compSize, dev);
    
    for (int jdx=0; jdx<compSize; jdx++) {
      indices[dev]->setHostValue(jdx,INVALID_COLUMN);
      data[dev]->setHostValue(jdx,0.0);
    }
  }
}



/**
 * Destroy a sparse, ELL formatted matrix datastructure, freeing all
 * preallocated memory and assigning all values to 0 or NULL.
 **/
ELLMatrix::~ELLMatrix() {
  for (int idx=0; idx<numDevices; idx++) {
    delete indices[idx];
    delete data[idx];
  }
  
  size = 0;
  numRows = 0;
  numCols = 0;
  numColsPerRow = 0;
  dirtyCPU = 1;
  dirtyGPU = 1;
}


/**
 * Copy indices and data element value data from the CPU memory to
 * the GPU memory storage areas.
 **/
void ELLMatrix::copyToGPU() {
  for (int idx=0; idx<numDevices; idx++) {
    indices[idx]->copyToGPU();
    data[idx]->copyToGPU();
  }

  dirtyCPU = 0;
  dirtyGPU = 0;
}


/**
 * Copy indices and data element value data from the GPU memory to
 * the CPU memory storage areas.
 **/
void ELLMatrix::copyFromGPU() {
  for (int idx=0; idx<numDevices; idx++) {
    indices[idx]->copyFromGPU();
    data[idx]->copyFromGPU();
  }

  dirtyCPU = 0;
  dirtyGPU = 0;
}


/**
 * Return the value of the element at the M[row,col] of the mathematical matrix.
 * Return 0.0 if the element does not exist in the sparse formatted matrix.  This
 * reads only values stored in CPU memory
 **/
REAL_TYPE ELLMatrix::getValue(int row, int col) const {
  REAL_TYPE value = 0.0;

  if ( (dirtyGPU) && CPUvsGPUmemWarnings )
    printf("WARNING:  Value from ELLMatrix CPU memory requested, but there have been GPU operations on the matrix since the last copy from GPU\n");
  
  int resolution = (int) ceil((float)numRows / (float)numDevices);
  int whichDevice = (int) floor((float)row / (float)resolution);
  int componentRow = row % resolution;
  
  if (whichDevice > (numDevices-1)) {
    componentRow = row;
    whichDevice = (numDevices -1);
  }

  for (int idx=0; idx<numColsPerRow; idx++) {
    int pos = componentRow * numColsPerRow + idx;

    if (indices[whichDevice]->getHostValue(pos) == col) {
      value = data[whichDevice]->getHostValue(pos);
      // printf("DBG: %d * %d + %d = %d, data[%d] = %0.1lf\n",
      //  	     row,matrix.numColsPerRow,idx,pos,pos,value);
      break;
    }
  }

  return value;
}


/**
 * Set the value of the element at the M[row,col] of the mathematical matrix.
 * The function returns an indicator as to whether this could be done or not.
 * In cases where there are already numColsPerRow number of elements in the
 * specified row, a new value cannot be added.  This affects only values
 * stored in CPU memory.
 **/
int ELLMatrix::setValue(int row, int col, REAL_TYPE value) {
  int found = 0;

  if ( (row < 0) || (row >= numRows) ) {
    printf("WARNING: Ignoring attempt to set value in row %d of a matrix with only %d rows (indices are zero-ref'd)\n",
	   row,numRows);
    return found;
  }
  
  if ( (col < 0) || (col >= numCols) ) {
    printf("WARNING: Ignoring attempt to set value in col %d of a matrix with only %d cols (indices are zero-ref'd)\n",
	   col,numCols);
    return found;
  }
  
  int resolution = (int) ceil((float)numRows / (float)numDevices);
  int whichDevice = (int) floor((float)row / (float)resolution);
  int componentRow = row % resolution;

  if (whichDevice > (numDevices-1)) {
    componentRow = row;
    whichDevice = (numDevices -1);
  }
    
  // Search for a place to put the value in the appropriate row
  for (int idx=0; idx<numColsPerRow; idx++) {
    int pos = componentRow * numColsPerRow + idx;

    // If the specified column index already exists,
    // overwrite the value for that (row,col) cell
    if (indices[whichDevice]->getHostValue(pos) == col) {
      data[whichDevice]->setHostValue(pos,value);
      found = 1;
      break;
    }

    // If there is an unused space available, use that one
    else if (indices[whichDevice]->getHostValue(pos)  == INVALID_COLUMN) {
      indices[whichDevice]->setHostValue(pos,col);
      data[whichDevice]->setHostValue(pos,value);
      found = 1;
      break;
    }
  }
  
  if (found)  dirtyCPU = 1;

  return found;
}

void ELLMatrix::rawSetValue(int rawIdx, int col, REAL_TYPE value) {
  int resolution = numColsPerRow * (int) ceil((float)numRows / (float)numDevices);
  int whichDevice = (int) floor((float)rawIdx / (float)resolution);
  int pos = rawIdx % resolution;

  if (whichDevice > (numDevices-1)) {
    pos = rawIdx;
    whichDevice = (numDevices -1);
  }

  // printf("DBG:   resolution=%d\n",resolution);
  // printf("DBG:   whichDevice=%d\n",whichDevice);
  // printf("DBG:   pos=%d\n",pos);
  // printf("DBG:   val=%0.3lf\n",value);
  // printf("DBG:   col=%d\n\n",col);
  
  indices[whichDevice]->setHostValue(pos,col);
  data[whichDevice]->setHostValue(pos,value);
}



/**
 * Pretty-print the mathematical matrix.
 **/
void ELLMatrix::print(int reduceZeros) const {
  if ( (dirtyGPU) && CPUvsGPUmemWarnings )
    printf("WARNING:  Printing ELLMatrix from CPU memory, but there have been GPU operations on the matrix since the last copy from GPU\n");

  for (int row=0; row<numRows; row++) { 
    printf(" [  ");
    for (int col=0; col<numCols; col++) {
      REAL_TYPE value = getValue(row,col); 
      if (reduceZeros && (value == 0.0)) printf("0 ");
      else printf("%0.4lf ",value);
    }
    printf(" ]\n");
  }
}


/**
 * Pretty-print the two 2D structures (values and column indices)
 * stored by the ELL structure
 **/
void ELLMatrix::printInSparseFormat() const {
  if ( (dirtyGPU) && CPUvsGPUmemWarnings )
    printf("WARNING:  Printing ELLMatrix from CPU memory, but there have been GPU operations on the matrix since the last copy from GPU\n");

  int resolution = (int) ceil((float)numRows / (float)numDevices);
  
  for (int row=0; row<numRows; row++) { 
    int whichDevice = (int) floor((float)row / (float)resolution);
    int componentRow = row % resolution;

    if (whichDevice > (numDevices-1)) {
      componentRow = row;
      whichDevice = (numDevices -1);
    }

    printf(" [  ");
    for (int idx=0; idx<numColsPerRow; idx++) {
      int pos = componentRow * numColsPerRow + idx;
      printf("%0.2lf ", data[whichDevice]->getHostValue(pos));
    }

    printf(" ]\t[  ");
    for (int idx=0; idx<numColsPerRow; idx++) {
      int pos = componentRow * numColsPerRow + idx;
      if (indices[whichDevice]->getHostValue(pos) == INVALID_COLUMN)
	printf(". ");
      else
	printf("%d ", indices[whichDevice]->getHostValue(pos) );
    }
    printf(" ]\n");
  }
}

/**
 * Pretty-print the two 2D structures (values and column indices)
 * stored by the ELL structure
 **/
void ELLMatrix::printRaw(const char *matrixName) const {
  if ( (dirtyGPU) && CPUvsGPUmemWarnings )
    printf("WARNING:  Printing ELLMatrix from CPU memory, but there have been GPU operations on the matrix since the last copy from GPU\n");

  printf("----------------------------\n");
  printf("  Raw matrix data for %s \n",matrixName);
  printf("----------------------------\n"); 
  printf("  numDevices=%d\n",numDevices);
  printf("  size=%d\n",size);
  printf("  numRows=%d\n",numRows);
  printf("  numCols=%d\n",numCols);
  printf("  numColsPerRow=%d\n",numColsPerRow);

  int jdx = 0;
  printf("\nDev\tIdx\tCol\tValue\n");
  printf("----------------------------\n");  
  for (int dev=0; dev<numDevices; dev++){
    int compSize = indices[dev]->getSize();
    for (int idx=0; idx<compSize; idx++) 
      printf("%d\t%d\t%d\t%0.2lf\n",dev,jdx++,indices[dev]->getHostValue(idx),data[dev]->getHostValue(idx));
  }
  printf("----------------------------\n");  
}


// void stubbedCPUMult(const int numRows, 
// 			     const int numCols, 
// 			     const int numColsPerRow,
// 			     const int *indices, 
// 			     const REAL_TYPE *data,
// 			     const REAL_TYPE *x, 
// 			     REAL_TYPE *y) {
//   for (int row=0; row<numCols; row++) {
//     y[row] = 0.0;

//     for (int idx=0; idx<numColsPerRow; idx++) {
//       int pos = row*numColsPerRow + idx;
//       int col = indices[pos];
//       REAL_TYPE value = data[pos];

//       if ( (col != INVALID_COLUMN) )
//         y[row] += value * x[col];

//       printf("DBG: row=%d, idx=%d, pos=%d, col=%d, value=%lf, x[col]=%lf\n",
// 	     row,idx,pos,col,value,x[col]);
//     }
//   }
// }


/**
 * This is a wrapper CPU-based function call around the GPU-based
 * matrix operations Ax=y, where A is a matrix, and x and y are
 * vectors.  This routine uses only the on-board GPU memory and
 * expects that data from the CPU storage has already been copied.
 * It DOES NOT copy the GPU-stored results structure back to the
 * CPU memory.
 **/
void ELLMatrix::multMatAbyVecX(Vector &x, Vector& y,int useCPUMultiplier) {
  if ( (dirtyCPU) && CPUvsGPUmemWarnings )
    printf("WARNING:  Multiplying ELLMatrix by Vector, but contents of matrix in CPU memory have changed since last GPU copy\n");

  if ( (x.dirtyCPU) && CPUvsGPUmemWarnings )
    printf("WARNING:  Multiplying ELLMatrix by Vector, but contents of vector in CPU memory have changed since last GPU copy\n");
  
  if (useCPUMultiplier) {
    // stubbedCPUMult(matrix.numRows,
    // 		   matrix.numCols,
    // 		   matrix.numColsPerRow,
    // 		   matrix.indices,
    // 		   matrix.data,
    // 		   x.data,
    // 		   y.data);
    // copyVectorToGPU(y);  // Copy it back into the GPU as though it had been done
  }
  else {
    // Execute the GPU Kernel
    int resolution = (int) ceil((float)numRows / (float)numDevices);
    int numComp = resolution;

    for (int dev=0; dev<numDevices; dev++) {
      cudaSetDevice(dev);

      int vectorOffsetPos = dev*resolution;// * numColsPerRow);
      if (dev==(numDevices-1))  numComp = numRows - dev*resolution +1;
      if (numComp > resolution) numComp = resolution;
      
      // printf("DBG:  Launching Matrix mult kernel:\n");
      // printf("       numRows=%d\n",numRows);
      // printf("       numCols=%d\n",numCols);
      // printf("       numCPR=%d\n", numColsPerRow);
      // printf("       dev=%d\n",dev);
      // printf("       res=%d\n",resolution);
      // printf("       vos=%d\n",vectorOffsetPos);
      // printf("       numComp=%d\n",numComp);
      // printf("       bpg=%d\n",GlobalSingleton::getBlocksPerGrid());
      // printf("       tpb=%d\n",GlobalSingleton::getThreadsPerBlock());
      
      launch_MultMatAbyVecXKernel(GlobalSingleton::getBlocksPerGrid(),GlobalSingleton::getThreadsPerBlock(),indices[dev]->getStream(),
				  numRows,
				  numCols,
				  numColsPerRow,
				  dev,vectorOffsetPos,numComp,
				  indices[dev]->getGPUDataPtr(),
				  data[dev]->getGPUDataPtr(),
				  x.getGPUDataPtr(dev),
				  y.getGPUDataPtr(dev));
    }
 
    #ifdef CUDA_DEBUG
      // check if kernel execution generated an error
      // Check for any CUDA errors
      checkCUDAError("gpuMulAbyVecXKernel invocation",1);
    #endif

    for (int dev=0; dev<numDevices; dev++) {
      cudaSetDevice(dev);    	
      data[dev]->streamSync();	
    }
    
    y.mergeOnGPU();

    for (int dev=0; dev<numDevices; dev++) {
      cudaSetDevice(dev);    	
      data[dev]->streamSync();	
    }
  }

  // Indicate that the contents of y changed in GPU memory but have
  // not yet been updated in CPU memory
  y.dirtyGPU = 1;
}




