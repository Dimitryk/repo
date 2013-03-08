#include <math.h>
#include <stdlib.h>
#include "icp.h"
#include "octTree.h"
#include "randomHelper.h"

#define MODEL_TREE_DEPTH 4
#define MAX_FLOAT_VALUE 9999.9f
#define THRESHOLD 0.1f
#define DELTA_THRESHOLD 0.001f
#define MAX_LOCAL_ICP_ITERATIONS 50

/* Macro dotTrnsVct
 * takes dst, v1, v2, must be static arrays or pointers of atleast 3 values
 * calculates a dot product of v1 and v2 transposed
 * and saves result in dst
 */
#define dotTrnsVct(dst, v1, v2) \
	dst[0] = v1[0] * v2[0];\
	dst[1] = v1[2] * v2[0];\
	dst[2] = v1[3] * v2[0];\
	dst[3] = v1[0] * v2[1];\
	dst[4] = v1[1] * v2[1];\
	dst[5] = v1[2] * v2[1];\
	dst[5] = v1[0] * v2[2];\
	dst[6] = v1[1] * v2[2];\
	dst[8] = v1[2] * v2[2]

/* Macro mtx9AddMtx9
 * takes a, b, must be static arrays or pointers of atleast 3 values
 * adds a to b 
 * and saves result in a
 */
#define mtx9AddMtx9(a,b)	a[0] += b[0]; a[1] += b[1]; a[2] += b[2];\
							a[3] += b[3]; a[4] += b[4]; a[5] += b[5];\
							a[6] += b[6]; a[7] += b[7]; a[8] += b[8]
/* Macro matrix4Access
 * for easy access of flat array representing a 2-diamentional matrix
 * spesific case 4x4 matrix
 */
#define matrix4Access(matrix, i, j) matrix[i + j * 4]
/* Macro matrixAccess
 * for easy access of flat array representing a 2-diamentional matrix
 */
#define matrixAccess(matrix, i, j, size) matrix[i + j*4]
/* Macro for calculation of Jacobian Eigenvalues
 */
#define ROTATE(a,i,j,k,l)	g=matrix4Access(a, i, j);\
							h=matrix4Access(a,k,l);matrix4Access(a,i,j)=g-s*(h+g*tau);\
							matrix4Access(a,k,l)=h+s*(g-h*tau)
/* Macro addMtr3x3ToMtr3x3
 * takes dst, a, b, must be static arrays or pointers of atleast 3 values
 * adds a to b 
 * and saves result in dst
 */
#define addMtr3x3ToMtr3x3(dst, a, b)	dst[0] = a[0] + b[0];dst[3] = a[3] + b[3];dst[6] = a[6] + b[6];\
										dst[1] = a[1] + b[1];dst[4] = a[4] + b[4];dst[7] = a[7] + b[7];\
										dst[2] = a[2] + b[2];dst[5] = a[5] + b[5];dst[8] = a[8] + b[8]
#define substractMtr3x3ToMtr3x3(dst, a, b) \
	dst[0] = a[0] - b[0];dst[3] = a[3] - b[3];dst[6] = a[6] - b[6];\
	dst[1] = a[1] - b[1];dst[4] = a[4] - b[4];dst[7] = a[7] - b[7];\
	dst[2] = a[2] - b[2];dst[5] = a[5] - b[5];dst[8] = a[8] - b[8]


typedef struct icpStruct_t{
	int maxModelPairs;
	int dataSize;
	float *data;
	float **modelPair;
	octTreeNode *model;
	float *registrationMatrix;
	float errorMeasure;
	float dataSntrMass[4];
	float modelSntrMass[4];
}icpStruct;

static float* sntrMass(float* dst, float *mesh, int size){
	float *maxBound = mesh + size;
	memset(dst,0,sizeof(float)*3);
	dst[3] = 1.0f;
	for(;mesh < maxBound; mesh +=3){
		dst[0] += mesh[0];
		dst[1] += mesh[1];
		dst[2] += mesh[2];
	}
	dst[0] /= (size / 3);
	dst[1] /= (size / 3);
	dst[2] /= (size / 3);
	return dst;
}

