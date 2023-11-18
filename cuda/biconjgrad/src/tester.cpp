
#include "defs.h"
#include "gpumeminterface.h"
#include "vector.h"
#include "ellmatrix.h"
#include "globalsingleton.h"

void testVectorOps(int n, int numDevices) {  
  printf("Testing vector operations...\n");
  GlobalSingleton::configureThreadsAndBlocks(n);
  // Vector x(n,numDevices);
  // Vector y(n,numDevices);

  // // fill the arrays 'x' and 'y' on the CPU
  // for (int idx=0; idx<n; idx++) {
  //   x.setValue(idx,(REAL_TYPE)idx);
  //   y.setValue(idx,(REAL_TYPE)idx*2.0);
  // }
  // x.copyToGPU();
  // y.copyToGPU();
    
  // x.print("x = ");
  // y.print("y = ");
  // printf("a = 5.0\n\n");

  // y.sumAXplusThis(x,5.0);
  // y.copyFromGPU();
  // y.print("y = a*x + y = ");

  // REAL_TYPE total = x.dotWith(y);
  // printf("<x,y> = %0.2lf\n",total);

  // // x.print("x = ");
  // x.scale(3.0);
  // x.copyFromGPU();
  // x.print("3*x = ");

  printf("\n\n  -- Testing Matrix Operations --- \n");

  ELLMatrix A(10,10,2,numDevices);
//  ELLMatrix A(3,3,2,1);
  A.setValue(0,0,0.1);
  A.setValue(0,1,0.2);

  A.setValue(1,1,0.3);

  A.setValue(2,0,0.4);
  A.setValue(2,2,0.5);

  A.setValue(3,0,0.6);
  A.setValue(3,3,0.7);

  if (A.getNumRows() > 4) {
    A.setValue(4,7,0.8);

    A.setValue(5,3,0.9);
    A.setValue(5,7,1.0);

    A.setValue(6,1,1.1);
    A.setValue(6,5,1.2);
    
    if (A.getNumRows() > 7) {      
      A.setValue(7,6,1.3);

      A.setValue(8,0,1.4);
    }

    if (A.getNumRows() > 9)
      A.setValue(9,9,1.5);
  }
  
  A.copyToGPU();
    
  A.print(0);
  printf("\n");
  A.printInSparseFormat();
  printf("\n");
  A.printRaw("A");
  printf("\n");

  printf("DBG ------------------------\n");
  
  Vector b(A.getNumRows(),numDevices);
  for (int idx=0; idx<b.getSize(); idx++)
    b.setValue(idx,(REAL_TYPE)(idx+1));
  b.print("b = ");
  printf("\n");
  b.copyToGPU();

  Vector c(A.getNumRows(),numDevices);
  A.multMatAbyVecX(b,c);
  c.copyFromGPU();
  c.print("A*b = ");
}

void testGPUMemInterface(int numValues, int numDevices) {
  GPUMemInterface<int> *interface[numDevices];
  
  printf("Creating memory for each device...\n");
  for (int idx=0; idx<numDevices; idx++) {
    printf(" device %d, ",idx);
    interface[idx] = new GPUMemInterface<int>(numValues,idx);
  }
  printf("\n");
  
  printf("\nSetting host memory values...\n");
  for (int idx=0; idx<numDevices; idx++) {
    interface[idx]->streamSync();  // Wait to make sure the allocation is done
    printf("  device %d : [ ",idx);
    for (int jdx=0; jdx<numValues; jdx++) {
      int val = (int)(jdx * (idx+1));
      printf("%0.0d ",val);
      interface[idx]->setHostValue(jdx,val);
    }
    printf("]\n");
  }
  
  printf("\nCopying from host memory to gpu memory...\n");
  for (int idx=0; idx<numDevices; idx++) {
    printf("  device %d, ",idx);
    interface[idx]->copyToGPU();
  }
  printf("\n");
  
  printf("\nErasing host memory values...\n");
  for (int idx=0; idx<numDevices; idx++) {
    printf("  device %d, ",idx);
    interface[idx]->streamSync(); // Wait to make sure the copy is done
    for (int jdx=0; jdx<numValues; jdx++) 
      interface[idx]->setHostValue(jdx,0);
  }
  printf("\n");
  
  printf("\nCopying back from gpu memory to host memory...\n");
  for (int idx=0; idx<numDevices; idx++) {
    printf("  device %d : [ ",idx);
    interface[idx]->copyFromGPU();
    for (int jdx=0; jdx<numValues; jdx++)
      printf("%0.0d ",interface[idx]->getHostValue(jdx));
    printf("]\n");
  }

  printf("\nDeleting...\n");  
  for (int idx=0; idx<numDevices; idx++) {
    printf("  device %d, ",idx);    
    interface[idx]->streamSync();  // Wait to make sure all operations are done
    delete interface[idx];
  }
  printf("\n");
  
}

void testVectorMem(int size, int numDevices) {
  GlobalSingleton::configureThreadsAndBlocks(size);
  
  Vector x(size,numDevices);

  for (int idx=0; idx<size; idx++)
    x.setValue(idx,1.0);
  x.copyToGPU();

  for (int idx=0; idx<size; idx++)
    x.setValue(idx,-.9);
  x.copyFromGPU();

  x.print("x=");

  // ELLMatrix A(size,size,10,numDevices);

  // for (int idx=0; idx<size; idx++)
  //   for (int jdx=0; jdx<10; jdx++)
  //     A.setValue(idx,jdx,1.0);  
  // A.copyToGPU();

  // for (int idx=0; idx<size; idx++)
  //   for (int jdx=0; jdx<10; jdx++)
  //     A.setValue(idx,jdx,-.9);  

  // A.copyFromGPU();

  // A.printInSparseFormat();

  x.scale(2.0);
  x.copyFromGPU();
  x.print("2x=");
  
//   Vector y(size,numDevices);
// //  A.multMatAbyVecX(x,y);
//   for (int idx=0; idx<size; idx++)
//     y.setValue(idx,1.0);
//   y.sumAXplusThis(x,1.0);
//   y.copyFromGPU();
//   y.print("y=");
}

  
int main( int argc, char *argv[] ) {
//  testGPUMemInterface(8200,1);
//  testVectorOps(30,2);
  testVectorMem(8200,1);
  
  return 1;
}
