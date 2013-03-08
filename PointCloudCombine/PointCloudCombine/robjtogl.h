
#ifndef __robjtogl_h_
#define __robjtogl_h_
#include "gl3w.h"

int openOBJ_file(char* filename);

void setVertexArray(GLfloat **vertexArray, int* size);

void setElementArray(GLuint **elementArray, int *size);

int readOBG_file(void);

#endif