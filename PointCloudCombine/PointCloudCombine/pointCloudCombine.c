#include <stdlib.h>
#include <math.h>
#include "pointCloudCombine.h"
#include "octTree.h"
#include "vector.h"
#include "arrayList.h"
#include "icp.h"

#define EPSILON 0.000001f
#define CLOSE_SQ_DISTANCE 1.0
#define MAX_SQ_DISTANCE 1.0f
#define OCTREE_DEPTH 4
#define NUM_DIMENSIONS 3
#define MAX_GLOBAL_REGISTRATION_RESTARTS 20

#define cross3v(dest, v1, v2) \
	(dest)[0] = (v1)[1] * (v2)[2] - (v1)[2] * (v2)[1];\
	(dest)[1] = (v1)[2] * (v2)[0] - (v1)[0] * (v2)[2];\
	(dest)[2] = (v1)[0] * (v2)[1] - (v1)[1] * (v2)[0]
#define dot3v(v1, v2) ((v1)[0]*(v2)[0] + (v1)[1]*(v2)[1] + (v1)[2]*(v2)[2])
#define sub3v(dest, v1, v2) \
	(dest)[0] = (v1)[0] - (v2)[0];\
	(dest)[1] = (v1)[1] - (v2)[1];\
	(dest)[2] = (v1)[2] - (v2)[2]

#define midPointVector3f(dest,min,max) (dest).x = (min).x + (((max).x - (min).x) / 2);\
	(dest).y = (min).y + (((max).y - (min).y) / 2);\
	(dest).z = (min).z + (((max).z - (min).z) / 2)

#define getQuadrant(point, mid) ((((point)[0] > (mid).x) ? 1 : 0 ) + \
	(((point)[2] > (mid).z) ? 2 : 0) + (((point)[1] > (mid).y) ? 4 : 0) )


#define SQ(x) ( (x) * (x) )

#define normalize3v(vector) {\
	float sqr = (float)sqrt( SQ((vector)[0]) + SQ((vector)[1]) + SQ((vector)[2]) );\
	(vector)[0] /= sqr;\
	(vector)[1] /= sqr;\
	(vector)[2] /= sqr;}


typedef struct triangulatedDataStruct_t{
	float* vertexArrayPntr;
	unsigned int* elementIndexPntr;
}triDataPack;

int vertexInRadiusData( float* vertex, float sqRadius, float* data, int dataSize ){
	float *dataEndPntr = data + dataSize;

	for( ; data < dataEndPntr; data += 3 ){
		if( (SQ(data[0] - vertex[0]) + SQ(data[1] - vertex[1]) + SQ(data[2] - vertex[2])) < sqRadius )
			return 1;
	}

	return 0;
}
static float distanceTringle(float** triangle, float* point);
static int rayIntersectsTriangle( float** triangle, float* ray, float* rayOrigin ){
	double edge1[3], edge2[3], tvec[3], pvec[3], qvec[3];
	double det, invDet;
	double u, v, t;

	normalize3v(ray);
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
	if( t > CLOSE_SQ_DISTANCE || t < -CLOSE_SQ_DISTANCE)
		return 0;
	return 1;
}

static int rayIntersectsDataOctTree( arrayListui *triList, float* vertexData, float* ray, float *rayOrigin){
	float* triangle[3];
	unsigned int *element = triList->data, *endPntr = triList->data + triList->lenght;
	for( ; element < endPntr; element += NUM_DIMENSIONS ){
		triangle[0] = vertexData + element[0] * NUM_DIMENSIONS;
		triangle[1] = vertexData + element[1] * NUM_DIMENSIONS;
		triangle[2] = vertexData + element[2] * NUM_DIMENSIONS;

		if(rayIntersectsTriangle(triangle, ray, rayOrigin))
			return 1;
	}

	return 0;
}

static int vertexInRadiusModelTri( unsigned int* element, int size, float* vertexData, float *point ){
	float* triangle[3];
	float distance;
	unsigned int *endPntr = element + size;
	for( ; element < endPntr; element += NUM_DIMENSIONS ){
		triangle[0] = vertexData + element[0] * NUM_DIMENSIONS;
		triangle[1] = vertexData + element[1] * NUM_DIMENSIONS;
		triangle[2] = vertexData + element[2] * NUM_DIMENSIONS;
		distance = distanceTringle(triangle, point);
		if( distance < CLOSE_SQ_DISTANCE && distance > -CLOSE_SQ_DISTANCE)
			return 1;
	}

	return 0;
}

