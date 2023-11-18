#ifndef __GLOBALSINGLETON_H
#define __GLOBALSINGLETON_H

#include<stdio.h>
#include<stdlib.h>
#include<cuda_runtime_api.h>

#include "defs.h"

class GlobalSingleton {
public:
  GlobalSingleton() {GlobalSingleton::initialize();}
  static void getCudaStream(int deviceIdx, cudaStream_t &stream);
  static void configureThreadsAndBlocks(int bpt, int tpb=1024);
  static int getThreadsPerBlock();
  static int getBlocksPerGrid();
protected:
  static void initialize();
};

#endif
