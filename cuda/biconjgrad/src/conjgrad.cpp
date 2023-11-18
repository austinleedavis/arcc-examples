#include "defs.h"
#include "vector.h"
#include "ellmatrix.h"
#include "randutils.h"
#include "databuilder.h"
#include "gpumeminterface.h"
#include "globalsingleton.h"

#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<time.h>
#include<string.h>
#include<math.h>

#define RESTART_IF_DIVERGING false

REAL_TYPE conjugateGradientSolve(ELLMatrix &A,
				 Vector &x, // In: init x, Out: Soln.					  
				 const Vector &b,
				 double &solveTime,
				 int &iterationCount,
				 const REAL_TYPE errorThreshold,
				 const int printLevel) {
  clock_t startTime, stopTime;
  startTime = clock();
  
  // Vector r(x.getSize(),x.getNumDevices());
  // Vector p(x.getSize(),x.getNumDevices());

  REAL_TYPE alpha, rsold, rsnew, rsUpperBound, rsLowerBound;
  int done = 0;
  int divergenceCount = 0;

  iterationCount = 0;
  
  A.copyToGPU();
  x.copyToGPU();
  
  // Ap = A * x (using Ap to remove unncessary mem copy to GPU)
  Vector Ap(x.getSize(),x.getNumDevices());
  A.multMatAbyVecX(x,Ap);
  Ap.dirtyCPU = 0;  // Ignore AP for CPU memory purposes

  double pdCheck = x.dotWith(Ap);

  // b - A*x -> r
  Vector r(b);
  r.copyToGPU();
  r.sumAXplusThis(Ap,-1.0);
  
  // We have to get r back from the GPU so we can copy it into p
  // RPW.29.Jun: There's a better way:  Write a GPU routine to copy r directly into p ...
  //             I should implement this anyway, but here it's not *too* bad because it's
  //             only done once.


  if (printLevel > 1) {
    r.copyFromGPU();
    r.print("r =");
    Ap.copyFromGPU();
    Ap.print("Ap =");
  }
  
  // p = r
  Vector p(r.getSize(),r.getNumDevices());
  p.dirtyCPU = 0;         // Ignore warnings, we don't care about p in CPU memory at all
  p.copyGPUContents(r);   // Copy directly on the GPU

  // Compute the residual from the "previous step". 
  rsold = r.dotWith(r);
  rsUpperBound = 0.0;
  rsLowerBound = 0.0;

  if (pdCheck < 0.0) {
    printf("Matrix is *not* positive definite!  x'Ax=%0.5lf\n",pdCheck);
    return rsold;
  }
  
  // DBG:  Special debug bail-out to check the state of things before the algorithm
  //       starts to iterate.  Leave commented out unless necessary ...
//  return rsold;
  
  while ( (iterationCount < A.getNumRows()) && (!done) ) {
    iterationCount++;
    
    // p.copyFromGPU();
    // p.print("p =");

   // A*p -> Ap
    A.multMatAbyVecX(p,Ap); 
    // Ap.copyFromGPU();
    // Ap.print("*********Ap=");
    
    // Compute the scalar ratio alpha based on residual
    //alpha = (rsold / p'A*p)
//    printf("DBG:  p'A*p=%lf\n",p.dotWith(Ap));
    alpha = rsold / p.dotWith(Ap);
    
    // Update x and r
    // x + alpha*p   -> x
    // r - alpha*Ap  -> r
    x.sumAXplusThis(p,alpha);
    r.sumAXplusThis(Ap,-1.0*alpha);

    if (printLevel > 1) 
      printf("alpha=%0.4lf\n",alpha);
    
    if (printLevel > 1) {
      x.copyFromGPU();
      x.print("x = ");
    }

    // Compute the new residual scalar
    rsnew = r.dotWith(r);
    if (printLevel > 0)
      printf("%d:Residual=%lf\n",iterationCount,rsnew);
    
    // If we meet our error threshold, we're done
    // Done if [sqrt(rsnew) < err] <=> [rsnew < err^2]
    if (rsnew < 2000) //(errorThreshold*errorThreshold)) 
      done = 1;

    // Otherwise, we must update p based on the scaled residual
    // change ratio and setup the residual scalar for the next step
    else {
      // r + (rsnew / rsold) * p  -> p
      // if (rsnew < rsold)
        p.scale(rsnew/rsold);
      p.sumAXplusThis(r,1.0);

//      printf("DBG:  rsnew=%lf, rsold=%lf, alpha=%lf\n",rsnew,rsold,alpha);
      // if something went horribly wrong, bail out
      if (isnan(rsnew))  
        done = 1;

      // Otherwise, if this step diverges ...
      else if (rsnew > rsold) {
        // If this is the first step diverging since last reset, set the bounds
        if (divergenceCount == 0) {
      	  rsLowerBound = rsnew / 2.0;
      	  rsUpperBound = 2.0 * rsnew;
      	}
	
        // Count the number of steps between resets  that diverge
      	divergenceCount++;  
      }

      // Otherwise, if the step converges and has made it past the lower bound,
      // then reset the diverging step counter
      else if (rsnew < rsLowerBound)
      	  divergenceCount = 0;

      rsold = rsnew;
    }

    // If we've taken more than 10 diverging steps AND exceeded to arbitrary
    // upper bound we set for ourselves, the bail out (possibly restarting)
    if ( (rsnew > rsUpperBound) && (divergenceCount > 10) && (RESTART_IF_DIVERGING) ) {
      printf("Warning:  conjugate gradient search seems to be diverging\n");
      return -1.0;
    }

  } // end of while
  
  // Make sure the solution vector is available
  x.copyFromGPU();
  
  stopTime = clock();  
  solveTime = ((double) (stopTime - startTime)) / (double)CLOCKS_PER_SEC;

  return rsnew;
}

