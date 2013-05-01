#include "gl3w.h"
#include <GL/freeglut.h>
#include <math.h>
#include "mouseFunctions.h"
#include "gMatrix.h"
#include "arrayList.h"
#include "vector.h"


/* Liniar Math macros on 3D vector */
#define cross3v(dest, v1, v2) \
			(dest)[0] = (v1)[1] * (v2)[2] - (v1)[2] * (v2)[1];\
			(dest)[1] = (v1)[2] * (v2)[0] - (v1)[0] * (v2)[2];\
			(dest)[2] = (v1)[0] * (v2)[1] - (v1)[1] * (v2)[0]
#define dot3v(v1, v2) ((v1)[0]*(v2)[0] + (v1)[1]*(v2)[1] + (v1)[2]*(v2)[2])
#define sub3v(dest, v1, v2) \
			(dest)[0] = (v1)[0] - (v2)[0];\
			(dest)[1] = (v1)[1] - (v2)[1];\
			(dest)[2] = (v1)[2] - (v2)[2]
#define add3v(dest, v1, v2) \
			(dest)[0] = (v1)[0] + (v2)[0];\
			(dest)[1] = (v1)[1] + (v2)[1];\
			(dest)[2] = (v1)[2] + (v2)[2]
#define multiplyConst3v(vector, constant) \
			(vector)[0] *= constant;\
			(vector)[1] *= constant;\
			(vector)[2] *= constant

#define SQ(x) ( (x) * (x) )

#define normalize3v(vector) {\
				float sqr = (float)sqrt( SQ((vector)[0]) + SQ((vector)[1]) + SQ((vector)[2]) );\
				(vector)[0] /= sqr;\
				(vector)[1] /= sqr;\
				(vector)[2] /= sqr;}

#define distanceToPlane(normal, d, point) (	(normal)[0]*(point)[0]+\
											(normal)[1]*(point)[1]+\
											(normal)[2]*(point)[2] + (d))


float pickV1[4], pickV2[4], pickV3[4], pickV4[4], pickOrigo[4];
GLint mouseMainDown, mouseSecondDown, mouseXpos, mouseYpos;


const GLint zoomFactor = 5;

static void rotateX(GLfloat value, Vector3f *rotationVector)
{
	rotationVector->x += value;
	if(rotationVector->x > 360)
		rotationVector->x = rotationVector->x - 360;
	else if(rotationVector->x < 0)
		rotationVector->x = rotationVector->x + 360;
}
static void rotateY(GLfloat value, Vector3f *rotationVector)
{
	rotationVector->y += value;
	if(rotationVector->y > 360)
		rotationVector->y-=360;
	else if(rotationVector->y < 0)
		rotationVector->y += 360;
}

/* Function to create 4 3D vectors given 4 2D points in normalize clip space and zNear plane
 * NB dependency with window/viewport aspect, height, and znear plane (or you camera to clip space transform)
 */
static void userPickVectors(float* edges, 
							float* v1, float* v2, float* v3, float* v4,
							float* origo){
	float inverseModelMatrix[16];

	v1[0] = near_height * aspect * edges[0];
	v1[1] = near_height * edges[1];
	v1[2] = -fzNear;
	v1[3] = 0.0f;

	v2[0] = near_height * aspect * edges[2];
	v2[1] = near_height * edges[3];
	v2[2] = -fzNear;
	v2[3] = 0.0f;

	v3[0] = near_height * aspect * edges[4];
	v3[1] = near_height * edges[5];
	v3[2] = -fzNear;
	v3[3] = 0.0f;

	v4[0] = near_height * aspect * edges[6];
	v4[1] = near_height * edges[7];
	v4[2] = -fzNear;
	v4[3] = 0.0f;

	origo[0] = 0;
	origo[1] = 0;
	origo[2] = 0;
	origo[3] = 1.0f;

	gPushMatrix();

	gLoadIdentity();
	gTranslate3f(-position.x, -position.y, -position.z);
	gStackMultiply(orientationMatrix);
	gInverte(inverseModelMatrix, gGetTop(), 4);
	gPopMatrix();

	gMatrixVectorMultiply(inverseModelMatrix, v1, 4);
	gMatrixVectorMultiply(inverseModelMatrix, v2, 4);
	gMatrixVectorMultiply(inverseModelMatrix, v3, 4);
	gMatrixVectorMultiply(inverseModelMatrix, v4, 4);
	gMatrixVectorMultiply(inverseModelMatrix, origo, 4);
}

