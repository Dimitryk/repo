#include <stdlib.h>
#include <math.h>
#include "octTree.h"

#define octTreeXmidPoint(node) (node->minCoordinates.x + (node->maxCoordinates.x - node->minCoordinates.x) / 2)
#define octTreeYmidPoint(node) (node->minCoordinates.y + (node->maxCoordinates.y - node->minCoordinates.y) / 2)
#define octTreeZmidPoint(node) (node->minCoordinates.z + (node->maxCoordinates.z - node->minCoordinates.z) / 2)

#define getQuadrant(root, point) (((point[0] > octTreeXmidPoint(root)) ? 1 : 0 )+\
	((point[1] > octTreeZmidPoint(root)) ? 2 : 0) + ((point[2] > octTreeYmidPoint(root)) ? 4 : 0))

#define SQ(x) ((x) * (x))
#define SQDistance3D(a,b,c) (SQ(a) + SQ(b) + SQ(c))
#define SQDistance3Dv(vector) (SQDistance3D((vector).x, (vector).y, (vector).z))

#define distance(a,b) (max((a),(b)) - min((a),(b)))
#define euclideanDistance(a,b,c) (sqrt((a)*(a) + (b)*(b) + (c)*(c)))
#define nodePntDistance(node,point) (euclideanDistance(\
	min(distance(node->minCoordinates.x,point[0]),distance(node->maxCoordinates.x,point[0])),\
	min(distance(node->minCoordinates.y,point[1]),distance(node->maxCoordinates.y,point[1])),\
	min(distance(node->minCoordinates.z,point[2]),distance(node->maxCoordinates.z,point[2]))))
#define pntToPntDistance(a,b) (euclideanDistance(distance((a)[0],(b)[0]),distance((a)[1],b[1]),distance((a)[2],(b)[2])))

/* A node struct 
 * Each node in the tree has it diamentions set by bounds
 * given in minCoordinates and maxCoordinates Vectors
 */
typedef struct octTreeNode_t{
	Vector3f minCoordinates, maxCoordinates;
	struct octTreeNode_t *children;
	void* dataList;
}octTreeNode;

typedef struct octNodeStack_t{
	struct octTreeNode_t *nodeList;
	int size;
	int stackPosition;
}octNodeStack;

typedef struct octTree_t{
	struct octNodeStack_t *stack;
	struct octTreeNode_t *root;	
	int (*addDataFnc)(octTreeNode_p, void*, int);
	void (*deleteDataFnc)(void*);
}octTree;

static octNodeStack* createNodeStack(int size){
	octNodeStack *stack = (octNodeStack*)malloc(sizeof(octNodeStack));
	if(stack == NULL)
		return NULL;
	stack->nodeList = (octTreeNode*)malloc(sizeof(octTreeNode) * size);
	if(stack->nodeList == NULL)
		free(stack);
		return NULL;
	stack->size = size;
	stack->stackPosition = 0;
	return stack;
}
static octTreeNode* getNewOctNodev(octNodeStack* stack, int size){
	octTreeNode *temp;
	if((stack->stackPosition + size) >= stack->size)
		return NULL;
	temp = stack->nodeList + stack->stackPosition;
	stack->stackPosition += size;
	return temp;
}



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
static int setNodeChildren(octTreeNode *current, octNodeStack* stack, int depth){

	if(depth){
		current->dataList = NULL;
		current->children = getNewOctNodev(stack, 8);
		if(current->children==NULL)
			return 0;

		setBoundsFirstNode(current);
		if(!setNodeChildren(current->children + 0, stack, depth - 1))
			return 0;

		setBoundsSecondNode(current);
		if(!setNodeChildren(current->children + 1, stack, depth - 1))
			return 0;

		setBoundsThirdNode(current);
		if(!setNodeChildren(current->children + 2, stack, depth - 1))
			return 0;

		setBoundsFourthNode(current);
		if(!setNodeChildren(current->children + 3, stack, depth - 1))
			return 0;

		setBoundsFifthNode(current);
		if(!setNodeChildren(current->children + 4, stack, depth - 1))
			return 0;

		setBoundsSixthNode(current);
		if(!setNodeChildren(current->children + 5, stack, depth - 1))
			return 0;

		setBoundsSeventhNode(current);
		if(!setNodeChildren(current->children + 6, stack, depth - 1))
			return 0;

		setBoundsEightNode(current);
		if(!setNodeChildren(current->children + 7, stack, depth - 1))
			return 0;

		return 1;
	}
	current->children = NULL;
	current->dataList = NULL;
	return 1;
}

