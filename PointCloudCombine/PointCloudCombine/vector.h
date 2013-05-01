#ifndef __GLVector_h_
#define __GLVector_h_
#include "gl3w.h"

typedef struct t_vector3f_ {
	GLfloat x;
	GLfloat y;
	GLfloat z;
} Vector3f;

typedef struct t_vector3d_ {
	double x;
	double y;
	double z;
} Vector3d;


typedef struct t_vector3ui_ {
	GLuint x;
	GLuint y;
	GLuint z;
} Vector3ui;

#endif