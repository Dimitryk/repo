#include <stdio.h>
#include <stdlib.h>
#include "GLSLshader.h"


static GLchar* fileToShaderString(char *file);

GLSLshader* createShader(GLenum type, char* fileName){
	GLSLshader *shader;

	shader = (GLSLshader*)malloc(sizeof(GLSLshader));
	shader->compiled = GL_FALSE;
	shader->fileName = fileName;
	shader->infolog = NULL;
	shader->type = type;
	shader->ID = glCreateShader( type );
	shader->sourceString = fileToShaderString(fileName);

	return shader;
}

int compileShader(GLSLshader *shader){
	GLint status;
	//Only supports single string
	glShaderSource( shader->ID, 1,(const GLchar**)&(shader->sourceString), NULL );
	glCompileShader( shader->ID );
	glGetShaderiv(shader->ID, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint infoLogLength;
		glGetShaderiv(shader->ID, GL_INFO_LOG_LENGTH, &infoLogLength);
        
		shader->infolog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength + 1);
        glGetShaderInfoLog(shader->ID, infoLogLength, NULL, shader->infolog);
        
		return 0;
    }
	shader->compiled = GL_TRUE;
	return 1;
}


void finalizeShader(GLSLshader *shader){
	if(shader->sourceString != NULL)
		free(shader->sourceString);
	if(shader->infolog != NULL)
		free(shader->infolog);
	glDeleteShader(shader->ID);
}

static GLchar* fileToShaderString(char *file)
{
    FILE *fptr;
    long length;
    char *buf;
 
    fptr = fopen(file, "rb"); /* Open file for reading */
    if (!fptr) /* Return NULL on failure */
        return NULL;
    fseek(fptr, 0, SEEK_END); /* Seek to the end of the file */
    length = ftell(fptr); /* Find out how many bytes into the file we are */
    buf = (char*)malloc(length+1); /* Allocate a buffer for the entire length of the file and a null terminator */
    fseek(fptr, 0, SEEK_SET); /* Go back to the beginning of the file */
    fread(buf, length, 1, fptr); /* Read the contents of the file in to the buffer */
    fclose(fptr); /* Close the file */
    buf[length] = 0; /* Null terminator */
 
    return buf; /* Return the buffer */
}