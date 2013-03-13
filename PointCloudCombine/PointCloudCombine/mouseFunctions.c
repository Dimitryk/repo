#include "gl3w.h"
#include <GL/freeglut.h>
#include "vector.h"
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