static int recRayIntersectsOctTree( octTreeNode_p root, float* vertexData, float* ray, float* rayOrigin ){
	if( nodeInDistance(root, rayOrigin, MAX_SQ_DISTANCE) ){
		if( getNodesChildren(root) != NULL ){
			int i;
			for( i = 0; i < 8; i++ ){
				if(recRayIntersectsOctTree(getNodesChild(root, i),vertexData, ray, rayOrigin)){
					return 1;
				}
			}
		}else{
			arrayListui *nodesDataList;
			if( (nodesDataList = (arrayListui*)getNodesDataPntr(root)) != NULL ){
				//return vertexInRadiusModelTri(nodesDataList->data, nodesDataList->lenght, vertexData, rayOrigin);

				return rayIntersectsDataOctTree(nodesDataList, vertexData, ray, rayOrigin);
			}
			return 0;
		}
	}
	return 0;
}


/* creates a list of vertex indexes that are beeing represented by model and data triangulated mesh
* 
*/
static arrayListui* modelNormalShootToOctTree(	float* modelVertexArray, float* modelNormalArray, int modelSize,
	octTreeNode_p root, float* dataVertexArray){
		arrayListui *elementList;
		float *vertex;
		float *endPntr = modelVertexArray + modelSize;

		elementList = createArrayListui();

		for(	vertex = modelVertexArray; 
			vertex < endPntr; 
			vertex += NUM_DIMENSIONS, modelNormalArray += NUM_DIMENSIONS )
		{
			if(recRayIntersectsOctTree(root, dataVertexArray, modelNormalArray, vertex)){
				addToArrayListui(elementList, vertex - modelVertexArray);
			}
		}
		return elementList;
}