void verifyGPUDevice(int maxNumDevices) {
  cudaDeviceProp  prop;
  int             deviceCount;
  int             driverVersion;
  int             runtimeVersion;
  cudaError_t error;

  error = cudaRuntimeGetVersion(&runtimeVersion);
  if (error != cudaSuccess) {
    printf("CUDA ERROR: Could not get runtime version.  Msg: %s \n", cudaGetErrorString(error));
    exit(1);
  }
  printf("  CUDA Runtime Version: %d\n",runtimeVersion);

  error = cudaDriverGetVersion(&driverVersion);
  if (error != cudaSuccess) {
    printf("CUDA ERROR: Could not get driver version.  Msg: %s \n", cudaGetErrorString(error));
    exit(1);
  }
  printf("  CUDA Driver Version: %d\n",driverVersion);

  error = cudaGetDeviceCount(&deviceCount);
  if (error != cudaSuccess) {
    printf("CUDA ERROR: Could not get device count.  Msg: %s \n", cudaGetErrorString(error));
    exit(1);
  }

  printf("  There are %d CUDA-accessible devices, out of a max of %d:\n",deviceCount, maxNumDevices);

  if (deviceCount < maxNumDevices)
    setDBNumberOfDevices(deviceCount);
  else
    setDBNumberOfDevices(maxNumDevices);
  
  double scale1 = 1024.0 * 1024.0;
  double scale2 = 1024.0 * 1024.0 * 1024.0;
  for (int idx=0; idx< deviceCount; idx++) {
     cudaGetDeviceProperties(&prop,idx);
     printf("    Name:               %s\n",prop.name);
     printf("    Compute capability: %d.%d\n",prop.major,prop.minor);
     double clockRate = (double)(prop.clockRate)/scale1;
     printf("    Clock rate:         %0.3lf GHz\n", clockRate);
     double totalMem = (double)(prop.totalGlobalMem)/scale2;
     printf("    Total global mem:   %0.3lf Gb\n", totalMem );
     printf("\n");
   }
}

