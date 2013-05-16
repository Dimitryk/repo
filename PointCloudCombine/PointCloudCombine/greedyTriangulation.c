#include <stdlib.h>
#include <string.h>
#include "greedyTriangulation.h"
#include "kdTree.h"


#define EDGE_SLOT_SIZE 1024
#define MAX_NEAREST_NEIGHBOURS 7000
#define TRI_DIRECTION_CW 0

#define SQ(x) ( (x) * (x) )
#define distance(a,b) (SQ((a)[0]-(b)[0]) + SQ((a)[1]-(b)[1]) + SQ((a)[2]-(b)[2]))
#define sub2v(dest, v1, v2) \
	(dest)[0] = (v1)[0] - (v2)[0];\
	(dest)[1] = (v1)[1] - (v2)[1]
#define cross2v(v1, v2) ((v1)[0]*(v2)[1] - (v1)[1]*(v2)[0])

#define projectPntPlane(dest, point, planeNormal, planePnt) {\
	double t =  ((planeNormal)[0]*(planePnt)[0] - (planeNormal)[0]*(point)[0] + (planeNormal)[1]*(planePnt)[1] - (planeNormal)[1]*(point)[1]\
				+ (planeNormal)[2]*(planePnt)[2] - (planeNormal)[2]*(point)[2])\
				/ (SQ((planeNormal)[0]) + SQ((planeNormal)[1]) + SQ((planeNormal)[2]));\
	(dest)[0] = (point)[0] + t*(planeNormal)[0];\
	(dest)[1] = (point)[1] + t*(planeNormal)[1];\
	(dest)[2] = (point)[2] + t*(planeNormal)[2];}

typedef struct edge_t{
	struct point_t *a, *b;
}edge;

typedef struct point_t{
	float *coords;
	int nrEdges;
	int maxEdges;
	struct edge_t **edges;
}point;

typedef struct pointArray_t{
	point *points;
	int lenght;
}pointArray;

typedef struct edgeSlot_t{
	struct edge_t edges[EDGE_SLOT_SIZE];
}edgeSlot;

static struct {
	struct edgeSlot_t **slots;
	int nrSlots;
	int size;
	int lenght;
}edgeStack;

/* The line interseption 
 * L1 = p1 + s (p2 - p1) & L2 = p3 + t ( p4 - p3 )
 * p1 + s * v1 = p3 + t * v2
 * s * v1 = (p3 - p1) + t * v2
 * s * (v1 x v2) = (p3 - p1) x v2 + t * v2 x v2
 * s = ( (p3 - p4) x v2 )/ v1 x v2
 */
static int edgesIntersept( float* p1, float* p2, float* p3, float* p4 ){
	double v1[2], v2[2], vp1p3[2];
	double den, s, t; 

	sub2v(v1, p2, p1);
	sub2v(v2, p4, p3);
	sub2v(vp1p3, p3, p1);

	den = cross2v(v1, v2);
	if( den == 0 )
		return 0;
	s = cross2v(vp1p3, v2) / den;
	if( s < 0 || s > 1)
		return 0;
	if( v2[0] != 0 )
		t = (p1[0] - p3[0] + s*v1[0])/v2[0];
	else
		t = (p1[1] - p3[1] + s*v1[1])/v2[1];
	if( t < 0 || t > 1 )
		return 0;
	return 1;
}

static int growEdgeStack(void){
	edgeSlot *newStack = (edgeSlot*)malloc(sizeof(edgeSlot));
	if( newStack == NULL )
		return 0;
	edgeStack.slots = (edgeSlot**)realloc(edgeStack.slots, sizeof(edgeSlot*) * (edgeStack.nrSlots + 1));
	if( edgeStack.slots == NULL )
		return 0;
	edgeStack.slots[edgeStack.nrSlots++] = newStack;
	edgeStack.size += EDGE_SLOT_SIZE;
	return 1;
}

static int growPointEdgesArray( point* pnt ){
	pnt->edges = (edge**)realloc(pnt->edges, sizeof(edge*) * pnt->nrEdges * 2);
	if(pnt->edges == NULL )
		return 0;
	pnt->maxEdges *= 2;
}

static int pointAddEdge( point* pnt, edge* newEdge ){
	if( pnt->nrEdges == pnt->maxEdges )
		if( !growPointEdgesArray(pnt) )
			return 0;
	pnt->edges[pnt->nrEdges++] = newEdge;
}