static void findMinMaxBoundsArray3f( float* vertexArray, int vertexCount, Vector3f *min, Vector3f *max ){
	float *endPntr = vertexArray + vertexCount;

	min->x = max->x = vertexArray[0];
	min->y = max->y = vertexArray[1];
	min->z = max->z = vertexArray[2];
	for( vertexArray += 3; vertexArray < endPntr; vertexArray += 3){
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
	/* Node is not a leaf node */
	minBound = getNodeMinBound(root);
	maxBound = getNodeMaxBound(root);

	midPointVector3f(mid, minBound, maxBound);

	pointQuad1 = getQuadrant(pointData + triangle[0] * 3, mid);
	pointQuad2 = getQuadrant(pointData + triangle[1] * 3, mid);
	pointQuad3 = getQuadrant(pointData + triangle[2] * 3, mid);

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
	unsigned int* elementIndex = ((triDataPack*)dataPack)->elementIndexPntr;
	unsigned int* endPntr;

	for(endPntr = elementIndex + size; elementIndex < endPntr; elementIndex += 3){
		if(!recAddTriangleData(root, vertexArray, elementIndex))
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
		tree = createOctTree(minBound, maxBound, (void*)(&dataPacked), elementCount, OCTREE_DEPTH, octTreeAddTriData, deleteDataFnc);
		return tree;
}

static arrayListui* radiusToModel(	float* modelVertexArray, int modelSize, 
									float* dataVertexArray, int dataSize){
		arrayListui *elementList;
		float *vertex;
		float *endPntr = modelVertexArray + modelSize;

		elementList = createArrayListui();

		for(	vertex = modelVertexArray; 
			vertex < endPntr; 
			vertex += NUM_DIMENSIONS )
		{
			if( vertexInRadiusData(vertex, 1.0f, dataVertexArray, dataSize) ){
				addToArrayListui(elementList, vertex - modelVertexArray);
			}
		}
		return elementList;
}

static arrayListui* triangleModel(	float* modelVertexArray, int modelSize,
									float* dataVertexArray,
									unsigned int* dataElementArray, int elementSize){
	arrayListui *elementList;
	float *vertex;
	float *endPntr = modelVertexArray + modelSize;
	float distance;

	elementList = createArrayListui();

	for(	vertex = modelVertexArray; 
			vertex < endPntr; 
			vertex += NUM_DIMENSIONS )
	{
		if( vertexInRadiusModelTri(dataElementArray, elementSize, dataVertexArray, vertex )){
			addToArrayListui(elementList, vertex - modelVertexArray);
		}else
			continue;
	}
	return elementList;
}

static arrayListui* getDuplicatedVertexList(	float* modelVertexArray, float* modelNormalArray, int modelSize,
												float* dataVertexArray, int dataSize, 
												unsigned int* dataElementArray, int dataElementCount )
{
		octTree_p modelTree;
		arrayListui *unionVertexIndexList;

		modelTree = createOctTreeTrangulated(dataVertexArray, dataSize, dataElementArray, dataElementCount);
		if( modelTree == NULL )
			return NULL;
		
		unionVertexIndexList = modelNormalShootToOctTree(modelVertexArray, modelNormalArray, modelSize, getRootOctTree(modelTree), dataVertexArray);
		if( unionVertexIndexList->lenght == 0 )
			return NULL;
		return unionVertexIndexList;
}

//#define region3() {s = 0; t = (e >= 0) ? 0 : ( (-e >= c) ? 1 : -e/c );  }
//#define region5() region3() 
//
//#define region2() { tmp0 = b + d;\
//					tmp1 = c + e;\
//					if ( tmp1 > tmp0 )\
//					{	numer = tmp1 - tmp0;\
//						denom = a - 2*b + c;\
//						s = (numer >= denom)? 1 : numer/denom;\
//						t = 1 - s;}\
//					else\
//					{	s = 0;\
//						t = ( tmp1 <= 0 ) ? 1: ( (e >= 0) ? 0 : -e/c); }\
//					}
//#define region4() region2()
//#define region6() region2()
//
//#define region1() { numer = c + e - b - d;\
//					if( numer <= 0 ){\
//						s = 0;\
//					}else{\
//						denom = a - 2*b + c;\
//						s = ( numer >= denom ) ? 1 : numer / denom;\
//					}\
//					t = 1 - s;}
//
//static float distanceTringle(float** triangle, float* point){
//	float *B, E0[3], E1[3], D[3];
//	float a, b, c, d, e, f;
//	float det, s, t, tmp0, tmp1;
//	//float sqrDistance;
//	float numer;
//	float denom;
//	float invDet;
//
//	// rewrite triangle in normal form
//	B = triangle[0];
//	sub3v(E0, triangle[1], B);
//	sub3v(E1, triangle[2], B);
//
//
//	sub3v(D, B, point);
//	a = dot3v(E0,E0);
//	b = dot3v(E0,E1);
//	c = dot3v(E1,E1);
//	d = dot3v(E0,D);
//	e = dot3v(E1,D);
//	f = dot3v(D,D);
//
//	det = a*c - b*b; // do we have to use abs here?
//	s = b*e - c*d;
//	t = b*d - a*e;
//
//	if ((s+t) <= det){
//		if (s < 0){
//			if (t < 0){
//				region4();
//			}else{
//				region3();
//			}
//		}else if (t < 0){
//			region5();
//		}else{
//			//% region 0
//			invDet = 1/det;
//			s = s*invDet;
//			t = t*invDet;
//		}
//
//	}else if (s < 0){
//		region2();
//	}else{
//		if (t < 0){ 
//			region6();
//		}else{
//			region1();
//		}
//	}
//
//	return a*s*s + 2*b*s*t + c*t*t + 2*d*s + 2*e*t + f;
//}



static float distanceTringle(float** triangle, float* point){
	float *B, E0[3], E1[3], D[3];
	float a, b, c, d, e, f;
	float det, s, t, temp1, temp2;
	float sqrDistance;
	float numer;
	float denom;
	float invDet;

	// rewrite triangle in normal form
	B = triangle[0];
	sub3v(E0, triangle[1], B);
	sub3v(E1, triangle[2], B);


	sub3v(D, B, point);
	a = dot3v(E0,E0);
	b = dot3v(E0,E1);
	c = dot3v(E1,E1);
	d = dot3v(E0,D);
	e = dot3v(E1,D);
	f = dot3v(D,D);

	det = a*c - b*b; // do we have to use abs here?
	s = b*e - c*d;
	t = b*d - a*e;

	if ((s+t) <= det){
		if (s < 0){
			if (t < 0){
				//region4
				if (d < 0){
					t = 0;
					if (-d >= a){
						s = 1;
						sqrDistance = a + 2*d + f;
					}else{
						s = -d/a;
						sqrDistance = d*s + f;
					}
				}else{
					s = 0;
					if (e >= 0){
						t = 0;
						sqrDistance = f;
					}else{
						if (-e >= c){
							t = 1;
							sqrDistance = c + 2*e + f;
						}else{
							t = -e/c;
							sqrDistance = e*t + f;
						}
					}
				}// %of region 4
			}else{
				//% region 3
				s = 0;
				if (e >= 0){
					t = 0;
					sqrDistance = f;
				}else{
					if (-e >= c){
						t = 1;
						sqrDistance = c + 2*e +f;
					}else{
						t = -e/c;
						sqrDistance = e*t + f;
					}
				}
			}//end region 3
		}else{
			if (t < 0){
				//% region 5
				t = 0;
				if (d >= 0){
					s = 0;
					sqrDistance = f;
				}else{
					if (-d >= a){
						s = 1;
						sqrDistance = a + 2*d + f;
					}else{
						s = -d/a;
						sqrDistance = d*s + f;
					}
				}
			}else{
				//% region 0
				invDet = 1/det;
				s = s*invDet;
				t = t*invDet;
				sqrDistance = s*(a*s + b*t + 2*d) + t*(b*s + c*t + 2*e) + f;
			}
		}
	}else{
		if (s < 0){
			//% region 2
			temp1 = b + d;
			temp2 = c + e;
			if (temp2 > temp1){// % minimum on edge s+t=1
				numer = temp2 - temp1;
				denom = a - 2*b + c;
				if (numer >= denom){
					s = 1;
					t = 0;
					sqrDistance = a + 2*d + f;
				}else{
					s = numer/denom;
					t = 1-s;
					sqrDistance = s*(a*s + b*t + 2*d) + t*(b*s + c*t + 2*e) + f;
				}
			}else{//          % minimum on edge s=0
				s = 0;
				if (temp2 <= 0){
					t = 1;
					sqrDistance = c + 2*e + f;
				}else{
					if (e >= 0){
						t = 0;
						sqrDistance = f;
					}else{
						t = -e/c;
						sqrDistance = e*t + f;
					}
				}
			}// %of region 2
		}else{
			if (t < 0){//   %region6 
				temp1 = b + e;
				temp2 = a + d;
				if (temp2 > temp1){
					numer = temp2 - temp1;
					denom = a-2*b+c;
					if (numer >= denom){
						t = 1;
						s = 0;
						sqrDistance = c + 2*e + f;
					}else{
						t = numer/denom;
						s = 1 - t;
						sqrDistance = s*(a*s + b*t + 2*d) + t*(b*s + c*t + 2*e) + f;
					}
				}else{  
					t = 0;
					if (temp1 <= 0){
						s = 1;
						sqrDistance = a + 2*d + f;
					}else{
						if (d >= 0){
							s = 0;
							sqrDistance = f;
						}else{
							s = -d/a;
							sqrDistance = d*s + f;
						}
					}
				}
				//%end region 6
			}else{//     % region 1
				numer = c + e - b - d;
				if (numer <= 0){
					s = 0;
					t = 1;
					sqrDistance = c + 2*e + f;
				}else{
					denom = a - 2*b + c;
					if (numer >= denom){
						s = 1;
						t = 0;
						sqrDistance = a + 2*d + f;
					}else{
						s = numer/denom;
						t = 1-s;
						sqrDistance = s*(a*s + b*t + 2*d) + t*(b*s + c*t + 2*e) + f;
					}
				}// %of region 1
			}
		}
	}
	return sqrDistance;
}

static void nonHomogen3DVctrMtrxMultiply(float *matrix, float *vector){
	float temp[4], homegenVector[4], sum;
	int row, col;
	homegenVector[0] = vector[0];
	homegenVector[1] = vector[1];
	homegenVector[2] = vector[2];
	homegenVector[3] = 1;
	for(col=0; col<4; col++){
		sum = 0;
		for(row=0; row<4; row++){
			sum += matrix[col + row * 4] * homegenVector[row];
		}
		temp[col] = sum;
	}
	memcpy(vector, temp, sizeof(float)*3);
}

static void registerData( float* dataVertexArray, int dataSize, float* registationMatrix ){
	float *endPntr;
	for (endPntr = dataVertexArray + dataSize; dataVertexArray < endPntr; dataVertexArray += NUM_DIMENSIONS){
		nonHomogen3DVctrMtrxMultiply(registationMatrix, dataVertexArray);
	}
}

static int alignedMeshes(	float* modelVertexArray, int modelSize,
							float* dataVertexArray, int dataSize )
{
	Vector3f min, max;
	float *registrationMatrix;

	findMinMaxBoundsArray3f(modelVertexArray, modelSize, &min, &max);
	registrationMatrix = globalICPRegistration(modelVertexArray, modelSize, min, max, dataVertexArray, dataSize, MAX_GLOBAL_REGISTRATION_RESTARTS);
	registerData( dataVertexArray, dataSize, registrationMatrix );
	free(registrationMatrix);
}