//int octTreeAddPoint(octTreeNode *root, float *data){
//	int position = 0;
//	if(root->children == NULL){
//		if(root->pointDataList == NULL){
//			root->pointDataList = createArrayListf();
//			return addToArrayListfv(root->pointDataList, data, 3);
//		}else
//			return addToArrayListfv(root->pointDataList, data, 3);
//	}
//	position += (data[0] > octTreeXmidPoint(root)) ? 1 : 0 ;
//	position += (data[2] > octTreeZmidPoint(root)) ? 2 : 0 ;
//	position += (data[1] > octTreeYmidPoint(root)) ? 4 : 0 ;
//	return octTreeAddPoint(root->children + position, data);
//}

static int pruneOctTree(octTreeNode* root){
	int numberValidChildren = 0;
	if(root->children != NULL){
		octTreeNode *currentChild, *endChildPntr = root->children + 8;
		for(currentChild = root->children; currentChild < endChildPntr; currentChild++){
			numberValidChildren += pruneOctTree(currentChild);
			}
		
		if(numberValidChildren == 0){
			root->children = NULL;
		}
		return numberValidChildren;
	}else{
		if(root->dataList == NULL)
			return 0;
		else
			return 1;
	}
	//Will not happened
	return -1;
}

//int createOctTree(octTreeNode *root, Vector3f minBound, Vector3f maxBound, float *pointData, int arraySize, int depth){
//	float *dataEndPntr;
//	root->minCoordinates = minBound;
//	root->maxCoordinates = maxBound;
//	if(!setNodeChildren(root, depth))
//		return 0;
//	for(dataEndPntr = pointData + arraySize; pointData < dataEndPntr; pointData += 3){
//		{
//			if(!octTreeAddPoint(root, pointData))
//				return 0;
//		}
//	}
//	//pruneOctTree(root);
//	return 1;
//}

octTree_p createOctTree(	Vector3f minBound, Vector3f maxBound, 
							void* pointData, int arraySize, 
							int depth,
							int (*addData_cb)(octTreeNode*, void*, int),
							void (*deleteData_cb)(void*))
{
	octTree_p tree;

	tree = (octTree*)malloc(sizeof(octTree));

	tree->stack = createNodeStack((int)pow(8, depth) + 1);
	
	tree->root = getNewOctNodev(tree->stack, 1);
	tree->root->minCoordinates = minBound;
	tree->root->maxCoordinates = maxBound;
	tree->addDataFnc = addData_cb;
	tree->deleteDataFnc = deleteData_cb;
	if(!setNodeChildren(tree->root, tree->stack, depth))
		return NULL;
	if(!tree->addDataFnc(tree->root, pointData, arraySize)){
		deleteOctTree(tree);
		return NULL;
	}
	return tree;
}

Vector3f getNodeMinBound(octTreeNode_p node){
	return node->minCoordinates;
}
Vector3f getNodeMaxBound(octTreeNode_p node){
	return node->maxCoordinates;
}

octTreeNode_p getNodesChildren(octTreeNode_p node){
	return node->children;
}

octTreeNode_p getNodesChild(octTreeNode_p node, int index){
	return node->children + index;
}

void* getNodesDataPntr(octTreeNode_p node){
	return node->dataList;
}

