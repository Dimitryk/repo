#ifndef __geometryMesh_h_
#define __geometryMesh_h_

typedef struct geometryMesh_t{
	float* vertexArray;
	float* normalsArray;
	int vertexCount;
	unsigned int* elementArray;
	int elementCount;
}geometryMesh;

#endif //geometryMesh.h