static edge* createEdge( point* a, point* b){
	edge *newEdge;
	int slotIndex, length;
	if( edgeStack.lenght == edgeStack.size )
		if(!growEdgeStack())
			return NULL;
	
	for( length = edgeStack.lenght; length >= EDGE_SLOT_SIZE; length -= EDGE_SLOT_SIZE );
	slotIndex = edgeStack.lenght / EDGE_SLOT_SIZE;
	newEdge = edgeStack.slots[slotIndex]->edges + length;
	newEdge->a = a;
	newEdge->b = b;
	if(!pointAddEdge( a, newEdge ))
		return NULL;
	if(!pointAddEdge( b, newEdge ))
		return NULL;
	edgeStack.lenght++;
	return newEdge;
}

void deletePointArray(point* points, int size){
	point *index = points, *endPntr = points + size;
	for( ; index < endPntr; index++ ){
		free(index->edges);
	}
	free(points);
}

void cleanEdgeStack(void){
	int i;
	edgeSlot **temp = edgeStack.slots;
	for( i = 0; i < edgeStack.nrSlots; i++){
		free(temp[i]);
	}
	
	edgeStack.slots == NULL;

	free(temp);
}

static int copyPointCoordinates( point* pntArray, float* modelVertexArray, int modelSize, float* dataVertexArray, int dataSize){
	point *pointIndex, *endPntr;

	for(	pointIndex = pntArray, endPntr = pntArray + modelSize;
			pointIndex < endPntr;
			pointIndex++, modelVertexArray += 3 ){
				pointIndex->coords = modelVertexArray;
				pointIndex->nrEdges = 0;
				pointIndex->maxEdges = 16;
				pointIndex->edges = (edge**)malloc(sizeof(edge*) * 16 );
				if(pointIndex->edges == NULL){
					deletePointArray(pntArray, pointIndex - pntArray);
					return 0;
				}
	}
	for(	endPntr = pntArray + modelSize + dataSize;
			pointIndex < endPntr;
			pointIndex++, dataVertexArray += 3 ){
				pointIndex->coords = dataVertexArray;
				pointIndex->nrEdges = 0;
				pointIndex->maxEdges = 16;
				pointIndex->edges = (edge**)malloc(sizeof(edge*) * 16 );
				if(pointIndex->edges == NULL){
					deletePointArray(pntArray, pointIndex - pntArray);
					return 0;
				}
	}
	return 1;
}

static int copyPointEdges(	point* pntArray, int modelSize,
							unsigned int* modelElement, int modelElementCount,
							unsigned int* dataElement, int dataElementCount ){
	unsigned int *endPntr;
	for( endPntr = modelElement + modelElementCount; modelElement < endPntr; modelElement += 3 ) {
		if( !createEdge(pntArray + modelElement[0], pntArray + modelElement[1] ) )
			return 0;
		if( !createEdge(pntArray + modelElement[1], pntArray + modelElement[2]) )
			return 0;
		if( !createEdge(pntArray + modelElement[2], pntArray + modelElement[0]) )
			return 0;
	}
	for( endPntr = dataElement + dataElementCount; dataElement < endPntr; dataElement += 3 ) {
		if( !createEdge(pntArray + modelSize + dataElement[0], pntArray + modelSize + dataElement[1]) )
			return 0;
		if( !createEdge(pntArray + modelSize + dataElement[1], pntArray + modelSize + dataElement[2]) )
			return 0;
		if( !createEdge(pntArray + modelSize + dataElement[2], pntArray + modelSize + dataElement[0]) )
			return 0;
	}
	return 1;
}

static pointArray* createPointArray(float* modelVertexArray, int modelSize, unsigned int* modelElementArray, int modelElementCount,
									float* dataVertexArray, int dataSize, unsigned int* dataElementArray, int dataElementCount ){
	pointArray *pntArray;

	modelSize /= 3;
	dataSize /= 3;

	pntArray = (pointArray*)malloc(sizeof(pointArray));
	if(pntArray == NULL)
		return NULL;
	pntArray->lenght = modelSize + dataSize;
	pntArray->points = (point*)malloc(sizeof(point) * pntArray->lenght);
	if(pntArray->points == NULL){
		free(pntArray);
		return NULL;
	}

	if(!copyPointCoordinates( pntArray->points, modelVertexArray, modelSize, dataVertexArray, dataSize )){
		free(pntArray);
		return NULL;
	}

	if(!copyPointEdges( pntArray->points, modelSize, modelElementArray, modelElementCount, dataElementArray, dataElementCount )){
		deletePointArray(pntArray->points, pntArray->lenght);
		return NULL;
	}
	return pntArray;
}

