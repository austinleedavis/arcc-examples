#include "randutils.h"
#include<stdio.h>
#include<stdlib.h>
#include<ctime>
#include<math.h>

static int s_wasSeeded = 0;
static const int s_defaultSeed = -1;

void initRand(int seed) {
  if (seed < 0)
    srand((unsigned)time(NULL));    
  else
    srand((unsigned)seed);

  s_wasSeeded = 1;
}


REAL_TYPE genRandomReal(REAL_TYPE min, REAL_TYPE max) {
  if (!s_wasSeeded) initRand(s_defaultSeed);
  
  REAL_TYPE draw = (REAL_TYPE)random() / (REAL_TYPE)(RAND_MAX);
  if (isnan(draw)) draw = 0.0;
  
  return draw*(max-min) + min;
}


int genRandomInt(int min, int max) {
  if (!s_wasSeeded) initRand(s_defaultSeed);

  // RPW:  This may not be distributionally correct, depending on the range size ...
  int range = (max - min) + 1;
  return (random() % range) + min;
}


void testRandGen(int sampleSize) {
  printf("Drawing random integers between 0 and 10:\n");
  printf("c(");
  for (int idx=0; idx<(sampleSize-1); idx++)
    printf("%d,",genRandomInt(0,10));
  printf("%d)\n",genRandomInt(0,10));
  
  printf("Drawing random reals between 0.0 and 10.0:\n");
  printf("c(");
  for (int idx=0; idx<(sampleSize-1); idx++)
    printf("%lf,",genRandomReal(0,10));
  printf("%lf)\n",genRandomReal(0,10));
}


void randomizeVector(Vector &vector,REAL_TYPE min, REAL_TYPE max) {
  for (int idx=0; idx<vector.getSize(); idx++)
    if (min == max) vector.setValue(idx,min);
    else            vector.setValue(idx,genRandomReal(min,max));
  
  vector.dirtyCPU = 1;
}


void randomizeSymmetricPDELLMatrix(ELLMatrix &matrix,REAL_TYPE min, REAL_TYPE max) {
  REAL_TYPE n = (REAL_TYPE) matrix.getNumColsPerRow();
  matrix.dirtyCPU = 1;
  REAL_TYPE val = (max*n)*(max*n) + 1.0;

  if ( (min < 0.0) && (max < 0.0) ) val = n;

  int       colCounts[matrix.getNumRows()];
  
  // for (int idx=0; idx<matrix.getNumRows(); idx++) {
  //   colCounts[idx] = 0;
  // }
  
  // Make sure the values on the diagonal are more than the sum of
  // squares of all values on the row
  for (int row=0; row<matrix.getNumRows(); row++)
    matrix.setValue(row,row,(REAL_TYPE)matrix.getNumColsPerRow()); // May overwrite a value
	  
  // Fill non-diagonal positions with smaller values
  for (int row=0; row<matrix.getNumRows(); row++) {
    REAL_TYPE rowTotal = 0.0;
    // printf("DBG: [%d] :: ",row);
    for (int idx=0; idx < matrix.getNumColsPerRow()-1; idx++) {
      int col = genRandomInt(0,matrix.getNumCols()-1);
      if ( (row!=col) && (matrix.getValue(row,col) == 0.0) && (matrix.getValue(col,row) == 0.0) ) {
	if ( (min < 0.0) && (max < 0.0) )  
	  val = 0.25 * genRandomReal(0.0,1.0) + 0.75;
	else	
          val = genRandomReal(min,max);
        int found = matrix.setValue(col,row,val); // (likewise), A should be symmetric
        if (found) {
          matrix.setValue(row,col,val); // May overwrite a value
          rowTotal += val;
          // colCounts[col]++;
          // printf(" (%d,%d):(%d,%d)=%0.3lf ",row,col,col,row,val);
          idx++;
	}
      }
    }

    // printf("  :: %0.3lf\n",rowTotal);
  }  
}
