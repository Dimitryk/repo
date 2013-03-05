#ifndef __GLSLprogram_h_
#define __GLSLprogram_h_
#include "gl3w.h"

typedef struct GLSLprogram_t{
	GLuint ID;
	GLuint ppMatrixBlock, modelToCameraMatrixUnif, normalModelToCameraMatrixUnif;
	GLuint ambientIntensityUnif, lightIntensityUnif, dirToLightUnif;
	GLint linkStatus;
	GLchar *infolog;
	int g_projectionBlockIndex;

}GLSLprogram;

GLSLprogram* createShaderProgram(char* vertexShaderFileName, char* fragmentShaderFilename, const int);


void finalizeProgram(GLSLprogram* program);

#endif