#include <string.h>
#include <stdlib.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include "gMatrix.h"

static struct matrixStack_t{
	int currentMatrix;
	float stack[MAX_MATRIX_STACK_SIZE * 16];
	float normalMatrix[9];
}matrixStack;

float* gGetTop(void){
	return (matrixStack.stack + matrixStack.currentMatrix * 16);
}

float* gSaveTop(float* dst){
	return (float*)memcpy(dst, matrixStack.stack + matrixStack.currentMatrix * 16, sizeof(float) * 16);
}

int gPushMatrix(void){
	if (matrixStack.currentMatrix >= MAX_MATRIX_STACK_SIZE)
		return 0;
	memcpy((matrixStack.stack + (matrixStack.currentMatrix + 1) * 16),
		(matrixStack.stack + matrixStack.currentMatrix * 16),
		16 * sizeof(float));
	matrixStack.currentMatrix++;
	return 1;
}

int gPopMatrix(void){
	if (matrixStack.currentMatrix <= 0)
		return 0;
	return matrixStack.currentMatrix--;
}
/* Multiplies current top stack matrix by 4x4 matrix given by float*/
void gStackMultiply(float *matrix){
	float temp[16], *stack;
	int row, col;
	stack = matrixStack.stack + matrixStack.currentMatrix * 16;
	for(row = 0; row < 4; row++){
		for(col = 0; col < 4; col++){
			temp[row * 4 + col] = matrix[row * 4] * stack[col]
								+ matrix[row * 4 + 1] * stack[4 + col]
								+ matrix[row * 4 + 2] * stack[4 * 2 + col]
								+ matrix[row * 4 + 3] * stack[4 * 3 + col];
		}
	}
	memcpy(stack, temp, 16 * sizeof(float));
}

void setIdentity(float *matrix){
	matrix[0] = 1;
	matrix[1] = 0;
	matrix[2] = 0;
	matrix[3] = 0;
	matrix[4] = 0;
	matrix[5] = 1;
	matrix[6] = 0;
	matrix[7] = 0;
	matrix[8] = 0;
	matrix[9] = 0;
	matrix[10] = 1;
	matrix[11] = 0;
	matrix[12] = 0;
	matrix[13] = 0;
	matrix[14] = 0;
	matrix[15] = 1;
}

void gRotate3f(float rotationAngle, float vX, float vY, float vZ){
	float quaterionRotationMatrix[16];
	float cosA = cos(rotationAngle / 180.0f * M_PI);
	float oneC = 1 - cosA;
	float sinA = sin(rotationAngle / 180.0f * M_PI);
	float axisVectLength = sqrt(vX*vX + vY*vY + vZ*vZ);
	float ux = vX/axisVectLength;
	float uy = vY/axisVectLength;
	float uz = vZ/axisVectLength;
	
	setIdentity(quaterionRotationMatrix);

	quaterionRotationMatrix[0] = ux*ux*oneC + cosA;
	quaterionRotationMatrix[1] = uy*ux*oneC + uz*sinA;
	quaterionRotationMatrix[2] = uz*ux*oneC - uy*sinA;
	quaterionRotationMatrix[4] = ux*uy*oneC - uz*sinA;
	quaterionRotationMatrix[5] = uy*uy*oneC + cosA;
	quaterionRotationMatrix[6] = uz*uy*oneC + ux*sinA;
	quaterionRotationMatrix[8] = ux*uz*oneC + uy*sinA;
	quaterionRotationMatrix[9] = uy*uz*oneC - ux*sinA;
	quaterionRotationMatrix[10] = uz*uz*oneC + cosA;

	gStackMultiply(quaterionRotationMatrix);
}