/* Selects a Submesh contaning all points, from all mesh objects, that are inside a frustum, 
 * frustum is given by 6 planes defined by 4x vectors, origo and near, and far distance
 */
static void selectSubMeshFrustum(	arrayListf* subMeshVertex, arrayListf* subMeshNormals,
									float* v1, float* v2, 
									float* v3, float* v4,
									float* origo, float maxDistance){
	float normPl1[3], normPl2[3], normPl3[3], normPl4[3], normNear[3], normFar[3];
	float p1[3], p2[3], p3[3];
	float vector1[3], vector2[3];
	float d1, d2, d3, d4, dNear, dFar;
	int index;
	meshObject *object = objectsArray, *endPntr;
	float *vertexArray;
	float *vertexEnd;
	
	/* Setting up the normals for the "sides" of the fustrum */
	cross3v(normPl1, v1, v2);
	cross3v(normPl2, v2, v3);
	cross3v(normPl3, v3, v4);
	cross3v(normPl4, v4, v1);

	/* Calculating points on the nearPlane of the fustrum */
	add3v(p1, origo, v1);
	add3v(p2, origo, v2);
	add3v(p3, origo, v3);

	sub3v(vector1, p1, p2);
	sub3v(vector2, p3, p2);
	/* Calculating normal of the topp side. Positiv direction to the senter of the fustrum */
	cross3v(normNear, vector2, vector1);
	/* Normalizing Vectors for easy depth calculation of the far plane */
	normalize3v(v1)
	normalize3v(v2)
	normalize3v(v3)
	/* Increasing vectors so vector + origin gives a point on the far plane */
	multiplyConst3v(v1, maxDistance);
	multiplyConst3v(v2, maxDistance);
	multiplyConst3v(v3, maxDistance);

	add3v(p1, origo, v1);
	add3v(p2, origo, v2);
	add3v(p3, origo, v3);
	/* Vectors on the far plane */
	sub3v(vector1, p1, p2);
	sub3v(vector2, p3, p2);
	/* Normal of the farPlane to the senter of fustrum */
	cross3v(normFar, vector1, vector2);

	/* Normalize all 6 Normals to avoid it in the loop */
	normalize3v(normPl1);
	normalize3v(normPl2);
	normalize3v(normPl3);
	normalize3v(normPl4);
	normalize3v(normFar);
	normalize3v(normNear);
	/* Calculate d on a plane equation */
	d1 = (normPl1[0]*origo[0] + normPl1[1]*origo[1] + normPl1[2]*origo[2]) * -1;
	d2 = (normPl2[0]*origo[0] + normPl2[1]*origo[1] + normPl2[2]*origo[2]) * -1;
	d3 = (normPl3[0]*origo[0] + normPl3[1]*origo[1] + normPl3[2]*origo[2]) * -1;
	d4 = (normPl4[0]*origo[0] + normPl4[1]*origo[1] + normPl4[2]*origo[2]) * -1;
	dFar = (normFar[0]*p1[0] + normFar[1]*p1[1] + normFar[2]*p1[2]) * -1;
	add3v(p2, v4, origo);
	dNear = (normNear[0]*p2[0] + normNear[1]*p2[1] + normNear[2]*p2[2]) * -1;

	subMeshVertex->lenght = 0;
	subMeshNormals->lenght = 0;

	for( endPntr = object + objectsCount; object < endPntr; object++ ){
		vertexArray = object->vertexArray;
		for( vertexEnd = vertexArray + object->vertexCount; vertexArray < vertexEnd; vertexArray += 3 ){
			if(	distanceToPlane(normFar, dFar, vertexArray) > 0 || 
				distanceToPlane(normNear, dNear, vertexArray) > 0 ||
				distanceToPlane(normPl1, d1, vertexArray) > 0 ||
				distanceToPlane(normPl2, d2, vertexArray) > 0 ||
				distanceToPlane(normPl3, d3, vertexArray) > 0 ||
				distanceToPlane(normPl4, d4, vertexArray) > 0)
					continue;
			/* the point is inside the Fustrum */
			addToArrayListfv(subMeshVertex, vertexArray, 3);
			addToArrayListfv(subMeshNormals, object->normalsArray + (vertexArray - object->vertexArray), 3);
			/*index = (vertexArray - object->vertexArray) / 3 * 4;
			object->colorArray[index + 1] = -0.7f;
			object->colorArray[index + 2] = -0.7f;*/
		}
		/*meshObjectUpdateColorBuffer(object);*/
	}
}