void testConjGrad(int n, int numValuesPerRow, int printLevel=0) {
    GlobalSingleton::configureThreadsAndBlocks(n);
    clock_t startTime, stopTime;
    double assemblyTime,solveTime;
    int iterationCount = 0;

    printf("  Using GPU with %d threads per block and %d blocks per grid\n",GlobalSingleton::getThreadsPerBlock(),GlobalSingleton::getBlocksPerGrid());
    printf("  Sharing matrix across %d devices\n", getDBNumberOfDevices());
    startTime = clock();

    printf("  Creating A,x, and b datastructures...\n");

    ELLMatrix A(n,n,numValuesPerRow,getDBNumberOfDevices());
    Vector x(n,getDBNumberOfDevices(),1.0);
    Vector b(n,getDBNumberOfDevices());

    printf("  Generating a random %d by %d matrix, where no row has more than %d non-zero values...\n",n,n,numValuesPerRow);    

    // Create a random, symmetric PD matrix, A    
    randomizeSymmetricPDELLMatrix(A,-1.0,-1.0);
    if ( (printLevel>=3) && (n < 30) )
      A.print(0);
    else if ( (printLevel>=3) && (n < 100) )
      A.print(1);
    else if (printLevel>=3)
      A.printInSparseFormat();
    A.copyToGPU();

    printf("Determining the arbitrary b vector with %d values...\n",n);
    
    // Create a vector of all 1's
    if (printLevel>=2) x.print("x = ");
    x.copyToGPU();
    
    // Generate b via matrix multiplication
    b.copyToGPU();  // To push the 0's ...
    A.multMatAbyVecX(x,b,0);  // Ax=b
    if (printLevel>=2) {
      b.copyFromGPU();
      b.print("Ax=b = ");
      printf("---\n");
    }
    
    printf("  Generating random initial x vector with %d values...\n",n);
    
    // Reset x to some random initial values
    randomizeVector(x,-2.0,2.0);
    if (printLevel>=2) x.print("Init x0 = ");

    stopTime = clock();  
    assemblyTime = ((double) (stopTime - startTime)) / (double)CLOCKS_PER_SEC;

    printf("  Solving %d by %d system ...\n",n,n);    
    REAL_TYPE resid = -1.0;
    while (resid < 0.0) {
      resid = conjugateGradientSolve(A,x,b,solveTime,iterationCount,CG_ERROR_TOLERANCE,printLevel);
      if (resid < 0.0) {
	printf("Restarting search ...\n");
        randomizeVector(x,0.0,0.1);
        if (printLevel>2) x.print("Init x0 = ");
      }
    }
    
    if (printLevel>0) x.print("Solved x = ");

    printf("  Performace Results:\n");
    printf("    Assembly Time = %lf\n",assemblyTime);
    printf("    Solve Time = %lf\n",solveTime);
    printf("    Residual = %0.5lf\n",resid);
    printf("    Performed %d iterations\n",iterationCount);

}


void filereadConjGrad(const char *matrixFileName, 
		      const char *vectorFileName, 
		      const char *outVectFileName,
		      int printLevel=0) {
    int nA, nb;
    clock_t startTime, stopTime;
    double assemblyTime,solveTime;
    int iterationCount = 0;

    printf("  Sharing matrix across %d devices\n", getDBNumberOfDevices());
    printf("  Reading files %s and %s\n",matrixFileName,vectorFileName);
    
    startTime = clock();

    int numRead;
    ELLMatrix *A = readMatrixFromELLFile(matrixFileName,numRead,printLevel);
    nA = A->getNumRows();
    double trueSize = (double)nA * (double)nA;

    printf("  Matrix read, (%d x %d) sparse structure storing %d values and there are %d non-zero values\n",nA,nA,A->getSize(),numRead);
    printf("  True density=%0.5lf, Effective density=%0.5lf\n", ((double)numRead/trueSize),(double)A->getSize()/trueSize);

    GlobalSingleton::configureThreadsAndBlocks(nA);
    printf("  Using GPU with %d threads per block and %d blocks per grid\n",GlobalSingleton::getThreadsPerBlock(),GlobalSingleton::getBlocksPerGrid());

    if ( (printLevel>=3) && (nA < 30) )
      A->print(0);
    else if ( (printLevel>=3) && (nA < 100) )
      A->print(1);
    else if (printLevel>=3)
      A->printInSparseFormat();

    A->copyToGPU();
        
    // Read & create in the b vector from a file      
    nb = nA;
    Vector *b = readVectorFromFile(vectorFileName,nb,numRead,printLevel);
    if (printLevel>0) printf("  Vector read\n");
    if (printLevel>1) b->print("  b = ");
    
    b->copyToGPU();
    
    if (printLevel>=1)
      printf("---\n");
    
    // Create x with some random initial values
    double rndBound = 0.1;
    Vector *x = new Vector(nb,getDBNumberOfDevices());
    randomizeVector(*x,-rndBound,rndBound);
    if (printLevel>1) x->print("  Init x0 = ");
    
    stopTime = clock();  
    assemblyTime = ((double) (stopTime - startTime)) / (double)CLOCKS_PER_SEC;

    printf("  Solving %d by %d system ...\n",nA,nA);
    REAL_TYPE resid = -1.0;
    while (resid < 0.0) {
      resid = conjugateGradientSolve(*A,*x,*b,solveTime,iterationCount,CG_ERROR_TOLERANCE,printLevel);
      if (resid < 0.0) {
	printf("Restarting search ...\n");
        // rndBound *= 2.0;
        randomizeVector(*x,-rndBound,rndBound);
        if (printLevel>1) x->print("Init x0 = ");
      }
    }

    int fileSpecified = (strcmp(outVectFileName,"__NO_FILE__")!=0);    
    if (fileSpecified) {
      printf("  Writing solution to file %s\n",outVectFileName);
      writeVectorToFile(outVectFileName,*x,printLevel);
      if (printLevel > 0) x->print("  Solved x = ");
    }

    else {// ( (printLevel > 0) || (!fileSpecified) )
      printf("  Solution outputfile not specified.\n");
      x->print("  Solved x = ");
    }
    
    printf("  Performance Results:\n");
    printf("    Assembly Time = %lf\n",assemblyTime);
    printf("    Solve Time = %lf\n",solveTime);
    printf("    Residual = %0.5lf\n",resid);
    printf("    Performed %d iterations\n",iterationCount);
    
    delete x;
    delete b;
    delete A;
}
  
  

