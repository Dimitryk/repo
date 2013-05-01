#include <stdlib.h>
#include <math.h>
#include "meshObject.h"
#include "robjtogl.h"
#include "rplytogl.h"
#include "gMatrix.h"
#include "vector.h"

/* Calculates surface Normal of a triangle */
static Vector3f calculateNormal(GLfloat triagleVertexArray[]){
	Vector3f Normal; 
	GLfloat l;

	Vector3f U = {	triagleVertexArray[3] - triagleVertexArray[0],
		triagleVertexArray[4] - triagleVertexArray[1],
		triagleVertexArray[5] - triagleVertexArray[2]};

	Vector3f V = {	triagleVertexArray[6] - triagleVertexArray[0],
		triagleVertexArray[7] - triagleVertexArray[1],
		triagleVertexArray[8] - triagleVertexArray[2]};

	Normal.x = U.y * V.z - U.z * V.y;
	Normal.y = U.z * V.x - U.x * V.z;
	Normal.z = U.x * V.y - U.y * V.x;
	l = (GLfloat)sqrt(Normal.x * Normal.x + Normal.y * Normal.y + Normal.z * Normal.z);
	Normal.x /= l;
	Normal.y /= l;
	Normal.z /= l;

	return Normal;
}
/* Calculates and sets the normal Array of a given object
 * Given the element index List and vertex list
 * NB normals are not uniform
 */
static int createNormArray(meshObject *object){
	Vector3f tempNorm;
	GLfloat triVertexArray[9];
	unsigned int i;
	GLfloat *normSumArray;
	GLfloat *vertexArray = object->vertexArray;
	GLuint *elementArray = object->elementArray;

	normSumArray = (GLfloat*) calloc(object->vertexCount, sizeof(GLfloat));
	if(normSumArray == NULL)
		return -1;

	for(i = 0; i < object->elementCount; i += 3){
		triVertexArray[0] = vertexArray[elementArray[i] * 3 + 0];
		triVertexArray[1] = vertexArray[elementArray[i] * 3 + 1];
		triVertexArray[2] = vertexArray[elementArray[i] * 3 + 2];
		triVertexArray[3] = vertexArray[elementArray[i + 1] * 3 + 0];
		triVertexArray[4] = vertexArray[elementArray[i + 1] * 3 + 1];
		triVertexArray[5] = vertexArray[elementArray[i + 1] * 3 + 2];
		triVertexArray[6] = vertexArray[elementArray[i + 2] * 3 + 0];
		triVertexArray[7] = vertexArray[elementArray[i + 2] * 3 + 1];
		triVertexArray[8] = vertexArray[elementArray[i + 2] * 3 + 2];

		tempNorm = calculateNormal(triVertexArray);
		normSumArray[elementArray[i] * 3 + 0] += tempNorm.x;
		normSumArray[elementArray[i] * 3 + 1] += tempNorm.y;
		normSumArray[elementArray[i] * 3 + 2] += tempNorm.z;
		normSumArray[elementArray[i + 1] * 3 + 0] += tempNorm.x;
		normSumArray[elementArray[i + 1] * 3 + 1] += tempNorm.y;
		normSumArray[elementArray[i + 1] * 3 + 2] += tempNorm.z;
		normSumArray[elementArray[i + 2] * 3 + 0] += tempNorm.x;
		normSumArray[elementArray[i + 2] * 3 + 1] += tempNorm.y;
		normSumArray[elementArray[i + 2] * 3 + 2] += tempNorm.z;
	}
	object->normalsArray = normSumArray;
	return 1;
}

static int loadPLY(meshObject *object, char *filename){
	if (!openPLY_file(filename))
		return 0;
	object->vertexCount = createVertexArrayPLY(&(object->vertexArray));
	object->elementCount = createElementArrayPLY(&(object->elementArray));
	return read_PLY();
}

static int loadOBJ(meshObject *object, char *filename){
	if (!openOBJ_file(filename))
		return 0;
	createVertexArrayOBJ(&(object->vertexArray), &((int)(object->vertexCount)));
	createElementArrayOBJ(&(object->elementArray), &((int)(object->elementCount)));
	if(!readOBG_file())
		return 0;
	return 1;
}