void gRotate2fv(float rotationAngle, float* p1, float* p2){
	float quaterionRotationMatrix[16];
	float cosA = cos(rotationAngle / 180.0f * M_PI);
	float oneC = 1 - cosA;
	float sinA = sin(rotationAngle / 180.0f * M_PI);
	float axisVectLength = sqrt((p2[0]-p1[0])*(p2[0]-p1[0])
								+ (p2[1]-p1[1])*(p2[1]-p1[1]) 
								+ (p2[2]-p1[2])*(p2[2]-p1[2]));
	float ux = (p2[0]-p1[0])/axisVectLength;
	float uy = (p2[1]-p1[1])/axisVectLength;
	float uz = (p2[2]-p1[2])/axisVectLength;
	
	gTranslate3f(-p1[0], -p1[1], -p1[2]);

	setIdentity(quaterionRotationMatrix);

	quaterionRotationMatrix[0] = ux*ux*oneC + cosA;
	quaterionRotationMatrix[1] = uy*ux*oneC + uz*sinA;
	quaterionRotationMatrix[2] = uz*ux*oneC - uy*sinA;
	quaterionRotationMatrix[4] = ux*uy*oneC - uz*sinA;
	quaterionRotationMatrix[5] = uy*uy*oneC + cosA;
	quaterionRotationMatrix[6] = uz*uy*oneC + ux*sinA;
	quaterionRotationMatrix[8] = ux*uz*oneC + uy*sinA;
	quaterionRotationMatrix[9] = uy*uz*oneC - ux*sinA;
	quaterionRotationMatrix[10] = uz*uz*oneC + cosA;

	gStackMultiply(quaterionRotationMatrix);

	gTranslate3f(p1[0], p1[1], p1[2]);
}

void gScale3f(float x, float y, float z){
	float scaleMatrix[16];

	setIdentity(scaleMatrix);

	scaleMatrix[3] = x;
	scaleMatrix[7] = y;
	scaleMatrix[11] = z;

	gStackMultiply(scaleMatrix);
}

void gTranslate3f(float x, float y, float z){
	float translateMatrix[16];

	setIdentity(translateMatrix);

	translateMatrix[12] = x;
	translateMatrix[13] = y;
	translateMatrix[14] = z;

	gStackMultiply(translateMatrix);
}

void gLoadIdentity(void){
	setIdentity(matrixStack.stack + matrixStack.currentMatrix * 16);
}

float* gGetTopNormal3fv(void){
	memcpy(matrixStack.normalMatrix, (matrixStack.stack + matrixStack.currentMatrix * 16), 3 * sizeof(float));
	memcpy((matrixStack.normalMatrix + 3), (matrixStack.stack + matrixStack.currentMatrix * 16 + 4), 3 * sizeof(float));
	memcpy((matrixStack.normalMatrix + 6), (matrixStack.stack + matrixStack.currentMatrix * 16 + 8), 3 * sizeof(float));
	gInverte(matrixStack.normalMatrix, matrixStack.normalMatrix, 3);
	return matrixStack.normalMatrix;
}

float* gInverte(float* dst, float* src, int n){
    float *matrix, ratio,a;
    int i, j, k, size;
	size = n*n;
	// matrix = [ src | I ]
	matrix = (float*)malloc(sizeof(float)*size*2);
	memcpy(matrix, src, sizeof(float)*size);
	//fill the rightside of the matrix with Identety
    for(i = 0; i < n; i++){
        for(j = n; j < 2*n; j++){
            if(i==(j-n))
                matrix[i + j * n] = 1.0;
            else
                matrix[i + j * n] = 0.0;
        }
    }
	// flat column-major matrix representation m2d[i][j] = m1d[i + j * n]
    for(i = 0; i < n; i++){
        for(j = 0; j < n; j++){
            if(i!=j){
                ratio = matrix[j + i * n]/matrix[i * n + i];
                for(k = 0; k < 2*n; k++){
                    matrix[j + k * n] -= ratio * matrix[i + k * n];
                }
            }
        }
    }
    for(i = 0; i < n; i++){
        a = matrix[i * n + i];
        for(j = 0; j < 2*n; j++){
            matrix[i + j * n] /= a;
        }
    }
	// now matrix = [ I | srcInverse ]
    memcpy(dst, (matrix + size), sizeof(float) * size);
	free(matrix);
    return dst;
}

void gMatrixMultiply4fv(float *matrix, float* multipyByMatrix){
	float temp[16];
	int row, col;
	for(row = 0; row < 4; row++){
		for(col = 0; col < 4; col++){
			temp[row * 4 + col] = multipyByMatrix[row * 4] * matrix[col]
								+ multipyByMatrix[row * 4 + 1] * matrix[4 + col]
								+ multipyByMatrix[row * 4 + 2] * matrix[4 * 2 + col]
								+ multipyByMatrix[row * 4 + 3] * matrix[4 * 3 + col];
		}
	}
	memcpy(multipyByMatrix, temp, 16 * sizeof(float));
}

void gMatrixVectorMultiply(float *matrix, float *vector, int n){
	float temp[4], sum;
	int row, col;
	for(col=0;col<n;col++){
		sum = 0;
		for(row=0;row<n;row++){
			sum += matrix[col + row * n] * vector[row];
		}
		temp[col] = sum;
	}
	memcpy(vector, temp, sizeof(float)*n);
}