static void mouseFunction(GLint button, GLint action, GLint x, GLint y)
{
	switch(button) 
	{
	case GLUT_LEFT_BUTTON:
		if(action == GLUT_DOWN){
			mouseMainDown = 1;
			mouseXpos = x;
			mouseYpos = y;
		}else if(action == GLUT_UP){
			mouseMainDown = 0;
		}
		else{}
		break;
	case GLUT_RIGHT_BUTTON:
		if(action == GLUT_DOWN){
			mouseSecondDown = 1;
			mouseXpos = x;
			mouseYpos = y;

		}else if(action == GLUT_UP)
			mouseSecondDown = 0;
		else{}
		break;
	default:
		break;
	}
}

static void mouseMoveFunction(GLint x, GLint y)
{
	static const float scale = -0.5f;
	if(mouseMainDown){
		gPushMatrix();
		gLoadIdentity();
		gRotate3f((mouseYpos - y) * scale, 1,0,0);
		gRotate3f((mouseXpos - x) * scale, 0,1,0);
		gStackMultiply(orientationMatrix);
		gSaveTop(orientationMatrix);
		gPopMatrix();
		mouseXpos = x;
		mouseYpos = y;
		glutPostRedisplay();
	}else if(mouseSecondDown){
		position.x -= (mouseXpos - x) * scale;
		position.y += (mouseYpos - y) * scale;
		mouseXpos = x;
		mouseYpos = y;
		glutPostRedisplay();
	}
}

static void userPickTransform(GLint x, GLint y, float* vector, float* origo){
	float inverseModelMatrix[16];

	int window_y = (window_height - y) - window_height/2;
	float norm_y = window_y/(float)(window_height/2);
	int window_x = x - window_width/2;
	float norm_x = window_x/(float)(window_width/2);
	vector[1] = near_height * norm_y;
	vector[0] = near_height * aspect * norm_x;
	vector[2] = -fzNear;
	vector[3] = 0.0f;
	origo[0] = 0;
	origo[1] = 0;
	origo[2] = 0;
	origo[3] = 1.0f;

	gPushMatrix();

	gLoadIdentity();
	gTranslate3f(-position.x, -position.y, -position.z);
	gStackMultiply(orientationMatrix);
	gInverte(inverseModelMatrix, gGetTop(), 4);
	gPopMatrix();
	gMatrixVectorMultiply(inverseModelMatrix, vector, 4);
	gMatrixVectorMultiply(inverseModelMatrix, origo, 4);
}

