
#ifndef __robjtogl_h_
#define __robjtogl_h_
#include "gl3w.h"

int openOBJ_file(char* filename);

void createVertexArrayOBJ(GLfloat **vertexArray, int* size);

void createElementArrayOBJ(GLuint **elementArray, int *size);

int readOBG_file(void);

#endif