static void genBuffers(meshObject *object){

	glGenBuffers(1, &(object->vertexBuffer));
	glBindBuffer(GL_ARRAY_BUFFER, object->vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, object->vertexCount * sizeof(object->vertexArray), object->vertexArray, GL_STATIC_DRAW);

	glGenBuffers(1, &(object->normalsBuffer));
	glBindBuffer(GL_ARRAY_BUFFER, object->normalsBuffer);
	glBufferData(GL_ARRAY_BUFFER, object->vertexCount * sizeof(object->normalsArray), object->normalsArray, GL_STATIC_DRAW);

	glGenBuffers(1, &(object->colorBuffer));
	glBindBuffer(GL_ARRAY_BUFFER, object->colorBuffer);
	glBufferData(GL_ARRAY_BUFFER, object->vertexCount / 3 * 4 * sizeof(object->colorArray), object->colorArray, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &(object->elementBuffer));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, object->elementBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, object->elementCount * sizeof(object->elementArray), object->elementArray, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

static void bindVOA(meshObject *object){
	GLint artibuteLocation;

	glGenVertexArrays(1, &(object->vao));
	glBindVertexArray(object->vao);

	glUseProgram(object->shaderProgram->ID);

	artibuteLocation = glGetAttribLocation(object->shaderProgram->ID, "position");
	glBindBuffer(GL_ARRAY_BUFFER, object->vertexBuffer);
	glEnableVertexAttribArray(artibuteLocation);
	glVertexAttribPointer(artibuteLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);

	artibuteLocation = glGetAttribLocation(object->shaderProgram->ID, "normal");
	glBindBuffer(GL_ARRAY_BUFFER, object->normalsBuffer);
	glEnableVertexAttribArray(artibuteLocation);
	glVertexAttribPointer(artibuteLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);

	artibuteLocation = glGetAttribLocation(object->shaderProgram->ID, "color");
	glBindBuffer(GL_ARRAY_BUFFER, object->colorBuffer);
	glEnableVertexAttribArray(artibuteLocation);
	glVertexAttribPointer(artibuteLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, object->elementBuffer);

	glUseProgram(0);

	glBindVertexArray(0);
}

void fillColor(float* colorArray, int size){
	float *endPntr = colorArray + size;
	for( ; colorArray < endPntr; colorArray += 4 ){
		colorArray[0] = 0.0f;
		colorArray[1] = -0.8f;
		colorArray[2] = -0.8f;
		colorArray[3] = 0.0f;
	}
}

meshObject* createMeshObject(char* fileName, GLSLprogram* shaderProgram){
	char *tolken;
	meshObject *object;

	object = (meshObject*)malloc(sizeof(meshObject));

	for(tolken = fileName; *tolken && *tolken != '.'; tolken++);
	tolken++;
	if(strcmp(tolken, "ply") == 0){
		if( !loadPLY(object, fileName )){
			free(object);
			return NULL;
		}
	}else if(strcmp(tolken, "obj") == 0){
		if( !loadOBJ(object, fileName)){
			free(object);
			return NULL;
		}
	}else{
		//TODO error handling;
		//deleteMeshObject(object);
		return NULL;
	}

	if(!createNormArray(object)){
		free(object->elementArray);
		free(object->vertexArray);
		return NULL;
	}

	object->colorArray = (float*)calloc((object->vertexCount / 3 * 4), sizeof(float));
	if( object->colorArray == NULL ){
		deleteMeshObject(object);
		return NULL;
	}
	//fillColor(object->colorArray, object->vertexCount / 3 * 4);
	genBuffers(object);
	object->shaderProgram = shaderProgram;
	bindVOA(object);
	return object;
}

void meshObjectChangeShaderProgram(meshObject* object, GLSLprogram* shaderProgram){
	object->shaderProgram = shaderProgram;
}

void meshObjectUpdateColorBuffer(meshObject* object){
	glBindBuffer(GL_ARRAY_BUFFER, object->colorBuffer);
	glBufferData(GL_ARRAY_BUFFER, object->vertexCount / 3 * 4 * sizeof(object->colorArray), object->colorArray, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void drawMeshObject(meshObject* object){
	GLSLprogram *shaderProgram = object->shaderProgram;
	GLenum err;

	glUseProgram(shaderProgram->ID);

	glUniformMatrix4fv(shaderProgram->modelToCameraMatrixUnif, 1, GL_FALSE, gGetTop());
	glUniformMatrix3fv(shaderProgram->normalModelToCameraMatrixUnif, 1, GL_TRUE, gGetTopNormal3fv());

	glBindVertexArray(object->vao);

	glDrawElements(GL_TRIANGLES, object->elementCount, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
	
	err = glGetError();
	glUseProgram(0);
}

void deleteMeshObject(meshObject* object){
	if(object->elementArray != NULL)
		free(object->elementArray);
	if(object->normalsArray != NULL)
		free(object->normalsArray);
	if(object->vertexArray != NULL)
		free(object->vertexArray);
	if(object->colorArray != NULL)
		free(object->colorArray);
	free(object);
}