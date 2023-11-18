#include "databuilder.h"
#include<stdio.h>
#include<stdlib.h>
#include<math.h>

int gNumDevices = 1;

void setDBNumberOfDevices(int numDevices) {
  if (numDevices > 0) gNumDevices = numDevices;
}

int getDBNumberOfDevices() {
  return gNumDevices;
}



int countValuesPerLine(const char *fileName, int printLevel) {
  FILE *file = fopen(fileName,"r");
  int numValsOnLine = -1;

  if (file) {
    int status = 0;
    long int eolPosition = -1L;

    if (printLevel > 2)
      printf("Counting values per line in %s\n",fileName);
    
    while ( (status != EOF) && (eolPosition < 0) ) {
      char c = fgetc(file);
      if (c == EOF) status = EOF;
      else if (c == '\n')
	eolPosition = ftell(file);
    }

    if (printLevel > 2)
      printf("  Determined EOL position.  Counting values to EOL:\n  ",fileName);

    rewind(file);

    status = 0;
    long int currPosition = -1L;
    while ( (currPosition <= eolPosition) && (status != EOF) ) {
      numValsOnLine++;

      double val;
      status = fscanf(file,"%lf",&val);

      if (status <= 0) {
	printf("READ ERROR:  File %s may not be a valid format.\n",fileName);
	exit(1);
      }
      
      currPosition = ftell(file);
      if ( (printLevel > 2) && ((numValsOnLine % 100 == 0)) ) printf(".");
    }

    if (printLevel > 2) printf("\n  Counted %d values\n\n",numValsOnLine);
    
    fclose(file);    
  }
    
  return numValsOnLine;
}


ELLMatrix *readMatrixFromFile(const char *fileName, int systemSize, int &numValsRead, int printLevel) {
  FILE *file = fopen(fileName,"r");
  int numColsPerRow[systemSize];
  int idx;
  int progress;
  int totalNumNonZeroVals = 0;
  int maxCols;
  ELLMatrix *matrix = NULL;
  
  numValsRead = -1;
  
  for (idx=0; idx<systemSize; idx++)
    numColsPerRow[idx] = 0;
  
  if (file) {
    int status = 0;
    numValsRead = 0;
    int row = 0, col= 0;

    if (printLevel > 2)
      printf("Determining ColsPerRow in %s:\n  ",fileName);

    // Read through once to determine the ColsPerRow
    while (status != EOF) {
      double val;
      status = fscanf(file,"%lf",&val);
  
      if (status != EOF) {
        // Count row values
	if (val != 0.0) numColsPerRow[row]++;	
	numValsRead++;

        // Increment matrix indices
        col++;	
	if ((numValsRead % systemSize)==0) {
	  row++;
	  col=0;
	  if ( (printLevel > 2) && (systemSize > 10) && ((row % (systemSize/10) == 0)) ) {
	    printf(" %d ",++progress);
	    fflush(stdout);
	  }
	}
      }
    } // end while not eof

    // Find the maximum number of cols per row
    maxCols = -1;
    totalNumNonZeroVals = 0;
    
    for (idx=0; idx<systemSize; idx++) {
      totalNumNonZeroVals+= numColsPerRow[idx];
      if (numColsPerRow[idx] > maxCols)
	maxCols = numColsPerRow[idx];
    }
    
    if (printLevel > 2) printf("\n  Reading matrix rows:\n  ",maxCols);
      
    matrix = new ELLMatrix(systemSize,systemSize,maxCols,gNumDevices);
    
    progress = 0;
    
    // Read through again to set the values
    rewind(file);
    row = 0; col = 0; status = 0;
    while (status != EOF) {
      double val;
      status = fscanf(file,"%lf",&val);
  
      if (status != EOF) {
        // Populate non-zero values in matrix
	if (val != 0.0) 
	  matrix->setValue(row,col,val);
	
        // Increment column and row indices
        col++;	
	if ((col % systemSize)==0) {
	  row++;
	  col=0;
	  if ( (printLevel > 2) && ((row % (systemSize/10) == 0)) ) {
	    printf(" %d ",++progress);
	    fflush(stdout);
	  }
	}
      }
    } // end while not eof

    if (printLevel > 2) printf("\n\n");
    
    fclose(file);    
  } // end of if file opened correctly
 
  printf("\nCounted at most %d values in each row.\n",maxCols);

  numValsRead = totalNumNonZeroVals;
  
  return matrix;
}