void setNodesDataPntr(octTreeNode_p node, void* pntr){
	node->dataList = pntr;
}

int addDataOctTree(octTree_p tree, void* data, int size){
	return tree->addDataFnc(tree->root, data, size);
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
//static float* recFindClsPntToPnt(octTreeNode *root, float *point, float *distance){
//	int i, guessQadrant;
//	float *closestPoint = NULL;
//	float *clsPointReturn;
//	if(root->children != NULL){
//		//we we estimate the best quadrant first
//		guessQadrant = getQuadrant(root, point);
//		if((clsPointReturn = recFindClsPntToPnt(root->children + guessQadrant, point, distance)) != NULL){
//			closestPoint = clsPointReturn;
//		}
//		//here we traverse the rest of the tree
//		for(i = 0; i<8; i++){
//			/* if the closest point is found 
//			* only qadrans that can produce better results are traversed
//			*/
//			if(nodePntDistance((root->children + i),point) < *distance && i != guessQadrant)
//				if((clsPointReturn = recFindClsPntToPnt(root->children + i, point, distance)) != NULL){
//					closestPoint = clsPointReturn;
//					
//				}
//		}
//	}else{// if leaf node
//		if(root->pointDataList != NULL){//and contains data
//			return clsPntInList(root->pointDataList->data, root->pointDataList->lenght, point, distance);
//		}
//	}
//	return closestPoint;
//}

//float* closestPntToPnt(octTreeNode* root, float* point, float* distance){
//	*distance = 1000000.0f;//should be float.max
//	return recFindClsPntToPnt(root, point, distance);
//}

int rayOctreeIntersept(octTreeNode* root, float* ray, float* origin, float maxSqDistance){
	float tmax, tmin;
	Vector3f maxRayLength, minRayLength, tmaxPoint, tminPoint, rayDirection, rayOrigin;

	rayDirection.x = ray[0];
	rayDirection.y = ray[1];
	rayDirection.z = ray[2];

	rayOrigin.x = origin[0];
	rayOrigin.y = origin[1];
	rayOrigin.z = origin[2];


	/* ray through x parallel square */
	if(rayDirection.x != 0){
		maxRayLength.x = root->maxCoordinates.x - rayOrigin.x;
		tmax = maxRayLength.x / rayDirection.x;

		maxRayLength.y = rayDirection.y * tmax;
		maxRayLength.z = rayDirection.z * tmax;
		/* Cheks if the ray in shorter then max square distance */
		if( SQDistance3Dv(maxRayLength) < maxSqDistance ){
			tmaxPoint.y = rayOrigin.y + maxRayLength.y;
			tmaxPoint.z = rayOrigin.z + maxRayLength.z;
			/* The ray goes through the square defined by y min-max and z min-max when it at x= x.max*/
			if( (root->minCoordinates.y < tmaxPoint.y) && (tmaxPoint.y < root->maxCoordinates.y)
				&& (root->minCoordinates.z < tmaxPoint.z) && (tmaxPoint.z < root->maxCoordinates.z) ){
					return 1;
			}	
		}

		minRayLength.x = root->minCoordinates.x - rayOrigin.x;
		tmin = minRayLength.x / rayDirection.x;

		minRayLength.y = rayDirection.y * tmin;
		minRayLength.z = rayDirection.z * tmin;
		if( SQDistance3Dv(minRayLength) < maxSqDistance ){
			tminPoint.y = rayOrigin.y + minRayLength.y;
			tminPoint.z = rayOrigin.z + minRayLength.z;
			/* The ray goes through the square defined by y min-max and z min-max when it at x= x.min*/
			if( (root->minCoordinates.y < tminPoint.y) && (tminPoint.y < root->maxCoordinates.y)
				&& (root->minCoordinates.z < tminPoint.z) && (tminPoint.z < root->maxCoordinates.z) ) {
					return 1;
			}
		}
	}
	/* ray through y parallel square */
	if(rayDirection.y != 0){
		maxRayLength.y = root->maxCoordinates.y - rayOrigin.y;
		tmax = maxRayLength.y / rayDirection.y;

		maxRayLength.x = rayDirection.x * tmax;
		maxRayLength.z = rayDirection.z * tmax;
		if( SQDistance3Dv(maxRayLength) < maxSqDistance ){
			tmaxPoint.x = rayOrigin.x + maxRayLength.x;
			tmaxPoint.z = rayOrigin.z + maxRayLength.y;
			if(		(root->minCoordinates.x < tmaxPoint.x) && (tmaxPoint.x < root->maxCoordinates.x)
				&&	(root->minCoordinates.z < tmaxPoint.z) && (tmaxPoint.z < root->maxCoordinates.z) ){
					return 1;
			}
		}

		minRayLength.y = root->minCoordinates.y - rayOrigin.y;
		tmin = minRayLength.y / rayDirection.y;

		minRayLength.x = rayDirection.x * tmin;
		minRayLength.z = rayDirection.z * tmin;
		if( SQDistance3Dv(minRayLength) < maxSqDistance ){
			tminPoint.x = rayOrigin.x + minRayLength.x;
			tminPoint.z = rayOrigin.z + minRayLength.z;
			if( (root->minCoordinates.x < tminPoint.x) && (tminPoint.x < root->maxCoordinates.x)
				&& (root->minCoordinates.z < tminPoint.z) && (tminPoint.z < root->maxCoordinates.z) ){
					return 1;
			}
		}
	}
	/* ray through z parallel square */
	if(rayDirection.z != 0){
		maxRayLength.z = root->maxCoordinates.z - rayOrigin.z;
		tmax = maxRayLength.z / rayDirection.z;

		maxRayLength.x = rayDirection.x * tmax;
		maxRayLength.y = rayDirection.y * tmax;
		if( SQDistance3Dv(maxRayLength) < maxSqDistance ){
			tmaxPoint.x = rayOrigin.x + maxRayLength.x;
			tmaxPoint.y = rayOrigin.y + maxRayLength.y;
			if( (root->minCoordinates.x < tmaxPoint.x) && (tmaxPoint.x < root->maxCoordinates.x)
				&& (root->minCoordinates.y < tmaxPoint.y) && (tmaxPoint.y < root->maxCoordinates.y) ){
					return 1;
			}
		}

		minRayLength.z = root->minCoordinates.z - rayOrigin.z;
		tmin = minRayLength.z / rayDirection.z;

		minRayLength.x = rayDirection.x * tmin;
		minRayLength.y = rayDirection.y * tmin;
		if( SQDistance3Dv(minRayLength) < maxSqDistance ){
			tminPoint.x = rayOrigin.x + minRayLength.x;
			tminPoint.y = rayOrigin.y + minRayLength.y;
			if( (root->minCoordinates.x < tminPoint.x) && (tminPoint.x < root->maxCoordinates.x)
				&& (root->minCoordinates.y < tminPoint.y) && (tminPoint.y < root->maxCoordinates.y) ){
					return 1;
			}
		}
	}
	/* the ray did not intersept any of the sides of octree cube meaning it missed it */
	return 0;
}

static void recDeleteNodesData(octTreeNode* root, void(*deleteFunc)(void*)){
	if(root->children != NULL){
		int i;
		for(i = 0; i < 8; i++)
			recDeleteNodesData(root->children + i, deleteFunc);
	}else if(root->dataList != NULL)
		deleteFunc(root->dataList);
}

static void deleteNodeStack(octNodeStack* stack){
	if(stack->nodeList != NULL)
		free(stack->nodeList);
	free(stack);
}

void deleteOctTree(octTree_p tree){
	recDeleteNodesData(tree->root, tree->deleteDataFnc);
	deleteNodeStack(tree->stack);
	free(tree);
}
