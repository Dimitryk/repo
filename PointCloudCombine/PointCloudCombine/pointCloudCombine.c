#include <stdlib.h>
#include "pointCloudCombine.h"
#include "octTree.h"
#include "vector.h"
#include "arrayList.h"

#define EPSILON 0.000001f
#define MAX_SQ_DISTANCE 15.0f

#define cross3v(dest, v1, v2) \
			(dest)[0] = (v1)[1] * (v2)[2] - (v1)[2] * (v2)[1];\
			(dest)[1] = (v1)[2] * (v2)[0] - (v1)[0] * (v2)[2];\
			(dest)[2] = (v1)[0] * (v2)[1] - (v1)[1] * (v2)[0]
#define dot3v(v1, v2) ((v1)[0]*(v2)[1] + (v1)[0]*(v2)[1] + (v1)[2]*(v2)[2])
#define sub3v(dest, v1, v2) \
			(dest)[0] = (v1)[0] - (v2)[0];\
			(dest)[1] = (v1)[1] - (v2)[1];\
			(dest)[2] = (v1)[2] - (v2)[2]

#define midPointVector3f(dest,min,max) (dest).x = (min).x + ((max).x - (min).x) / 2;\
				(dest).y = (min).y + ((max).y - (min).y) / 2;\
				(dest).z = (min).z + ((max).z - (min).z) / 2

#define getQuadrant(point, mid) ((((point)[0] > (mid).x) ? 1 : 0 ) + \
	(((point)[2] > (mid).z) ? 2 : 0) + (((point)[1] > (mid).y) ? 4 : 0) )

#define NUM_DIMENSIONS 3

typedef struct triangulatedDataStruct_t{
	float* vertexArrayPntr;
	unsigned int* elementIndexPntr;
}triDataPack;


static int rayIntersectsTriangle( float** triangle, float* ray, float* rayOrigin ){
	float edge1[3], edge2[3], tvec[3], pvec[3], qvec[3];
	float det, invDet;
	float u, v, t;
	
	sub3v(edge1, triangle[1], triangle[0]);
	sub3v(edge2, triangle[2], triangle[0]);

	cross3v(pvec, ray, edge2);

	det = dot3v(edge1, pvec);

	if( det > -EPSILON && det < EPSILON )
		return 0;
	invDet = 1.0f/det;

	sub3v(tvec, rayOrigin, triangle[0]);

	u = dot3v(tvec, pvec) * invDet;

	if( u < 0.0f || u > 1.0f )
		return 0;

	cross3v(qvec, tvec, edge1);

	v = dot3v(ray, qvec) * invDet;
	if( v < 0.0f || v > 1.0f )
		return 0;
	
	t = dot3v(edge2, qvec) * invDet;
	return 1;
}

static int rayIntersectsDataOctTree( arrayListui *triList, float* vertexData, float* ray, float *rayOrigin){
	float* triangle[3];
	unsigned int *element = triList->data, *endPntr = triList->data + triList->lenght;
	for( ; element < endPntr; element += 3 ){
		triangle[0] = vertexData + element[0];
		triangle[1] = vertexData + element[1];
		triangle[2] = vertexData + element[2];

		if(rayIntersectsTriangle(triangle, ray, rayOrigin))
			return 1;
	}
	
	return 0;
}

static int recRayIntersectsOctTree( octTreeNode_p root, float* vertexData, float* ray, float* rayOrigin ){
	if( rayOctreeIntersept(root, ray, rayOrigin, MAX_SQ_DISTANCE) ){
		int i;
		if( getNodesChildren(root) == NULL ){
			if( getNodesDataPntr(root) != NULL )
				return rayIntersectsDataOctTree((arrayListui*)getNodesDataPntr(root), vertexData, ray, rayOrigin);
			else
				return 0;
		}
		for( i = 0; i < 8; i++ ){
			if(recRayIntersectsOctTree(getNodesChild(root, i),vertexData, ray, rayOrigin))
				return 1;
		}
	}
	return 0;
}

static int rayIntersectsModel( octTree_p tree, float* vertexData, float* ray, float* rayOrigin ){
	return 1;
}

/* creates a list of vertex indexes that are beeing represented by model and data triangulated mesh
 * 
 */
static arrayListui* modelNormalShootToOctTree(	float* modelVertexArray, float* modelNormalArray, int modelSize,
										octTreeNode_p root, float* dataVertexArray){
	arrayListui *elementList;
	float *modelVertexIndex;
	float *endPntr = modelVertexArray + modelSize;

	elementList = createArrayListui();

	for( modelVertexIndex = modelVertexArray; modelVertexIndex < endPntr; modelVertexIndex += NUM_DIMENSIONS, modelNormalArray += NUM_DIMENSIONS ){
		if(recRayIntersectsOctTree(root, dataVertexArray, modelNormalArray, modelVertexIndex)){
			addToArrayListui(elementList, modelVertexIndex - modelVertexArray);
		}
	}
	return elementList;
}

