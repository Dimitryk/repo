#include <stdlib.h>
#include "robj.h"
#include "arrayList.h"
#include "robjtogl.h"

static obj_p obj;
static arrayListf *vertexDataList = NULL;
static arrayListui *elementDataList = NULL;
static GLfloat **outputVertexArray;
static GLuint **outputElementArray;
static int *vertexCount, *elementCount;

static int fillVertexDataList(float *vertexVector, int size){
	return addToArrayListfv(vertexDataList,vertexVector,size);
}

static int fillElementDataList(unsigned int *vertexIndex,unsigned int *textureindex, unsigned *normalIndex, int size){
	return addToArrayListuiv(elementDataList, vertexIndex,size);
}

int openOBJ_file(char* filename){
	obj = openOBJ(filename);
	if( obj == NULL )
		return 0;
	if(vertexDataList == NULL)
		vertexDataList = createArrayListf();
	else
	{
		vertexDataList->index = 0;
		vertexDataList->lenght = 0;
	}
	if(elementDataList == NULL)
		elementDataList = createArrayListui();
	else
	{
		elementDataList->index = 0;
		elementDataList->lenght = 0;
	}
	return 1;
}

void createVertexArrayOBJ(GLfloat **vertexArray, int *size){
	outputVertexArray = vertexArray;
	vertexCount = size;
	setVertex_cb(obj, fillVertexDataList);
}
void createElementArrayOBJ(GLuint **elementArray, int *size){
	outputElementArray = elementArray;
	elementCount = size;
	setFace_cb(obj,fillElementDataList);
}

static int arrayListftoGLfloat(arrayListf *list, GLfloat **dst){
	if(sizeof(float)!=sizeof(GLfloat))
		return 0;
	*dst = (GLfloat*)malloc(sizeof(GLfloat) * list->lenght);
	if( *dst == NULL )
		return 0;
	memcpy(*dst, list->data, sizeof(GLfloat) * list->lenght);
	return 1;
}

static int arrayListuitoGLuint(arrayListui *list, GLuint **dst){
	if(sizeof(unsigned int)!=sizeof(GLuint))
		return 0;
	*dst = (GLuint*)malloc(sizeof(GLuint) * list->lenght);
	if( *dst == NULL )
		return 0;
	memcpy(*dst, list->data, sizeof(unsigned int) * list->lenght);
	return 1;
}

static void cleanrobj(void){
	arrayListf *tempVertexList;
	arrayListui *tempElementList;

	tempVertexList = vertexDataList;
	vertexDataList = NULL;
	deleteArrayListf(tempVertexList);
	tempElementList = elementDataList;
	elementDataList = NULL;
	deleteArrayListui(tempElementList);
}

int readOBG_file(void){
	if(!readOBJ(obj))
		return 0;
	if(!arrayListftoGLfloat(vertexDataList, outputVertexArray)){
		cleanrobj();
		return 0;
	}
	*vertexCount = vertexDataList->lenght;
	if(!arrayListuitoGLuint(elementDataList, outputElementArray)){
		cleanrobj();
		return 0;
	}
	*elementCount = elementDataList->lenght;
	cleanrobj();
	return closeOBJ(obj);
}
