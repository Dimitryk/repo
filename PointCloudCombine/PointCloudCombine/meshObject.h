#ifndef __meshObject_h_
#define __meshObject_h_
#include "gl3w.h"
#include "GLSLprogram.h"
#include "geometryMesh.h"

//TODO make meshObject opaque and provide vital get set methods
typedef struct meshObject_t{
	GLuint vertexBuffer, colorBuffer, elementBuffer, normalsBuffer;
	GLuint vao;
	GLfloat *vertexArray, *normalsArray, *colorArray;
	GLuint *elementArray;
	unsigned int vertexCount, elementCount;
	GLSLprogram *shaderProgram;
}meshObject;

/* Creates a mesh object
 * param
 * filename: file name/addres to the source for point mesh. Supported .ply or .obj formats 
 * shaderProgram: Program that will be used to render the mesh object
 */
meshObject* createMeshObject(char* filename, GLSLprogram* shaderProgram);

meshObject* meshObjectFromGeometry(geometryMesh* mesh, GLSLprogram* shaderProgram);

void meshObjectUpdateColorBuffer(meshObject* object);

/* Changes GLSL program pointer that will be used to render the mesh object
 */
void meshObjectChangeShaderProgram(meshObject* object, GLSLprogram* shaderProgram);

/* Renders mesh object with current GLSL program
 */
void drawMeshObject(meshObject* object);

/* frees allocated memmory by the mesh Object 
 */
void deleteMeshObject(meshObject* object);

#endif