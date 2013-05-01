#include <stdlib.h>
#include "objectGeometryMeshConverter.h"

geometryMesh* convertObjectGeometryMesh( meshObject* object ){
	geometryMesh *geometry;

	geometry = (geometryMesh*)malloc(sizeof(geometryMesh));
	if(geometry == NULL)
		return NULL;
	geometry->elementArray = object->elementArray;
	geometry->elementCount = object->elementCount;
	geometry->normalsArray = object->normalsArray;
	geometry->vertexArray = object->vertexArray;
	geometry->vertexCount = object->vertexCount;
	return geometry;
}