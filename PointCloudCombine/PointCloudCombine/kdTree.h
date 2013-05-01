#ifndef __kdTree_h
#define __kdTree_h

#define DIMENSIONS 3
#define TYPE float

typedef struct kdTree_t *kdTree_p;

kdTree_p createKD_Tree(float* data, int size);

/* Returns pointer to point data
 * stores distance to returned point in distance param
 */
float* closestPnt(kdTree_p, float* point, float* distance);

float** kdTree_KNN(kdTree_p tree, float** kNearestsPoints, float* point, int k, float maxSQDistance);

void deleteKD_Tree(kdTree_p tree);
#endif