static void selectSubMesh(float *v1, float *v2, float *origo){
	Vector3f min = {9999.0f,9999.0f,9999.0f}, max = {-9999.0f,-9999.0f,-9999.0f};
	Vector3f minBound, maxBound;
	//float *testReturn;
	float scale1, scale2, temp;
	arrayListf *subMeshArray;
	unsigned int i;
	int objectIndex;
	int subMeshCount = 0;

	subMeshArray = createArrayListf();
	for(objectIndex = 0; objectIndex < objectsCount; objectIndex++){
		meshObject currentObject = objectsArray[objectIndex];
		
		float *vertexArray = currentObject.vertexArray;
		
		for(i = 0; i < currentObject.vertexCount; i += 3){
			scale1 = ((vertexArray[i+2]-origo[2])/v1[2]);
			scale2 = ((vertexArray[i+2]-origo[2])/v2[2]);
			minBound.x = origo[0] + v1[0] * scale1;
			maxBound.x = origo[0] + v2[0] * scale2;
			if(minBound.x>maxBound.x){
				temp = minBound.x;
				minBound.x = maxBound.x;
				maxBound.x = temp;
			}

			minBound.y = origo[1] + v1[1] * scale1;
			maxBound.y = origo[1] + v2[1] * scale2;
			if(minBound.y>maxBound.y){
				temp = minBound.y;
				minBound.y = maxBound.y;
				maxBound.y = temp;
			}
			if(vertexArray[i + 0]>minBound.x && vertexArray[i + 0]<maxBound.x
				&& vertexArray[i + 1]>minBound.y && vertexArray[i + 1]<maxBound.y){
					addToArrayListfv(subMeshArray, vertexArray + i, 3);
					currentObject.colorArray[i / 3 * 4 + 1] = -0.7f;
					currentObject.colorArray[i / 3 * 4 + 2] = -0.7f;

					if(subMeshArray->data[subMeshCount]<min.x)
						min.x = subMeshArray->data[subMeshCount];
					if(subMeshArray->data[subMeshCount + 1]<min.y)
						min.y = subMeshArray->data[subMeshCount + 1];
					if(subMeshArray->data[subMeshCount + 2]<min.z)
						min.z = subMeshArray->data[subMeshCount + 2];
					if(subMeshArray->data[subMeshCount]>max.x)
						max.x = subMeshArray->data[subMeshCount];
					if(subMeshArray->data[subMeshCount + 1]>max.y)
						max.y = subMeshArray->data[subMeshCount + 1];
					if(subMeshArray->data[subMeshCount + 2]>max.z)
						max.z = subMeshArray->data[subMeshCount + 2];

					subMeshCount+=3;
			}
		}
	meshObjectUpdateColorBuffer(objectsArray + objectIndex);
	}
	if(subMeshCount>0){
		glutPostRedisplay();
		//testReturn = globalICPRegistration(objectsArray[0].vertexArray, objectsArray[0].vertexCount, min, max, subMeshArray->data, subMeshArray->lenght, 50);
	}
	free(subMeshArray);
}

void mouseFcn(GLint button, GLint action, GLint x, GLint y)
{
	if( button == GLUT_LEFT_BUTTON && shiftButtonDown){
		if(action == GLUT_DOWN){
			initWireRecEdges(x, y);
			wireRectUpdateBuffer();
			glutPostRedisplay();
		}else if(action == GLUT_UP){
			if(wireRect.updated){
				userPickVectors(wireRect.edgeArray, pickV1, pickV2, pickV3, pickV4, pickOrigo);
				selectSubMeshFrustum(userDefinedSegment, pickV1, pickV2, pickV3, pickV4, pickOrigo, 100.0f);
				glutPostRedisplay();
			}
		}
		else{}
	}
	else{
		mouseFunction(button, action, x, y);
	}
}
void mouseMoveFnc(GLint x, GLint y){
	if(!shiftButtonDown){
		mouseMoveFunction(x, y);
	}else{
		updateWireRecEdges(x, y);
		wireRectUpdateBuffer();
		glutPostRedisplay();
	}
}
void mouseWheelFnc(GLint button, GLint dir, GLint x, GLint y){
	if(dir>0){
		position.z += zoomFactor;
		glutPostRedisplay();
	}else if(dir<0){
		position.z -= zoomFactor;
		glutPostRedisplay();
	}
}