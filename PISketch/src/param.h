#ifndef  _param_H_
#define  _param_H_

#include <string.h>
#include <stdint.h>
#include <random>
#include <string>
#include <cmath>
#include <math.h>
#include <vector>
#include <map>
#include <set>
#include <iostream>
#include <time.h>
#include <stdlib.h>
#include <sys/time.h>

#define TOT_PKTS 10000000
#define COUNTER_PER_BUCKET 100
#define BUCKET_NUM 10000
#define CMP_TOT KEY
#define CMP_FILTER KEY
uint32_t NUM_OF_WIN = 1000;
uint32_t WIN_THRE = 500;
double SP_THRE = 2.0;
uint32_t WIN_THRE_DETECT = WIN_THRE;
double SP_THRE_DETECT = SP_THRE;

#endif