static float* crossVarMtx(float *dst, icpStruct *icp){
	float dataDotmodel[9];
	float *endPointer = icp->data + icp->maxModelPairs;
	float *data = icp->data;
	float **modelPair = icp->modelPair;
	for( ; data < endPointer; data += 3, modelPair++){
		dotTrnsVct(dataDotmodel, data, *modelPair);
		mtx9AddMtx9(dst, dataDotmodel);
	}
	dotTrnsVct(dataDotmodel, icp->dataSntrMass, icp->modelSntrMass);
	dst[0] = dst[0] / icp->maxModelPairs - dataDotmodel[0];
	dst[1] = dst[1] / icp->maxModelPairs - dataDotmodel[1];
	dst[2] = dst[2] / icp->maxModelPairs - dataDotmodel[2];
	dst[3] = dst[3] / icp->maxModelPairs - dataDotmodel[3];
	dst[4] = dst[4] / icp->maxModelPairs - dataDotmodel[4];
	dst[5] = dst[5] / icp->maxModelPairs - dataDotmodel[5];
	dst[6] = dst[6] / icp->maxModelPairs - dataDotmodel[6];
	dst[7] = dst[7] / icp->maxModelPairs - dataDotmodel[7];
	dst[8] = dst[8] / icp->maxModelPairs - dataDotmodel[8];
	return dst;
}

static void toIdentity(float* matrix, int n){
			float *endPntr = matrix + n * n;
			memset(matrix,0,sizeof(float)*n*n);
			for( ; matrix<endPntr; matrix += (n + 1))
				*matrix = 1.0f;
}

/* computes egenvectors and eigenvalues using Jacobian method
 * returns egenValues on succses and NULL of failiar
 * eigenVectors on succses will be filled with egenVectors corresponding to eigenvalues
 * NB n can be up to a maximum of 10 for speed increace (static alloc vs dynamic)
 */
static float* computeJacobianEigenValuesAndVectors(float* matrix, float** eigenVectors, int n)
		{
			int j,iq,ip,i,nrot;
			float tresh,theta,tau,t,sm,s,h,g,c,b[10],z[10],*d;

			float *eigenValues;
			*eigenVectors = (float*)malloc(sizeof(float) * n*n);
			toIdentity(*eigenVectors, n);

			d = eigenValues = (float*)malloc(sizeof(float) * n );

			for (ip=0;ip<n;ip++)
			{
				b[ip]=d[ip]=matrix4Access(matrix, ip, ip); //Initialize b and d to the diagonal of a.
				z[ip]=0.0; //This vector will accumulate terms of the form tapq as in equation (11.1.14)
			}

			nrot=0;
			for (i=1;i<=50;i++)
			{
				sm=0.0;
				for (ip=0;ip<n-1;ip++) //Sum off-diagonal elements.
				{
					for (iq=ip+1;iq<n;iq++)
						sm += (float)fabs(matrix4Access(matrix, ip, iq));
				}

				if (sm == 0.0) //The normal return, which relies on quadratic convergence to machine underflow.
				{
					//we only need the absolute values of eigenvalues
					for (ip=0;ip<n;ip++)
						d[ip]=(float)fabs(d[ip]);
					return eigenValues;
				}

				if (i < 4)
					tresh = 0.2f * sm/(float)(n*n); //...on the first three sweeps.
				else
					tresh = 0.0f; //...thereafter.

				for (ip=0;ip<n-1;ip++)
				{
					for (iq=ip+1;iq<n;iq++)
					{
						g=100.0f * (float)fabs(matrix4Access(matrix, ip, iq));
						//After four sweeps, skip the rotation if the off-diagonal element is small.
						if (i > 4 && (float)(fabs(d[ip])+g) == (float)fabs(d[ip])
							&& (float)(fabs(d[iq])+g) == (float)fabs(d[iq]))
						{
							matrix4Access(matrix, ip, iq)=0.0f;
						}
						else if (fabs(matrix4Access(matrix, ip, iq)) > tresh)
						{
							h=d[iq]-d[ip];
							if ((float)(fabs(h)+g) == (float)fabs(h))
								t = matrix4Access(matrix, ip, iq )/h; //t = 1/(2¦theta)
							else
							{
								theta=0.5f * h/matrix4Access(matrix, ip, iq); //Equation (11.1.10).
								t=1.0f/(float)(fabs(theta)+sqrt(1.0f+theta*theta));
								if (theta < 0.0)
									t = -t;
							}

							c=1.0f/(float)sqrt(1.0f+t*t);
							s=t*c;
							tau=s/(1.0f+c);
							h=t*matrix4Access(matrix, ip, iq);
							z[ip] -= h;
							z[iq] += h;
							d[ip] -= h;
							d[iq] += h;
							matrix4Access(matrix, ip, iq)=0.0;

							for (j=0;j<=ip-1;j++) //Case of rotations 1 <= j < p.
							{
								ROTATE(matrix,j,ip,j,iq);
							}
							for (j=ip+1;j<=iq-1;j++) //Case of rotations p < j < q.
							{
								ROTATE(matrix,ip,j,j,iq);
							}
							for (j=iq+1;j<n;j++) //Case of rotations q < j <= n.
							{
								ROTATE(matrix,ip,j,iq,j);
							}
							for (j=0;j<n;j++)
							{
								ROTATE((*eigenVectors),j,ip,j,iq);
							}

							++nrot;
						}

					}

				}

				for (ip=0;ip<n;ip++)
				{
					b[ip]+=z[ip];
					d[ip]=b[ip]; //Update d with the sum of tapq,
					z[ip]=0.0; //and reinitialize z.
				}

			}

			//Too many iterations in routine jacobi!
			free(eigenValues);
			free(*eigenVectors);
			return NULL;
}

