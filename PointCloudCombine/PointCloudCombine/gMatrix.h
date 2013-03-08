
/* Matrix stack manipulation header
 * Represent orientation matrix in a form of a 16 floats array,
 * a 4 by 4 column-major matrix
 */

#ifndef __gMatrix_h_
#define __gMatrix_

/* A maximum size of a Matrix Stack */
#define MAX_MATRIX_STACK_SIZE 10

/* Returns a pointer to a array representing current top a matrix stack */
float* gGetTop(void);
/* Coppys top of the matrixstack to dst pointer
 * returns dst or NULL pointer
 */
float* gSaveTop(float* dst);
/* Copys current top of the stack and pushes it on the stack 
 * returns 0 if MAX_MATRIX_STACK_SIZE is reached
 */
int gPushMatrix(void);
/* Removes the current matrix 
 * returns 0 if the stack is empty
 */
int gPopMatrix(void);
/* creates a quatroion rotation matrix about axis defined by origo and x,y,z coordinates
 * and multiplys it with top of the stack
 * Parrameters: rotation angle, x, y, z
 */
void gRotate3f(float angle, float, float, float);
/* creates a quatroion rotation matrix about axis defined by p1 and p2
 * and multiplys it with top of the stack
 * Parrameters: rotation angle, p1, p2
 * p1 and p2 must be a arrays of 3 floats: x, y, z
 */
void gRotate2fv(float, float* p1, float* p2);
/* multiplys top of the stack with a scale matrix defined by x, y, z
 */
void gScale3f(float x, float y, float z);
/* multiplys top of the stack with a translate matrix defined by x, y, z
 */
void gTranslate3f(float x, float y, float z);
/* Replaces the top of the stac with the Identity matrix
 */
void gLoadIdentity(void);
/* returns a pointer to a 3 by 3 matrix used for normals calculations
 * based on the top of the stack. For right lighting the matrix has beed inverted,
 * but not transposed, transposing a matrix in openGL is as easy as changing
 * collumn and row major
 */
float* gGetTopNormal3fv(void);
/* Computes a inverse of src and saves it int dst
 * using Gauss-Jordan Method for nn matrixes
 */
float* gInverte(float* dst, float* src, int n);

void gStackMultiply(float *matrix);
/* Multiplies m1 by m2 and stores result in m2
 */
void gMatrixMultiply(float* m1, float* m2, int n);
/* Multiplies n by n matrix by n vertical vector and stores result in vector
 */
void gMatrixVectorMultiply(float *matrix, float *vector, int n);

#endif