#include "defs.h"
#include "vector.h"
#include "ellmatrix.h"
#include "randutils.h"
#include "databuilder.h"

#include<stdio.h>
#include <sys/types.h>
#include <time.h>



__host__ void filereadFinalSolve(const char *matrixFileName, const char *vectorFileName, 
                                 const char *indicesName, const char *outVectFileName, int printLevel=0) {
    ELLMatrix A;
    Vector x,b;
    int nA, nb;
    int *indices;
    clock_t startTime, stopTime;
    double computeTime, assemblyTime;

    printf("  Reading files %s, %s, and %s\n",matrixFileName,vectorFileName,indicesName);
    
    startTime = clock();
    
    // Read & create in the A matrix from a file 
    int numNonZeroVals = readMatrixFromELLFile(matrixFileName,A,printLevel);
    printf("  Matrix read, sparse structure storing %d values and there are %d non-zero values\n",A.size,numNonZeroVals);

    nA = A.numRows;
    blocksPerGrid = nA/threadsPerBlock;

    if (printLevel>1) printELLMatrix(A,(nA>20));
    copyELLMatrixToGPU(A);

    // Read and populate indices
    indices = new int[nA];
    int numIndices = readIndicesFromFile(indicesName,nA,indices,printLevel);

    // Read & create in the b vector from a file      
    nb = nA;
    readVectorFromFile(vectorFileName,nb,x,printLevel,indices);
    if (printLevel>0) printf("  Vector read\n");
    if (printLevel>1) printVector("  x = ",x);
    copyVectorToGPU(x);
    
    if (printLevel>=1)
      printf("---\n");
    
    // Create initial b vector
    createVector(b,nb);
    
    stopTime = clock();  
    assemblyTime = ((double) (stopTime - startTime)) / (double)CLOCKS_PER_SEC;

    printf("  Matrix multiplication, Ax, for %d by %d system ...\n",nA,nA);

    startTime = clock();
    multMatAbyVecX(A,x,b);
    copyVectorFromGPU(b);
    stopTime = clock();  
    computeTime = ((double) (stopTime - startTime)) / (double)CLOCKS_PER_SEC;

    int fileSpecified = (strcmp(outVectFileName,"__NO_FILE__")!=0);    
    if (fileSpecified) {
      printf("  Writing solution to file %s\n",outVectFileName);
      writeVectorToFile(outVectFileName,b,printLevel);
      if (printLevel > 0)
        printVector("  Computed b = ",b);
    }

    else {// ( (printLevel > 0) || (!fileSpecified) )
      printf("  Solution outputfile not specified.\n");
      printVector("  Computed b = ", b);
    }

    printf("  Performance Results:\n");
    printf("    Assembly Time = %lf\n",assemblyTime);
    printf("    Compute Time = %lf\n",computeTime);

    free(indices);
    destroyVector(x);
    destroyVector(b);
    destroyELLMatrix(A);
}
  

void printUsageAndExit(char progname[]) {
  printf("\nUsage: %s <K-matrix-filename> <xprime-vector-filename> <indices-filename> <f-out-vect-filename> [print-level]\n",progname);
  exit(1);
}

int main( int argc, char *argv[] ) {
  int printLevel = 3;
  char matrixFileName[100];  
  char vectorFileName[100];  
  char indicesFileName[100];  
  char outVectFileName[100];
  
  strcpy(matrixFileName,"data.A");
  strcpy(vectorFileName,"data.x");
  strcpy(indicesFileName,"data.idx");
  strcpy(outVectFileName,"__NO_FILE__");

  // Testing arg 1
  if ( (argc > 1) && (strncmp(argv[1],"-help",2) == 0) )
      printUsageAndExit(argv[0]);

  else if (argc <= 3)
    printUsageAndExit(argv[0]);

  else {
    strcpy(matrixFileName,argv[1]);
    strcpy(vectorFileName,argv[2]);
    strcpy(indicesFileName,argv[3]);
    if (argc > 4) strcpy(outVectFileName,argv[4]);
    if (argc > 5) printLevel = atoi(argv[5]);

    filereadFinalSolve(matrixFileName,vectorFileName,indicesFileName,outVectFileName,printLevel);
  }
  
  return 0;
}
