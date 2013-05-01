/*
 * A header for representation of 3D point data in a form of octav tree
 * each point is represented as 3 floating points variable 3dPoint = (x,y,z)
 * each octTreeNode that are not leaf nodes contain array og eight children 
 * children in the form octTreeNode* nodes pointDataList will be set to NULL
 * if a octTreeNode is a leaf node its children array will be set to NULL
 * and it will contain pointDataList in type of Linked list implementation
 */
#ifndef __octTree_h_
#define __octTree_h_
#include "vector.h"
#include "arrayList.h"

typedef struct octTree_t* octTree_p;
typedef struct octTreeNode_t* octTreeNode_p;

/* Grows a tree from given root node of a given depth
 * if depth is 0 all data that has coordinates with in 
 * min and max bound will be attached to the root node
 */
octTree_p createOctTree(	Vector3f minBound, Vector3f maxBound, 
							void* pointData, int arraySize, 
							int depth,
							int (*addData_cb)(octTreeNode_p, void*, int),
							void (*deleteData_cb)(void*));

octTreeNode_p getRootOctTree(octTree_p);

Vector3f getNodeMinBound(octTreeNode_p);
Vector3f getNodeMaxBound(octTreeNode_p);

octTreeNode_p getNodesChildren(octTreeNode_p);

octTreeNode_p getNodesChild(octTreeNode_p, int);

void* getNodesDataPntr(octTreeNode_p);

void setNodesDataPntr(octTreeNode_p, void*);

int addDataOctTree(octTree_p tree, void* data, int size);

/* Calculates the closest point in the oct tree to the given point
 * Returns a pointer to a 3x float point = (x,y,z) 
 * and saces eculiadian distance to closest point in distance
 */
//float* closestPntToPnt(octTree_p* tree, float* point, float* distance);

int rayOctreeIntersept(octTreeNode_p root, float* ray, float* origin, float maxSqDistance);

int nodeInDistance(octTreeNode_p root, float* point, float maxSQDistance);

/* Frees all the allocated memmory for the root node and all sub nodes
 * including Data lists by user providen delete-function in creation prosses
 */
void deleteOctTree(octTree_p tree);


#endif