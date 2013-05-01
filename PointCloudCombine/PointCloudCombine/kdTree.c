
#include <stdlib.h>
#include <string.h>
#include "kdTree.h"


#define SWAP(a,b) { TYPE *temp; temp = (a)->data;\
					(a)->data = (b)->data; (b)->data = temp; }

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
}kdTree;

typedef struct neighbor_t{
	float *data;
	float distance;
}neighbour;
typedef struct kNearestNeighbours_t{
	struct neighbor_t *neighbours;
	struct neighbor_t *worst;
	int k;
}kNN;

/* NB this stack does not support deletion of nodes
 * as this KD tree implementation does not support morfing of data
 */
static nodeStack* createNodeStack(float* data, int size){
	nodeStack *stack = (nodeStack*)malloc(sizeof(nodeStack));
	kdNode *index, *end;
	stack->nodeList = (kdNode*)malloc(sizeof(kdNode) * size);
	stack->size = size;
	stack->stackPosition = 0;
	for(index = stack->nodeList, end = stack->nodeList + stack->size;
		index < end; index++, data += DIMENSIONS ){
			index->data = data;
	}
	return stack;
}

static kdNode* getNewNode(nodeStack* stack){
	return stack->nodeList + (stack->stackPosition++);
}

static void deleteNodeStack(nodeStack *stack){
	free(stack->nodeList);
	free(stack);
}

static kdNode* findMedian(kdNode* start, kdNode* end, int dim){

	kdNode *p, *sorted, *mid;
	float pivot;
	/* as data array is tightly packed floats in a form x,y,z,x,y,z 
	 * if we simply devide a pointer in halv we can end up in the middle of
	 * coordinates seaquence
	 */
	mid = start + ((end - start) / 2);

	if (end <= start) return NULL;
	if (end == start + 1)
		return start;

	
	while (1) {
		pivot = mid->data[dim];
 
		SWAP(mid, (end - 1));
		for (sorted = p = start; p < end; p++) {
			if (p->data[dim] < pivot) {
				if (p != sorted)
					SWAP(p, sorted);
				sorted += 1;
			}
		}
		SWAP(sorted, (end - 1));
 
		/* median has duplicate values */
		if (sorted->data[dim] == mid->data[dim])
			return mid;
 
		if (sorted > mid)	end = sorted;
		else		start = sorted;
	}
}

static kdNode* recCreateKDTree(kdNode* dataStart, kdNode* dataEnd, int dim){
	kdNode *currentNode;
	kdNode *midDataPntr;

	if(dataStart>=dataEnd)
		return NULL;
	//get pre allocated pointer
	midDataPntr = findMedian(dataStart, dataEnd, dim);
	currentNode = midDataPntr;
	dim = (dim + 1) % DIMENSIONS;
	currentNode->left  = recCreateKDTree(dataStart, midDataPntr, dim);
	currentNode->right = recCreateKDTree(midDataPntr + 1, dataEnd, dim);
	return currentNode;
}

kdTree_p createKD_Tree(float* data, int size){
	kdTree *tree;
	tree = (kdTree*)malloc(sizeof(kdTree));
	tree->nodes = createNodeStack(data, size / DIMENSIONS);
	tree->root = recCreateKDTree(tree->nodes->nodeList, tree->nodes->nodeList + tree->nodes->size, 0);
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

static void addclosestNeighbor( kNN *kNeighbours, float* data, float distance ){
	neighbour *curentNeighbour = kNeighbours->neighbours, *endPntr = kNeighbours->neighbours + kNeighbours->k;
	kNeighbours->worst->data = data;
	kNeighbours->worst->distance = distance;
	for( ; curentNeighbour < endPntr; curentNeighbour++){
		if(curentNeighbour->distance > kNeighbours->worst->distance){
			kNeighbours->worst = curentNeighbour;
		}
	}
}

static void recFindKNN(kdNode* root, float* point, int dim, kNN *best)
{
	int i;
	float d = 0, ddim, ddim2;
 
	if (!root) return;
	for(i = 0; i < DIMENSIONS; i++){
		d += SQ(root->data[i] - point[i]);
	}
	ddim = root->data[dim] - point[dim];
	ddim2 = SQ(ddim);

	if (d < best->worst->distance) {
		addclosestNeighbor(best, root->data, d);
	}
 
	dim = (dim + 1) % DIMENSIONS;
 
	recFindKNN(ddim > 0 ? root->left : root->right, point, dim, best);
	if (ddim2 >= best->worst->distance) return;
	recFindKNN(ddim > 0 ? root->right : root->left, point, dim, best);
}

float** kdTree_KNN(kdTree_p tree, float** kNearestsPoints, float* point, int k, float maxSQDistance){
	kNN nNearest;
	float d = 0;
	int i;

	nNearest.k = k;
	nNearest.neighbours = (neighbour*)malloc(sizeof(neighbour) * k);
	if(nNearest.neighbours == NULL)
		return NULL;
	nNearest.worst = nNearest.neighbours;

	for( i = 0; i < k; i++){
		nNearest.neighbours[i].data = NULL;
		nNearest.neighbours[i].distance = maxSQDistance;
	}
	recFindKNN(tree->root, point, 0, &nNearest);

	for( i = 0; i < k; i++ ){
		kNearestsPoints[i] = nNearest.neighbours[i].data;
	}
	free(nNearest.neighbours);
	return kNearestsPoints;
}

void deleteKD_Tree(kdTree_p tree){
	deleteNodeStack(tree->nodes);
	free(tree);
}