#define cross3v(dest, v1, v2) \
	(dest)[0] = (v1)[1] * (v2)[2] - (v1)[2] * (v2)[1];\
	(dest)[1] = (v1)[2] * (v2)[0] - (v1)[0] * (v2)[2];\
	(dest)[2] = (v1)[0] * (v2)[1] - (v1)[1] * (v2)[0]

#define length3v(vector) (SQ((vector)[0]) + SQ((vector)[1]) + SQ((vector)[2]))

#define sub3v(dest, v1, v2) \
	(dest)[0] = (v1)[0] - (v2)[0];\
	(dest)[1] = (v1)[1] - (v2)[1];\
	(dest)[2] = (v1)[2] - (v2)[2]

static int addTriagleList( arrayListui* list, point* points, edge** triangle ){
	double ab[3], bc[3], cross[3];
	unsigned int tri[3];
	point *A, *B, *C;
	double sin;
	
	B = triangle[0]->a;
	C = triangle[0]->b;
	A = (triangle[1]->a == B || triangle[1]->a == C) ? triangle[1]->b : triangle[1]->a;

	sub3v(ab, B->coords, A->coords);
	sub3v(bc, C->coords, B->coords);

	cross3v(cross, ab, bc);

	sin = length3v(cross) / (length3v(ab) * length3v(bc));
	if(TRI_DIRECTION_CW){
		if(sin > 0){
			tri[0] = C - points;
			tri[1] = B - points;
			tri[2] = A - points;
			return addToArrayListuiv(list, tri, 3);
		}
		tri[0] = A - points;
		tri[1] = B - points;
		tri[2] = C - points;
		return addToArrayListuiv(list, tri, 3);
	}
	if(sin > 0){
		tri[0] = A - points;
		tri[1] = B - points;
		tri[2] = C - points;
		return addToArrayListuiv(list, tri, 3);
	}
	tri[0] = C - points;
	tri[1] = B - points;
	tri[2] = A - points;
	return addToArrayListuiv(list, tri, 3);
}

static int edgeValid( point* current, point* toPnt, float* normal, edge** triangles, int* nrTriangles ){
	float p2[3], p3[3], p4[3];
	float *p1 = current->coords;
	edge **currentOldEdge, **oldEdgeEdge, **endPntrCurrent, **endPntrOld;
	point *connectedOldPoint, *oldPointsConnectedPoint;

	projectPntPlane(p2, toPnt->coords, normal, p1);
	*nrTriangles = 0;

	for(currentOldEdge = current->edges, endPntrCurrent = current->edges + current->nrEdges;
		currentOldEdge < endPntrCurrent; 
		currentOldEdge++ ){
			connectedOldPoint = (*currentOldEdge)->a;
			if(current == connectedOldPoint)
				connectedOldPoint = (*currentOldEdge)->b;
			if(connectedOldPoint == toPnt)
				return 0;//edge allready exist
			projectPntPlane(p3, connectedOldPoint->coords, normal, p1);
			for(	oldEdgeEdge = connectedOldPoint->edges, 
					endPntrOld = connectedOldPoint->edges + connectedOldPoint->nrEdges;
					oldEdgeEdge < endPntrOld;
					oldEdgeEdge++ ){
					oldPointsConnectedPoint = (*oldEdgeEdge)->a;
					if( oldPointsConnectedPoint == connectedOldPoint )
						oldPointsConnectedPoint = (*oldEdgeEdge)->b;
					if( oldPointsConnectedPoint == toPnt ){
						triangles[*nrTriangles * 2 + 0] = *oldEdgeEdge;//triangle match
						triangles[*nrTriangles * 2 + 1] = *currentOldEdge;
						(*nrTriangles)++;
					}
					projectPntPlane(p4, oldPointsConnectedPoint->coords, normal, p1);
					if( edgesIntersept(p1, p2, p3, p4) )
						return 0;
			}
	}
	return 1;
}

