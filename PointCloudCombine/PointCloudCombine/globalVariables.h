#ifndef __globalVariables_h_
#define __globalVariables_h_

#include "gl3w.h"
#include "GLSLprogram.h"
#include "meshObject.h"
#include "vector.h"
#include "arrayList.h"

#define fzFar 2000.0f
#define fzNear 1.5f
#define near_height 1.0f
#define MAX_OBJECTS 5

/* struct for drawing a rectangular selection on the screen 
 * TODO Separate the Struct and the methods from the Program.c to separete file/header
 * and suport multy object if needed
 */
struct {
	GLfloat edgeArray[8];
	GLSLprogram *simpleProgram;
	GLuint edgeBuffer;
	GLuint recVOA;
	GLint updated;
} wireRect;

/* Array of objects to operate on in the program */
meshObject **objectsArray;
GLfloat ppMatrix[16], orientationMatrix[16];
Vector3f position;
GLfloat aspect;
GLint shiftButtonDown;
GLint window_height, window_width;
arrayListf *userDefinedSegmentVertex;
arrayListui *userDefinedSegmentColor;
float markingColor[4], reverseMarkingColor[4];
/* Number og objects int the program */
int objectsCount;

/* TODO move with struct */
void wireRectUpdateBuffer(void);
void initWireRecEdges(int x, int y);
void updateWireRecEdges(int x, int y);

void colorIndexes( meshObject* obj, arrayListui* list, float* color );

#endif