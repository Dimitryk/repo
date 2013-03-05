#include <stdlib.h>
#include <math.h>
#include "octTree.h"

#define octTreeXmidPoint(node) (node->minCoordinates.x + (node->maxCoordinates.x - node->minCoordinates.x) / 2)
#define octTreeYmidPoint(node) (node->minCoordinates.y + (node->maxCoordinates.y - node->minCoordinates.y) / 2)
#define octTreeZmidPoint(node) (node->minCoordinates.z + (node->maxCoordinates.z - node->minCoordinates.z) / 2)

#define getQuadrant(root, point) (((point[0] > octTreeXmidPoint(root)) ? 1 : 0 )+\
	((point[1] > octTreeZmidPoint(root)) ? 2 : 0) + ((point[2] > octTreeYmidPoint(root)) ? 4 : 0))


#define distance(a,b) (max(a,b) - min(a,b))
#define euclideanDistance(a,b,c) (sqrt(a*a + b*b + c*c))
#define nodePntDistance(node,point) (euclideanDistance(\
	min(distance(node->minCoordinates.x,point[0]),distance(node->maxCoordinates.x,point[0])),\
	min(distance(node->minCoordinates.y,point[1]),distance(node->maxCoordinates.y,point[1])),\
	min(distance(node->minCoordinates.z,point[2]),distance(node->maxCoordinates.z,point[2]))))
#define pntToPntDistance(a,b) (euclideanDistance(distance(a[0],b[0]),distance(a[1],b[1]),distance(a[2],b[2])))

static void setBoundsFirstNode(octTreeNode *current){
	current->children[0].minCoordinates.x = current->minCoordinates.x;
	current->children[0].minCoordinates.y = current->minCoordinates.y;
	current->children[0].minCoordinates.z = current->minCoordinates.z;

	current->children[0].maxCoordinates.x = octTreeXmidPoint(current);
	current->children[0].maxCoordinates.y = octTreeYmidPoint(current);
	current->children[0].maxCoordinates.z = octTreeZmidPoint(current);
}
static void setBoundsSecondNode(octTreeNode *current){
	current->children[1].minCoordinates.x = octTreeXmidPoint(current);
	current->children[1].minCoordinates.y = current->minCoordinates.y;
	current->children[1].minCoordinates.z = current->minCoordinates.z;

	current->children[1].maxCoordinates.x = current->maxCoordinates.x;
	current->children[1].maxCoordinates.y = octTreeYmidPoint(current);
	current->children[1].maxCoordinates.z = octTreeZmidPoint(current);
}
static void setBoundsThirdNode(octTreeNode *current){
	current->children[2].minCoordinates.x = current->minCoordinates.x;
	current->children[2].minCoordinates.y = current->minCoordinates.y;
	current->children[2].minCoordinates.z = octTreeZmidPoint(current);

	current->children[2].maxCoordinates.x = octTreeXmidPoint(current);
	current->children[2].maxCoordinates.y = octTreeYmidPoint(current);
	current->children[2].maxCoordinates.z = current->maxCoordinates.z;
}
static void setBoundsFourthNode(octTreeNode *current){
	current->children[3].minCoordinates.x = octTreeXmidPoint(current);
	current->children[3].minCoordinates.y = current->minCoordinates.y;
	current->children[3].minCoordinates.z = octTreeZmidPoint(current);

	current->children[3].maxCoordinates.x = current->maxCoordinates.x;
	current->children[3].maxCoordinates.y = octTreeYmidPoint(current);
	current->children[3].maxCoordinates.z = current->maxCoordinates.z;
}
static void setBoundsFifthNode(octTreeNode *current){
	current->children[4].minCoordinates.x = current->minCoordinates.x;
	current->children[4].minCoordinates.y = octTreeYmidPoint(current);
	current->children[4].minCoordinates.z = current->minCoordinates.z;

	current->children[4].maxCoordinates.x = octTreeXmidPoint(current);
	current->children[4].maxCoordinates.y = current->maxCoordinates.y;
	current->children[4].maxCoordinates.z = octTreeZmidPoint(current);
}
static void setBoundsSixthNode(octTreeNode *current){
	current->children[5].minCoordinates.x = octTreeXmidPoint(current);
	current->children[5].minCoordinates.y = octTreeYmidPoint(current);
	current->children[5].minCoordinates.z = current->minCoordinates.z;

	current->children[5].maxCoordinates.x = current->maxCoordinates.x;
	current->children[5].maxCoordinates.y = current->maxCoordinates.y;
	current->children[5].maxCoordinates.z = octTreeZmidPoint(current);
}
static void setBoundsSeventhNode(octTreeNode *current){
	current->children[6].minCoordinates.x = current->minCoordinates.x;
	current->children[6].minCoordinates.y = octTreeYmidPoint(current);
	current->children[6].minCoordinates.z = octTreeZmidPoint(current);

	current->children[6].maxCoordinates.x = octTreeXmidPoint(current);
	current->children[6].maxCoordinates.y = current->maxCoordinates.y;
	current->children[6].maxCoordinates.z = current->maxCoordinates.z;
}
static void setBoundsEightNode(octTreeNode *current){
	current->children[7].minCoordinates.x = octTreeXmidPoint(current);
	current->children[7].minCoordinates.y = octTreeYmidPoint(current);
	current->children[7].minCoordinates.z = octTreeZmidPoint(current);

	current->children[7].maxCoordinates.x = current->maxCoordinates.x;
	current->children[7].maxCoordinates.y = current->maxCoordinates.y;
	current->children[7].maxCoordinates.z = current->maxCoordinates.z;
}
static int setNodeChildren(octTreeNode *current, int depth){

	if(depth){
		current->children = (octTreeNode *)malloc( 8 * sizeof(octTreeNode));
		if(current->children==NULL)
			return 0;

		setBoundsFirstNode(current);
		if(!setNodeChildren(current->children + 0, depth - 1))
			return 0;

		setBoundsSecondNode(current);
		if(!setNodeChildren(current->children + 1, depth - 1))
			return 0;

		setBoundsThirdNode(current);
		if(!setNodeChildren(current->children + 2, depth - 1))
			return 0;

		setBoundsFourthNode(current);
		if(!setNodeChildren(current->children + 3, depth - 1))
			return 0;

		setBoundsFifthNode(current);
		if(!setNodeChildren(current->children + 4, depth - 1))
			return 0;

		setBoundsSixthNode(current);
		if(!setNodeChildren(current->children + 5, depth - 1))
			return 0;

		setBoundsSeventhNode(current);
		if(!setNodeChildren(current->children + 6, depth - 1))
			return 0;

		setBoundsEightNode(current);
		if(!setNodeChildren(current->children + 7, depth - 1))
			return 0;

		return 1;
	}
	current->children = NULL;
	current->pointDataList = NULL;
	return 1;
}


