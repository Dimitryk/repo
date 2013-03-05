#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "GLSLshader.h"
#include "GLSLprogram.h"

GLSLprogram* createShaderProgram(char* vertexShaderFileName, char* fragmentShaderFilename, const int g_projectionBlockIndex){
	GLSLprogram *program;
	GLSLshader vertexShader, fragmentShader;
	GLint maxL;
	char *err;

	program = (GLSLprogram*)malloc(sizeof(GLSLprogram));
	program->ID = glCreateProgram();
	program->infolog = NULL;

	vertexShader = *(createShader( GL_VERTEX_SHADER, vertexShaderFileName ));
	if(!compileShader(&vertexShader))
	{
		program->infolog = (char*) malloc(strlen(vertexShader.infolog) + sizeof(GLchar) * 50);
		sprintf(program->infolog, "ERROR: failed to compile vertex shader: %s \n",vertexShader.infolog);
		program->linkStatus = GL_FALSE;
		return program;
	}

	fragmentShader = *(createShader( GL_FRAGMENT_SHADER, fragmentShaderFilename ));

	if( !compileShader(&fragmentShader) )
	{
		program->infolog = (char*) malloc(strlen(fragmentShader.infolog) + sizeof(GLchar) * 50);
		sprintf(program->infolog, "ERROR: failed to compile fragment shader: %s \n",fragmentShader.infolog);
		program->linkStatus = GL_FALSE;
		return program;
	}

	glAttachShader( program->ID, vertexShader.ID );
	glAttachShader( program->ID, fragmentShader.ID );

	glLinkProgram( program->ID );

	glGetProgramiv( program->ID, GL_LINK_STATUS, &(program->linkStatus) );
	if( program->linkStatus  != GL_TRUE )
	{
		/* Noticed that glGetProgramiv is used to get the length for a shader program, not glGetShaderiv. */
		glGetProgramiv(program->ID, GL_INFO_LOG_LENGTH, &maxL);

		/* The maxLength includes the NULL character */
		program->infolog = (GLchar *)malloc(maxL);

		/* Notice that glGetProgramInfoLog, not glGetShaderInfoLog. */
		glGetProgramInfoLog(program->ID, maxL, &maxL, program->infolog);
		return NULL;
	}
	glDetachShader(program->ID, vertexShader.ID);
	glDetachShader(program->ID, fragmentShader.ID);
	finalizeShader(&vertexShader);
	finalizeShader(&fragmentShader);

	/* NB this values are higly implementation dependent and must match with Vertex and Fragment shaders
	 * for this project this will be the standard naming
	 */
	program->g_projectionBlockIndex = g_projectionBlockIndex;
	program->lightIntensityUnif = glGetUniformLocation(program->ID, "lightIntensity");
	program->ambientIntensityUnif = glGetUniformLocation(program->ID, "ambientIntensity");
	program->modelToCameraMatrixUnif = glGetUniformLocation(program->ID, "modelToCameraMatrix");
	program->normalModelToCameraMatrixUnif = glGetUniformLocation(program->ID, "normalModelToCameraMatrix");
	program->dirToLightUnif = glGetUniformLocation(program->ID, "dirToLight");
	program->ppMatrixBlock = glGetUniformBlockIndex(program->ID, "projectionMatrix");
	glUniformBlockBinding(program->ID, program->ppMatrixBlock, program->g_projectionBlockIndex);

	return program;
}

void finalizeProgram(GLSLprogram* program){
	if(program->infolog != NULL)
		free(program->infolog);
	free(program);
}