static void findMinMaxBoundsArray3f( float* vertexArray, int vertexCount, Vector3f *min, Vector3f *max ){
	float *endPntr = vertexArray + vertexCount;

	min->x = max->x = vertexArray[0];
	min->y = max->y = vertexArray[1];
	min->z = max->z = vertexArray[2];
	for( vertexArray += 3 ; vertexArray < endPntr; vertexArray += 3){
		if(min->x > vertexArray[0])
			min->x = vertexArray[0];
		else if(max->x < vertexArray[0])
			max->x = vertexArray[0];
		if(min->y > vertexArray[1])
			min->y = vertexArray[1];
		else if(max->y < vertexArray[1])
			max->y = vertexArray[1];
		if(min->z > vertexArray[2])
			min->z = vertexArray[2];
		else if(max->z < vertexArray[2])
			max->z = vertexArray[2];
	}
}

static int recAddTriangleData( octTreeNode_p root, float* pointData, unsigned int* triangle ){
	octTreeNode_p children = getNodesChildren(root);
	Vector3f minBound, maxBound, mid;
	arrayListui *data;
	int pointQuad1, pointQuad2, pointQuad3;

	if(children == NULL){
		if ( (data = (arrayListui*)getNodesDataPntr(root)) == NULL ){
			data = createArrayListui();
			if( data == NULL )
				return 0;
			setNodesDataPntr(root, (void*)data);
			return addToArrayListuiv(data, triangle, 3);
		}else
			return addToArrayListuiv(data, triangle, 3);
	}
	minBound = getNodeMinBound(root);
	maxBound = getNodeMaxBound(root);

	midPointVector3f(mid, minBound, maxBound);

	pointQuad1 = getQuadrant(pointData + triangle[0], mid);
	pointQuad2 = getQuadrant(pointData + triangle[1], mid);
	pointQuad3 = getQuadrant(pointData + triangle[2], mid);

	if(!recAddTriangleData(getNodesChild(root, pointQuad1), pointData, triangle))
		return 0;
	if(pointQuad2 != pointQuad1)
		if(!recAddTriangleData(getNodesChild(root, pointQuad2), pointData, triangle))
			return 0;
	if(pointQuad3 != pointQuad1 && pointQuad3 != pointQuad2)
		if(!recAddTriangleData(getNodesChild(root, pointQuad3), pointData, triangle))
			return 0;
	return 1;
}

/* Given the index list of vertexes to delete, 
 * creates new array and coppys all the values to new array, exept the one with delete index
 */
static float* restructureVertexArray( float* vertexArray, int size, arrayListui* indexToDeleteList ){
	float *newVertexArray;
	int copyTo, copyFrom, numberOfElements, elementsCopyed, newSize = size - indexToDeleteList->lenght * NUM_DIMENSIONS;
	unsigned int *index, *endPntr;
	newVertexArray = (float*)malloc(newSize * sizeof(float));
	copyFrom = 0;
	elementsCopyed = 0;
	for( index = indexToDeleteList->data, endPntr = indexToDeleteList->data + indexToDeleteList->size;
		index < endPntr; index++ ){
			copyTo = (*index) * NUM_DIMENSIONS;
			if( (numberOfElements = (copyTo - copyFrom)  ) > 0 ){
				elementsCopyed += numberOfElements;
				if(elementsCopyed > newSize){//To many elements;
					free(newVertexArray);
					return NULL;
				}
				memcpy(newVertexArray + elementsCopyed, vertexArray + copyFrom, numberOfElements * sizeof(float));
				copyFrom += (copyTo + 1);
				
			}
			else{
				//Error
				free(newVertexArray);
				return NULL;
			}

	}

}

int octTreeAddTriData( octTreeNode_p root, void* dataPack, int size ){
	float *vertexArray = ((triDataPack*)dataPack)->vertexArrayPntr;
	unsigned int* elementIndexArray = ((triDataPack*)dataPack)->elementIndexPntr;
	unsigned int* endPntr;

	for(endPntr = elementIndexArray + size; elementIndexArray < endPntr; elementIndexArray += 3){
		if(!recAddTriangleData(root, vertexArray, elementIndexArray))
			return 0;
	}
	return 1;
}

void deleteDataFnc(void* dataList){
	deleteArrayListui((arrayListui*)dataList);
}

octTree_p createOctTreeTrangulated(	float* vertexArray, int vertexCount,
								unsigned int* elementsArray, int elementCount ){
	octTree_p tree;
	Vector3f minBound, maxBound;
	triDataPack dataPacked;
	
	dataPacked.vertexArrayPntr = vertexArray;
	dataPacked.elementIndexPntr = elementsArray;

	findMinMaxBoundsArray3f( vertexArray, vertexCount, &minBound, &maxBound );
	tree = createOctTree(minBound, maxBound, (void*)(&dataPacked), elementCount, 5, octTreeAddTriData, deleteDataFnc);
	return tree;
}

void pointCloudCombine( float* modelVertexArray, float* modelNormalArray, int modelSize,
						float* dataVertexArray, int dataSize, 
						unsigned int* dataElementArray, int dataElementCount ){
	octTree_p modelTree;
	arrayListui *unionVertexIndexList;

	modelTree = createOctTreeTrangulated(dataVertexArray, dataSize, dataElementArray, dataElementCount);
	if( modelTree == NULL )
		return;

	unionVertexIndexList = modelNormalShootToOctTree(modelVertexArray, modelNormalArray, modelSize, getRootOctTree(modelTree), dataVertexArray);
	if( unionVertexIndexList->lenght == 0 )
		return;
}