int readIndicesFromFile(const char *fileName, int systemSize, int *indices, int printLevel) {
  int status = 0;	
  FILE *file = fopen(fileName,"r");
  
  if (!file) {
    printf("Error:  Could not read indices file %s\n",fileName);
    return -1;
  }  

  int numRead = 0;
  for (int idx=0; idx<systemSize; idx++) {
    int index = -1;

    status = fscanf(file,"%d",&index);
    if (status == EOF) index = -1;
    else numRead++;

    indices[idx] = index;
  }

  fclose(file);

  return numRead;
}


ELLMatrix *readMatrixFromELLFile(const char *fileName, int &numValsRead, int printLevel) {
  int status = 0;	
  FILE *file = fopen(fileName,"r");
  numValsRead = -1;
  
  if (!file) {
    printf("Error:  Could not read ELL matri file %s\n",fileName);
    return NULL;
  }

  int numRows;
  int numCols;
  int numColsPerRow;

  fscanf(file,"%d",&numRows);
  fscanf(file,"%d",&numCols);
  fscanf(file,"%d",&numColsPerRow);

  ELLMatrix *matrix = new ELLMatrix(numRows,numCols,numColsPerRow,gNumDevices);

  int numNonZeroVals = 0;

  for (int idx=0; idx<matrix->getSize(); idx++) {
    REAL_TYPE val = 0.0;
    int       index = -1;

    if ( (printLevel >= 1) && (matrix->getSize() > 50) ) {
      if ( (idx % (matrix->getSize()/5)) == 0 )  printf("*");
      if ( (idx % (matrix->getSize()/50)) == 0 ) printf(".");
      fflush(stdout);
    }
    status = fscanf(file,"%lf",&val);

    if (status == EOF) {
      matrix->rawSetValue(idx,-1,0.0);
      // matrix->data[idx] = 0.0;
      // matrix->indices[idx] = -1;
    }

    else {
      if (isnan(val)) val = 0.0;
      // matrix->data[idx] = val;

      if (val != 0.0) numNonZeroVals++;

      status = fscanf(file,"%d",&index);

      // printf("DBG:  Attempting to place value:  idx=%d, index=%d, val=%0.3lf\n",idx,index,val);
      
      if (status != EOF) matrix->rawSetValue(idx,index,val);
      else               matrix->rawSetValue(idx,-1,0.0);
      // if (status != EOF)  matrix->indices[idx] = index;
      // else                matrix->indices[idx] = -1;
    }
  }

  if ( (printLevel >= 1) && (matrix->getSize() > 50) ) 
    printf("\n");	

  fclose(file);

  numValsRead = numNonZeroVals;
  
  return matrix;
}
    

Vector *readVectorFromFile(const char *fileName, int systemSize, int &numValsRead, int printLevel, int *indices) {
  FILE *file = fopen(fileName,"r");
  int progress = 0;
  Vector *vector = NULL;

  numValsRead = -1;  

  if (file) {
    int status = 0;
    numValsRead = 0;
    int idx = 0;

    if (printLevel > 2)
      printf("Reading vector values from %s:\n  ",fileName);
    
    vector = new Vector(systemSize,gNumDevices);
    
    // Read the values out of the file and into the data array
    int jdx = 0;
    while (status != EOF) {
      double val;

      // If this index corresponds with one of the missing cols/rows
      // indicated in indices, don't read but instead just assume it is 0.0
      if (indices && (indices[jdx] == idx)) {
        val = 0.0;
        jdx++;
      }
      else {
        status = fscanf(file,"%lf",&val);
      }
  
      if (status != EOF) {
        vector->setValue(idx,val);
	numValsRead++;
        idx++;
	if ( (printLevel > 2) && ((numValsRead % (systemSize/10) == 0)) ) {
	  printf(" %d ",++progress);
	  fflush(stdout);
	}
      }
    } // end of while not eof

    if (printLevel > 2) printf("\n\n");
    
    fclose(file);    
  } // end of if file opened correctly
  
  return vector;
}

int writeVectorToFile(const char *fileName, Vector &vector, int printLevel) {
  FILE *file = fopen(fileName,"w");
  int numValsWritten = -1;
  int progress = 0;
  
  if (!file) {
    printf("Error:  Could not write to file %s\n",fileName);
    return 0;
  }
      
  numValsWritten = 0;

  vector.copyFromGPU();
  
  if (printLevel > 2)
    printf("Writing vector values to %s:\n  ",fileName);
    
  // Read the values out of the file and into the data array
  for (int idx=0; idx<vector.getSize(); idx++) {
    fprintf(file,"%lf\n",vector.getValue(idx));
    numValsWritten++;
    
    if ( (printLevel > 2) && ((numValsWritten % (vector.getSize()/10) == 0)) ) {
      printf(" %d ",++progress);
      fflush(stdout);
    }
  }

  if (printLevel > 2) printf("\n\n");
    
  fclose(file); 
  return numValsWritten;
} // end of if file opened correctly


