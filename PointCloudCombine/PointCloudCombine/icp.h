
#ifndef __icp_h_
#define __icp_h_
#include "vector.h"

float* globalICPRegistration( float* model, int modelSize, Vector3f min, Vector3f max, float *data, int dataSize, int maxIterations);


#endif