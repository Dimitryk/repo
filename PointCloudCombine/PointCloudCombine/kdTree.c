#include <stdlib.h>
#include <string.h>
#include "kdTree.h"


#define SWAP(a,b) { TYPE temp[DIMENSIONS]; memcpy(temp,a,sizeof(TYPE)*DIMENSIONS);\
					memcpy(a,b,sizeof(TYPE)*DIMENSIONS);memcpy(b,temp,sizeof(TYPE)*DIMENSIONS); }

#define SQ(x) ((x) * (x))

typedef struct kdNode_t{
	float *data;
	struct kdNode_t *left, *right;
}kdNode;

typedef struct nodeStack_t{
	struct kdNode_t *nodeList;
	int size;
	int stackPosition;
}nodeStack;

typedef struct kdTree_t{
	struct kdNode_t *root;
	struct nodeStack_t *nodes;
	float *treeData;
}kdTree;

/* NB this stack does not support deletion of nodes
 * as this KD tree implementation does not support morfing of data
 */
static nodeStack* createNodeStack(int size){
	nodeStack *stack = (nodeStack*)malloc(sizeof(nodeStack));
	stack->nodeList = (kdNode*)malloc(sizeof(kdNode) * size);
	stack->size = size;
	stack->stackPosition = 0;
	return stack;
}

static kdNode* getNewNode(nodeStack* stack){
	return stack->nodeList + (stack->stackPosition++);
}

static void deleteNodeStack(nodeStack *stack){
	free(stack->nodeList);
	free(stack);
}

static float* findMedian(float* start, float* end, int dim){

	float *p, *sorted, *mid;
	float pivot;
	/* as data array is tightly packed floats in a form x,y,z,x,y,z 
	 * if we simply devide a pointer in halv we can end up in the middle of
	 * coordinates seaquence
	 */
	mid = start + (((end - start) / DIMENSIONS) / 2) * DIMENSIONS;

	if (end <= start) return NULL;
	if (end == start + DIMENSIONS)
		return start;

	
	while (1) {
		pivot = mid[dim];
 
		SWAP(mid, (end - DIMENSIONS));
		for (sorted = p = start; p < end; p += DIMENSIONS) {
			if (p[dim] < pivot) {
				if (p != sorted)
					SWAP(p, sorted);
				sorted += DIMENSIONS;
			}
		}
		SWAP(sorted, (end - DIMENSIONS));
 
		/* median has duplicate values */
		if (sorted[dim] == mid[dim])
			return mid;
 
		if (sorted > mid)	end = sorted;
		else		start = sorted;
	}
}

static kdNode* recCreateKDTree(float* dataStart, float* dataEnd, int dim, nodeStack* nodes){
	kdNode *currentNode;
	float *midDataPntr;

	if(dataStart>=dataEnd)
		return NULL;
	//get pre allocated pointer
	currentNode = getNewNode(nodes);
	midDataPntr = findMedian(dataStart, dataEnd, dim);
	currentNode->data = midDataPntr;
	dim = (dim + 1) % DIMENSIONS;
	currentNode->left  = recCreateKDTree(dataStart, midDataPntr, dim, nodes);
	currentNode->right = recCreateKDTree(midDataPntr + DIMENSIONS, dataEnd, dim, nodes);
	return currentNode;
}

kdTree_p createKD_Tree(float* data, int size){
	kdTree *tree;
	tree = (kdTree*)malloc(sizeof(kdTree));
	tree->treeData = (float*)malloc(sizeof(float) * size);
	memcpy(tree->treeData, data, sizeof(float) * size);
	//Aggresive allocation we know that the max number of node in a kdTree is number of data entries
	tree->nodes = createNodeStack(size/3);
	tree->root = recCreateKDTree(tree->treeData, tree->treeData + size, 0, tree->nodes);
	return tree;
}

void recFindCstPnt(kdNode* root, float* point, int dim, kdNode **best, float *bestDst)
{
	int i;
	float d = 0, ddim, ddim2;
 
	if (!root) return;
	for(i = 0; i < DIMENSIONS; i++){
		d += SQ(root->data[i] - point[i]);
	}
	ddim = root->data[dim] - point[dim];
	ddim2 = SQ(ddim);

	if (!*best || d < *bestDst) {
		*bestDst = d;
		*best = root;
	}
 
	/* if chance of exact match is high */
	if (bestDst == 0) return;
 
	dim = (dim + 1) % DIMENSIONS;
 
	recFindCstPnt(ddim > 0 ? root->left : root->right, point, dim, best, bestDst);
	if (ddim2 >= *bestDst) return;
	recFindCstPnt(ddim > 0 ? root->right : root->left, point, dim, best, bestDst);
}

float* closestPnt(kdTree_p tree, float* point, float* distance){
	kdNode *closest = NULL;
	recFindCstPnt(tree->root, point, 0, &closest, distance);
	if(!closest)
		return NULL;
	return closest->data;
}


void deleteKD_Tree(kdTree_p tree){
	deleteNodeStack(tree->nodes);
	free(tree->treeData);
	free(tree);
}