static float* transpose(float* dst, float* src, int n){
	int i,j;
	for(i=0; i<n; i++){
		for(j=i; j<n; j++){
			matrixAccess(dst, i, j, n) = matrixAccess(src, j, i, n);
			matrixAccess(dst, j, i, n) = matrixAccess(src, i, j, n);
		}
	}
	return dst;
}

static float trace(float* matrix, int n){
			float trace = 0;
			float *endPointer = matrix + (n*n);
			for (; matrix < endPointer; matrix += (n + 1))
				trace += *matrix;
			return trace;
}

static float* findMaxEigenVector(float* eigenValues, float* eigenVectors, int size){
	int i,maxIndex=0;
	for (i=1;i<size;++i){
		if (eigenValues[i]>eigenValues[maxIndex])
			maxIndex=i;
	}
	return eigenVectors + (maxIndex * size);
}

static float* rotationMtrxFromQuaternion( float *dst, float *q )
		{
			float q00 = q[0]*q[0];
			float q11 = q[1]*q[1];
			float q22 = q[2]*q[2];
			float q33 = q[3]*q[3];
			float q03 = q[0]*q[3];
			float q13 = q[1]*q[3];
			float q23 = q[2]*q[3];
			float q02 = q[0]*q[2];
			float q12 = q[1]*q[2];
			float q01 = q[0]*q[1];
			toIdentity(dst, 4);
			matrixAccess(dst, 0, 0, 4) = (q00 + q11 - q22 - q33);
			matrixAccess(dst, 1, 1, 4) = (q00 - q11 + q22 - q33);
			matrixAccess(dst, 2, 2, 4) = (q00 - q11 - q22 + q33);
			matrixAccess(dst, 0, 1, 4) = (2.0f*(q12-q03));
			matrixAccess(dst, 1, 0, 4) = (2.0f*(q12+q03));
			matrixAccess(dst, 0, 2, 4) = (2.0f*(q13+q02));
			matrixAccess(dst, 2, 0, 4) = (2.0f*(q13-q02));
			matrixAccess(dst, 1, 2, 4) = (2.0f*(q23-q01));
			matrixAccess(dst, 2, 1, 4) = (2.0f*(q23+q01));
			return dst;
		}

