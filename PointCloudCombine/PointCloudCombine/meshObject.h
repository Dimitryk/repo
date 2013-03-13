#ifndef __meshObject_h_
#define __meshObject_h_
#include "gl3w.h"
#include "GLSLprogram.h"


typedef struct meshObject_t{
	GLuint vertexBuffer, colorBuffer, elementBuffer, normalsBuffer;
	GLuint vao;
	GLfloat *vertexArray, *normalsArray;
	GLuint *elementArray;
	unsigned int vertexCount, elementCount;
	GLSLprogram *shaderProgram;
}meshObject;

meshObject* createMeshObject(char* filename, GLSLprogram* shaderProgram);

void drawMeshObject(meshObject* object);

void deleteMeshObject(meshObject* object);

#endif