int octTreeAddPoint(octTreeNode *root, float *data){
	int position = 0;
	if(root->children == NULL){
		if(root->pointDataList == NULL){
			root->pointDataList = createArrayListf();
			return addToArrayListfv(root->pointDataList, data, 3);
		}else
			return addToArrayListfv(root->pointDataList, data, 3);
	}
	position += (data[0] > octTreeXmidPoint(root)) ? 1 : 0 ;
	position += (data[1] > octTreeZmidPoint(root)) ? 2 : 0 ;
	position += (data[2] > octTreeYmidPoint(root)) ? 4 : 0 ;
	return octTreeAddPoint(root->children + position, data);
}

int createOctTree(octTreeNode *root, Vector3f minBound, Vector3f maxBound, float *pointData, int arraySize, int depth){
	root->minCoordinates = minBound;
	root->maxCoordinates = maxBound;
	if(!setNodeChildren(root, depth))
		return 0;
	for(arraySize-=3; arraySize>=0; arraySize-=3){
		{
			if(!octTreeAddPoint(root, pointData + arraySize))
				return 0;
		}
	}
	return 1;
}

/* Returns NULL if no closer point is found
* returns a float pointer if a closer match is found
*/
static float* clsPntInList(float* data, int size, float* point, float* distance){
	float* endPoint = data + size;
	float* clsPnt = NULL;
	float newDist;
	for( ; data < endPoint; data +=3 ){
		newDist = (float)pntToPntDistance(data,point);
		if(newDist < *distance){
			*distance = newDist;
			clsPnt = data;
		}
	}
	return clsPnt;
}

static float* recFindClsPntToPnt(octTreeNode *root, float *point, float *distance){
	int i, guessQadrant;
	float *closestPoint = NULL;
	float *clsPointReturn;
	if(root->children != NULL){
		//we we estimate the best quadrant first
		guessQadrant = getQuadrant(root, point);
		if((clsPointReturn = recFindClsPntToPnt(root->children + guessQadrant, point, distance)) != NULL){
			closestPoint = clsPointReturn;
		}
		//here we traverse the rest of the tree
		for(i = 0; i<8; i++){
			/* if the closest point is found 
			* only qadrans that can produce better results are traversed
			*/
			if(nodePntDistance((root->children + i),point) < *distance && i != guessQadrant)
				if((clsPointReturn = recFindClsPntToPnt(root->children + i, point, distance)) != NULL){
					closestPoint = clsPointReturn;
				}
		}
	}else{// if leaf node
		if(root->pointDataList != NULL){//and contains data
			return clsPntInList(root->pointDataList->data, root->pointDataList->lenght, point, distance);
		}
	}
	return closestPoint;
}

float* closestPntToPnt(octTreeNode *root, float *point){
	float distance = 1000000.0f;
	return recFindClsPntToPnt(root, point, &distance);
}

static void deleteNode(octTreeNode *node){
	int i;
	if (node->children != NULL){
		for(i=0; i<8; i++){
			deleteNode(node->children + i);
		}
		free(node->children);
	}else{
		if(node->pointDataList != NULL)
			deleteArrayListf(node->pointDataList);
	}
}

void deleteOctTree(octTreeNode *root){
	deleteNode(root);
	free(root);
}

#undef octTreeXmidPoint
#undef octTreeYmidPoint
#undef octTreeZmidPoint