#define GET_WEIGHED_NORMAL(dest) {\
	float w; unsigned int tpnt; float *normal;int validNeighbours = 0;\
	(dest)[0] = 0;(dest)[1] = 0;(dest)[2] = 0;\
	for( i = 0; i < MAX_NEAREST_NEIGHBOURS; i++){\
		if(kNNTopoint[i] != NULL){\
			validNeighbours++;\
			w = distance(point,kNNTopoint[i]);\
			w = maxSQDistance/w;\
			if((tpnt = mergedVertexArray - kNNTopoint[i]) > (unsigned int)modelSize){\
				normal = dataNormals + tpnt - modelSize;\
			}else\
				normal = modelNormals + tpnt;\
			(dest)[0] += normal[0] * w;\
			(dest)[1] += normal[1] * w;\
			(dest)[2] += normal[2] * w;\
		}\
	}\
	(dest)[0] /= validNeighbours;\
	(dest)[0] /= validNeighbours;\
	(dest)[0] /= validNeighbours;\
	}

arrayListui* greedyTriangulation(	geometryMesh* model, geometryMesh* data,
									arrayListui* pointsToTrangulate, float maxSQDistance)
{
	/* Unpack meshes */
	double w; unsigned int tpnt; float *normal;int validNeighbours = 0;
	float *modelVertexArray = model->vertexArray, *modelNormals = model->normalsArray;
	float *dataVertexArray = data->vertexArray, *dataNormals = data->normalsArray;
	int modelSize = model->vertexCount, modelElementCount = model->elementCount;
	int dataSize = data->vertexCount, dataElementCount = data->elementCount;
	unsigned int *modelElementArray = model->elementArray, *dataElementArray = data->elementArray;

	arrayListui *triangleList;
	pointArray *pntArray;
	unsigned int pointIndex, nearestPntIndex;
	int i, nrTriangles;
	edge *isTriangle[200], *newEdge;
	float *mergedVertexArray, *kNNTopoint[MAX_NEAREST_NEIGHBOURS], *point, **nearestPoint, **kNNEndpntr;
	float weighedNormal[3];
	kdTree_p tree;

	/* Create a map over points and edges */
	pntArray = createPointArray( modelVertexArray, modelSize, modelElementArray, modelElementCount,
		dataVertexArray,  dataSize, dataElementArray, dataElementCount);
	mergedVertexArray = (float*)malloc(sizeof(float) * (modelSize + dataSize));
	memcpy(mergedVertexArray, modelVertexArray, sizeof(float) * modelSize);
	memcpy(mergedVertexArray + modelSize, dataVertexArray, sizeof(float) * dataSize);

	tree = createKD_Tree(mergedVertexArray, modelSize + dataSize);

	kNNEndpntr = kNNTopoint + MAX_NEAREST_NEIGHBOURS;
	triangleList = createArrayListui();

	for( pointsToTrangulate->index = 0; pointsToTrangulate->index < pointsToTrangulate->lenght; pointsToTrangulate->index++ ){
		pointIndex = pointsToTrangulate->data[pointsToTrangulate->index];
		point = mergedVertexArray + pointIndex;
		kdTree_KNN(tree, kNNTopoint, point, MAX_NEAREST_NEIGHBOURS, maxSQDistance); 


		(weighedNormal)[0] = 0;(weighedNormal)[1] = 0;(weighedNormal)[2] = 0;
		for( i = 0; i < MAX_NEAREST_NEIGHBOURS; i++){
			if(kNNTopoint[i] != NULL){
				validNeighbours++;
				if( (w = distance(point,kNNTopoint[i])) == 0)
					continue;
				w = maxSQDistance/w;
				if((tpnt = kNNTopoint[i] - mergedVertexArray) > (unsigned int)modelSize){
					normal = dataNormals + tpnt - modelSize;
				}else
					normal = modelNormals + tpnt;
				(weighedNormal)[0] += (float)(normal[0] * w);
				(weighedNormal)[1] += (float)(normal[1] * w);
				(weighedNormal)[2] += (float)(normal[2] * w);
			}
		}
		(weighedNormal)[0] /= validNeighbours;
		(weighedNormal)[0] /= validNeighbours;
		(weighedNormal)[0] /= validNeighbours;

		for( nearestPoint = kNNTopoint; nearestPoint < kNNEndpntr; nearestPoint++){
			if( (*nearestPoint) == NULL)
				continue;
			/* Caculate with nearestPoint index point (start-pointer - point-pointer) */
			nearestPntIndex = ((*nearestPoint) - mergedVertexArray)/3;
			if(edgeValid(pntArray->points + pointIndex, pntArray->points + nearestPntIndex, weighedNormal, isTriangle, &nrTriangles) ){
				/* Add an edge to your map */
				if( (newEdge = createEdge(pntArray->points + pointIndex, pntArray->points + nearestPntIndex)) == NULL ){
					deleteKD_Tree(tree);//out Of memmory;
					free(mergedVertexArray);
					deletePointArray(pntArray->points, pntArray->lenght);
					free(pntArray);
					deleteArrayListui(triangleList);
					return NULL;
				}
				/* Add new point to try to trangulate */
				addToArrayListui(pointsToTrangulate, nearestPntIndex);
				for( i = 0; i < nrTriangles; i++ ){
					if(!addTriagleList(triangleList, pntArray->points, isTriangle + i)){
						deleteKD_Tree(tree);//out Of memmory;
						free(mergedVertexArray);
						deletePointArray(pntArray->points, pntArray->lenght);
						free(pntArray);
						deleteArrayListui(triangleList);
						return NULL;
					}
				}
			}
		}
	}
	deleteKD_Tree(tree);
	free(mergedVertexArray);
	deletePointArray(pntArray->points, pntArray->lenght);
	free(pntArray);
	cleanEdgeStack();
	return triangleList;
}

