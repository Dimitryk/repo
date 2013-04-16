#ifndef __mouseFunctions_h_
#define __mouseFunctions_h_

#include "vector.h"
#include "globalVariables.h"

void mouseMoveFunction(GLint x, GLint y, Vector3f *positionVector, float *rotationMatrix);
void mouseFunction(int button, int action, int x, int y);
void mouseZoomFnc(GLint dir, Vector3f *positionVector);

void userPickVectors(		float* edges, 
							float* v1, float* v2, float* v3, float* v4,
							float* origo);

#endif