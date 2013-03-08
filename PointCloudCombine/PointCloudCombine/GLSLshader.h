#ifndef __GLSLshader_h_
#define __GLSLshader_h_
#include "gl3w.h"

typedef struct GLSLshader_t {
	GLchar* sourceString;
	GLuint ID;
	GLint compiled;
	GLenum type;
	GLchar *infolog;
	char *fileName;
}GLSLshader;


GLSLshader* createShader(GLenum type, char* fileName);

int compileShader(GLSLshader *shader);

//NB shader must be deattached from the program
void finalizeShader(GLSLshader *shader);

#endif