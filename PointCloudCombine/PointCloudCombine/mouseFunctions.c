#include "gl3w.h"
#include <GL/freeglut.h>
#include "mouseFunctions.h"
#include "gMatrix.h"

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

void userPickVectors(float* edges, 
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

void mouseFunction(GLint button, GLint action, GLint x, GLint y)
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

void mouseMoveFunction(GLint x, GLint y, Vector3f *positionVector, float *rotationMatrix)
{
	static const float scale = -0.5f;
	if(mouseMainDown){
		gPushMatrix();
		gLoadIdentity();
		gRotate3f((mouseYpos - y) * scale, 1,0,0);
		gRotate3f((mouseXpos - x) * scale, 0,1,0);
		gStackMultiply(rotationMatrix);
		gSaveTop(rotationMatrix);
		gPopMatrix();
		mouseXpos = x;
		mouseYpos = y;
		glutPostRedisplay();
	}else if(mouseSecondDown){
		positionVector->x -= (mouseXpos - x) * scale;
		positionVector->y += (mouseYpos - y) * scale;
		mouseXpos = x;
		mouseYpos = y;
		glutPostRedisplay();
	}
}

void mouseZoomFnc(GLint dir, Vector3f *positionVector){
	if(dir>0){
		positionVector->z += zoomFactor;
		glutPostRedisplay();
	}else if(dir<0){
		positionVector->z -= zoomFactor;
		glutPostRedisplay();
	}
}