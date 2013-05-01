
#ifndef __icp_h_
#define __icp_h_
#include "vector.h"
#define scalar double

/* Runs an ICP algorithm on two point clouds
 * returns best registration matrix (float[16] colon major)
 */
float* globalICPRegistration( float* model, int modelSize, Vector3f min, Vector3f max, float *data, int dataSize, int maxIterations);


#endif