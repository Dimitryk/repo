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


/* A node struct 
 * Each node in the tree has it diamentions set by bounds
 * given in minCoordinates and maxCoordinates Vectors
 */
typedef struct octTreeNode_t{
	Vector3f minCoordinates, maxCoordinates;
	struct octTreeNode_t *children;
	arrayListf *pointDataList;
}octTreeNode;

/* Grows a tree from given root node of a given depth
 * if depth is 0 all data that has coordinates with in 
 * min and max bound will be attached to the root node
 */
int createOctTree(octTreeNode *root, Vector3f minBound, Vector3f maxBound, float *pointData, int arraySize, int depth);

/* Calculates the closest point in the oct tree to the given point
 * Returns a pointer to a 3x float point = (x,y,z)
 */
float* closestPntToPnt(octTreeNode *root, float *point);
/* Frees all the allocated memmory for the root node and all sub nodes
 * including Data lists
 */
void deleteOctTree(octTreeNode *root);


#endif