// n upp to 4
static void mtrxVctrMultiply(float *matrix, float *vector, int n){
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

static float* createRegistrationMatrix(float *registrationMatrix, icpStruct* icp){
	float sigma_px[9], sigma_px_t[9], Aij[9], bottomMat[9], transpVector[4];
	float Qsigma[16];
	float trace_sigma_px;
	float *eigenVectors, *eigenValues;

	crossVarMtx(sigma_px, icp);
	transpose(sigma_px_t, sigma_px, 3);
	substractMtr3x3ToMtr3x3(Aij,sigma_px,sigma_px_t);
	trace_sigma_px = trace(sigma_px, 3);

	memset(bottomMat,0,sizeof(float)*9);
	bottomMat[0] = -trace_sigma_px;
	bottomMat[4] = -trace_sigma_px;
	bottomMat[8] = -trace_sigma_px;
	addMtr3x3ToMtr3x3(bottomMat,sigma_px,sigma_px_t);

	//Filling Q Sigma
	matrix4Access(Qsigma, 0, 0) = trace_sigma_px;

	matrix4Access(Qsigma, 0, 1) = matrix4Access(Qsigma, 1, 0) = matrix4Access(Aij, 1, 2);
	matrix4Access(Qsigma, 0, 2) = matrix4Access(Qsigma, 2, 0) = matrix4Access(Aij, 2, 0);
	matrix4Access(Qsigma, 0, 3) = matrix4Access(Qsigma, 3, 0) = matrix4Access(Aij, 0, 1);

	matrix4Access(Qsigma, 1, 1) = matrixAccess(bottomMat, 0, 0, 3);
	matrix4Access(Qsigma, 1, 2) = matrixAccess(bottomMat, 0, 1, 3);
	matrix4Access(Qsigma, 1, 3) = matrixAccess(bottomMat, 0, 2, 3);

	matrix4Access(Qsigma, 2, 1) = matrixAccess(bottomMat, 1, 0, 3);
	matrix4Access(Qsigma, 2, 2) = matrixAccess(bottomMat, 1, 1, 3);
	matrix4Access(Qsigma, 2, 3) = matrixAccess(bottomMat, 1, 2, 3);

	matrix4Access(Qsigma, 3, 1) = matrixAccess(bottomMat, 2, 0, 3);
	matrix4Access(Qsigma, 3, 2) = matrixAccess(bottomMat, 2, 1, 3);
	matrix4Access(Qsigma, 3, 3) = matrixAccess(bottomMat, 2, 2, 3);

	//computes eigenvalues and eigenvectors from Qsigma matrix
	eigenValues = computeJacobianEigenValuesAndVectors(Qsigma, &eigenVectors, 4);
	//get rotaion Matrix from eigenvector coresponding to max eigenvalue
	rotationMtrxFromQuaternion(registrationMatrix, findMaxEigenVector(eigenValues, eigenVectors, 4));
	//copys data from data senterMass vector to transpVector
	memcpy(transpVector, icp->dataSntrMass, sizeof(float) * 4);
	//rotates SenterMass of data set by optimal rotation
	mtrxVctrMultiply(registrationMatrix, transpVector, 4);
	//translation vector = SenterMass model - rotated SenterMass data
	transpVector[0] = icp->modelSntrMass[0] - transpVector[0];
	transpVector[1] = icp->modelSntrMass[1] - transpVector[1];
	transpVector[2] = icp->modelSntrMass[2] - transpVector[2];
	// fills 4th column of registration matrix with translation vector
	memcpy(registrationMatrix + 12, transpVector, sizeof(float) * 3);
	return registrationMatrix;
}

static float findClosestPair(icpStruct *icp){
	int pairIndex, dataIndex;
	int _maxPairIndex = icp->maxModelPairs;
	float distance, avrDistance = 0;
	octTreeNode *_model = icp->model;
	float **_modelPair = icp->modelPair;
	float *_data = icp->data;

	for(pairIndex = 0; pairIndex < _maxPairIndex; pairIndex++, dataIndex += 3){
		if((_modelPair[pairIndex] = closestPntToPnt(_model, _data + dataIndex, &distance)) == NULL)
			return -1;//something bad happened in tree
		avrDistance += distance;
	}

	return avrDistance/_maxPairIndex;
}

static void matrixMultiply4fv(float *matrix, float* multipyByMatrix){
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

static void applyRegistration(icpStruct *icp, float *registrationMatrix){
	float *dataPointArray = icp->data, *endPointer = icp->data + icp->dataSize;
	for( ; dataPointArray < endPointer; dataPointArray += 3){
		mtrxVctrMultiply(registrationMatrix, dataPointArray, 3);
	}
	mtrxVctrMultiply(registrationMatrix, icp->dataSntrMass, 4);
	matrixMultiply4fv(registrationMatrix, icp->registrationMatrix);
}

static icpStruct* localICPRegistration(icpStruct *icp, float threshold, float deltaThreshold, int maxIterations){
	int iteration;
	float newError;
	float *optimalRegistrationMatrix;
	optimalRegistrationMatrix = (float*)malloc(sizeof(float) * 16);
	//run Local ICP loop
	for(iteration = 0; iteration < maxIterations; iteration++){
		//find pairs and check if error is lower then treshHold
		if((newError = findClosestPair(icp))<0)
			return icp;//something BAD ERROR HANDLING
		if((icp->errorMeasure - newError)<deltaThreshold)
			return icp;
		icp->errorMeasure = newError;
		if(icp->errorMeasure < threshold)
			return icp;
		//find optimal rotation
		optimalRegistrationMatrix = createRegistrationMatrix(optimalRegistrationMatrix, icp);
		// else appy registration
		applyRegistration(icp, optimalRegistrationMatrix);
		// increment itteration
	}
	free(optimalRegistrationMatrix);
	return icp;
}

float* globalICPRegistration(	float* model, int modelSize, 
								Vector3f min, Vector3f max, 
								float *data, int dataSize, 
								int maxIterations)
{
	float mdlSntrMass[4], dataSntrMass[4], randomTransVector[3];
	float *bestRegistration, lowestError; //stores best attempt to registrate
	int iterations;
	icpStruct *localICP;

	//create IcpStruct 
	localICP = (icpStruct*)malloc(sizeof(icpStruct));
	localICP->model = (octTreeNode*)malloc(sizeof(octTreeNode));
	localICP->data = (float*)malloc(sizeof(float) * dataSize);
	localICP->registrationMatrix = (float*)malloc(sizeof(float) * 16);
	localICP->modelPair = (float**)malloc(sizeof(float*) * localICP->maxModelPairs);

	createOctTree(localICP->model, min, max, model, modelSize, MODEL_TREE_DEPTH);
	localICP->dataSize = dataSize;
	localICP->maxModelPairs = min(modelSize, dataSize) / 3;

	bestRegistration = (float*)malloc(sizeof(float) * 16);
	sntrMass(mdlSntrMass, model, modelSize);
	sntrMass(dataSntrMass, data, dataSize);

	seedRandomGen();

	for(iterations = 0; iterations < maxIterations; iterations++){
		memcpy(localICP->data, data, sizeof(float) * dataSize);
		memcpy(localICP->dataSntrMass, dataSntrMass, sizeof(float) * 4);
		memcpy(localICP->modelSntrMass, mdlSntrMass, sizeof(float) * 4);
		localICP->errorMeasure = MAX_FLOAT_VALUE;
		toIdentity(localICP->registrationMatrix, 4);

		//fill registration Translation vector with random trans values
		matrix4Access(localICP->registrationMatrix, 3, 0) = randomInLimitf(min.x,max.x);
		matrix4Access(localICP->registrationMatrix, 3, 1) = randomInLimitf(min.y,max.y);
		matrix4Access(localICP->registrationMatrix, 3, 2) = randomInLimitf(min.z,max.z);
		
		localICPRegistration(localICP, THRESHOLD, DELTA_THRESHOLD, MAX_LOCAL_ICP_ITERATIONS);

		if( localICP->errorMeasure < lowestError ){
			memcpy(bestRegistration, localICP->registrationMatrix, sizeof(float) * 16);
			lowestError = localICP->errorMeasure;
		}
	}
	return bestRegistration;
}