geometryMesh* combimeMeshes(geometryMesh* model, geometryMesh* data, arrayListui* cutEdgePoints, float maxSQDistance){

	arrayListui *newTriangles;
	geometryMesh *newMesh;
	unsigned int *dataElementIndex, *endDataElementPntr, *newMeshElement;
	unsigned int dataPointOfset;
	/* Create new triangles between meshes */
	newTriangles = greedyTriangulation(model, data, cutEdgePoints, maxSQDistance);
	if(newTriangles == NULL)
		return NULL;
	
	newMesh = (geometryMesh*)malloc(sizeof(geometryMesh));
	
	/* Allocate place for new VertexArray */
	newMesh->vertexCount = model->vertexCount + data->vertexCount;
	newMesh->vertexArray = (float*)malloc(sizeof(float) * newMesh->vertexCount);
	/* Copy vertex data */
	memcpy(newMesh->vertexArray, model->vertexArray, sizeof(float) * model->vertexCount);
	memcpy(newMesh->vertexArray + model->vertexCount, data->vertexArray, sizeof(float) * data->vertexCount);
	/* Allocate memmory for element array */
	newMesh->elementCount = model->elementCount + data->elementCount + newTriangles->lenght;
	newMesh->elementArray = (unsigned int*)malloc(sizeof(unsigned int) * newMesh->elementCount);
	/* copy triangle information */
	memcpy(newMesh->elementArray, model->elementArray, sizeof(unsigned int) * model->elementCount);
	/* copy triangle information from data set, and compensate for index increace from merging */
	dataPointOfset = (unsigned int)(model->vertexCount/3);
	dataElementIndex = data->elementArray;
	endDataElementPntr = data->elementArray + data->elementCount;
	newMeshElement = newMesh->elementArray + model->elementCount;
	for( ; dataElementIndex < endDataElementPntr; dataElementIndex++, newMeshElement++ ){
			*newMeshElement = (*dataElementIndex) + dataPointOfset;
	}
	/* Copy New triangles to Element Array */
	memcpy(newMesh->elementArray + model->elementCount + data->elementCount, newTriangles->data, sizeof(unsigned int) * newTriangles->lenght);
	
	/* Copy Normals to combined mesh NB! normals should be recomputed */
	newMesh->normalsArray = (float*)malloc(sizeof(float) * newMesh->vertexCount);
	memcpy(newMesh->normalsArray, model->normalsArray, sizeof(float) * model->vertexCount);
	memcpy(newMesh->normalsArray + model->vertexCount, data->normalsArray, sizeof(float) * data->vertexCount);

	deleteArrayListui(newTriangles);
	return newMesh;
}