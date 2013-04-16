#ifndef __globalVariables_h_
#define __globalVariables_h_

#include "gl3w.h"
#include "GLSLprogram.h"
#include "meshObject.h"
#include "vector.h"

#define fzFar 2000.0f
#define fzNear 1.5f
#define near_height 1.0f

struct {
	GLSLprogram *simpleProgram;
	GLuint edgeBuffer;
	GLuint recVOA;
	GLfloat edgeArray[8];
} wireRect;

float pickV1[4], pickV2[4], pickV3[4], pickV4[4], pickOrigo[4];
GLint shiftButtonDown;
meshObject objectsArray[5];
float ppMatrix[16], orientationMatrix[16];
Vector3f position;
int window_height, window_width;
int objectsCount;
float aspect;



#endif