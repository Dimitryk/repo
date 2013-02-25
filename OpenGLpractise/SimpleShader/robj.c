#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "robj.h"

#define BUFFERSIZE 512
#define WHITESPACE " \t\n\r"

typedef struct obj_t_{
	FILE *fp;
	int vertexCount, elementCount, normalCount, textureCount;
	int (*vertexData_cb)(float *, int);
	int (*normalData_cb)(float *, int);
	int (*textureData_cb)(float *, int);
	int (*vertexFace_cb)(unsigned int *, unsigned int *, unsigned int *, int);
	char buffer[BUFFERSIZE];
}obj_t;

obj_p openOBJ(char* fileName){
	obj_t *obj;
	obj = (obj_t*)malloc(sizeof(obj_t));
	obj->fp = fopen(fileName, "rb");
	if (obj->fp == NULL) /* Return NULL on failure */
        return NULL;
	obj->vertexData_cb = NULL;
	obj->textureData_cb = NULL;
	obj->textureData_cb = NULL;
	obj->vertexFace_cb = NULL;
	return obj;
}

void setVertex_cb(obj_p obj, int (*vertexFunction)(float *vertex, int)){
	obj->vertexData_cb = vertexFunction;
}
void setNormal_cb(obj_p obj, int (*normalFunction)(float *normal, int)){
	obj->normalData_cb = normalFunction;
}
void setTexture_cb(obj_p obj, int (*textureFunction)(float *texture, int)){
	obj->textureData_cb = textureFunction;
}
void setFace_cb(obj_p obj, int (faceFunction)(unsigned int* vertexIndex, unsigned int* textureIndex, unsigned int* normalIndex,int)){
	obj->vertexFace_cb = faceFunction;
}


static void parseVertex(obj_p obj){
	float data[3];
	data[0] = atof( strtok(NULL, WHITESPACE) );
	data[1] = atof( strtok(NULL, WHITESPACE) );
	data[2] = atof( strtok(NULL, WHITESPACE) );
	obj->vertexData_cb(data, 3);
}
static void parseNormal(obj_p obj){
	float data[3];
	data[0] = atof( strtok(NULL, WHITESPACE) );
	data[1] = atof( strtok(NULL, WHITESPACE) );
	data[2] = atof( strtok(NULL, WHITESPACE) );
	obj->normalData_cb(data, 3);
}
static void parseTexture(obj_p obj){
	float data[2];
	data[0] = atof( strtok(NULL, WHITESPACE) );
	data[1] = atof( strtok(NULL, WHITESPACE) );
	obj->textureData_cb(data, 2);
}
static void parseFace(obj_p obj){
	char *temp_str;
	char *token;
	int vertex_count = 0;
	unsigned int vertexIndex[4], textureIndex[4], normalIndex[4];
	
	while( (token = strtok(NULL, WHITESPACE)) != NULL)
	{
		textureIndex[vertex_count] = 0;
		normalIndex[vertex_count] = 0;

		vertexIndex[vertex_count] = (unsigned int)atoi( token );
		
		if( strstr(token, "//") != NULL)  //normal only
		{
			temp_str = strchr(token, '/');
			temp_str++;
			normalIndex[vertex_count] = (unsigned int)atoi( ++temp_str );
		}
		else if( strstr(token, "/") != NULL)
		{
			temp_str = strchr(token, '/');
			textureIndex[vertex_count] = (unsigned int)atoi( ++temp_str );

			if( strstr(temp_str, "/") != NULL)
			{
				temp_str = strchr(temp_str, '/');
				normalIndex[vertex_count] = (unsigned int)atoi( ++temp_str );
			}
		}
		
		vertex_count++;
	}

	obj->vertexFace_cb(vertexIndex, textureIndex, normalIndex, vertex_count);
}
static void parseFile(obj_p obj){
	char *current_token;
	while(fgets(obj->buffer, BUFFERSIZE, obj->fp)){
		current_token = strtok( obj->buffer, WHITESPACE);
		
		//skip comments
		if( current_token == NULL || current_token[0] == '#')
			continue;

		//parse objects
		else if( strcmp(current_token, "v") == 0) 
		{//process vertex
			if(obj->vertexData_cb == NULL)
				continue;
			parseVertex(obj);
			obj->vertexCount++;
		}
		
		else if( strcmp(current_token, "vn") == 0 ) 
		{
			//process vertex normal
			if(obj->normalData_cb == NULL)
				continue;
			parseNormal(obj);
			obj->normalCount++;
		}
		
		else if( strcmp(current_token, "vt") == 0 ) 
		{
			//process vertex texture
			if(obj->textureData_cb == NULL)
				continue;
			parseTexture(obj);
			obj->textureCount++;
		}
		
		else if( strcmp(current_token, "f") == 0 ) 
		{
			//process face
			if(obj->vertexFace_cb == NULL)
				continue;
			parseFace(obj);
			obj->elementCount++;
		}
	}
}

int readOBJ(obj_p obj){
	parseFile(obj);
	return 1;
}
int closeOBJ(obj_p obj){
	return fclose(obj->fp);
}