void printUsageAndExit(char progname[]) {
  printf("\nUsage:  %s can be used in one of two ways:\n",progname);
  printf("  1) To solve data provided by input files, or\n");
  printf("      %s <K-matrix-filename> <f-vector-filename> <x-out-vect-filename> [print-level] [num-gpu-devices]\n",progname);
  printf("  2) To solve random test cases.\n");
  printf("      %s <num-equations> <num-cols-per-row> [print-level] [num-gpu-devices]\n\n",progname);
  exit(1);
}

int main( int argc, char *argv[] ) {
  int n = 10;
  int cpr = 4;
  int printLevel = 3;
  int isArg1Integer = 1;
  int maxNumDevices=4;	
  char matrixFileName[100];  
  char vectorFileName[100];
  char outVectFileName[100];
  strcpy(matrixFileName,"data.A");
  strcpy(vectorFileName,"data.b");
  strcpy(outVectFileName,"__NO_FILE__");
  

  // Testing arg 1
  if (argc > 1) {
    if ( (strncmp(argv[1],"-help",2) == 0) ||
         (strncmp(argv[1],"-HELP",2) == 0) )
      printUsageAndExit(argv[0]);
 
    else {
      int len = strlen(argv[1]);
      for (int idx=0; idx<len; idx++) 
        if ( (argv[1][idx] < '0') || (argv[1][idx] > '9') )
          isArg1Integer = 0;
    }
  }

  else if (argc <= 2)
    printUsageAndExit(argv[0]);


  if (isArg1Integer) {
    printf("  Since arg 1 is a number, the program assumes you are running random test cases\n\n");

    n = atoi(argv[1]);  
    cpr = atoi(argv[2]);
    if (argc > 3) printLevel = atoi(argv[3]);
    if (argc > 4) maxNumDevices = atoi(argv[4]);

    verifyGPUDevice(maxNumDevices);
    testConjGrad(n,cpr,printLevel);
  }

  else {
    printf("  Since arg 1 is not a number, the program assumes you are loading data from files\n\n");

    strcpy(matrixFileName,argv[1]);
    strcpy(vectorFileName,argv[2]);

    if (argc > 3) strcpy(outVectFileName,argv[3]);
    if (argc > 4) printLevel = atoi(argv[4]);
    if (argc > 5) maxNumDevices = atoi(argv[5]);

    verifyGPUDevice(maxNumDevices);
    filereadConjGrad(matrixFileName,vectorFileName,outVectFileName,printLevel);
  }

//  testVectorOps(8);
//  testRandGen(500);
  
  return 0;
}
