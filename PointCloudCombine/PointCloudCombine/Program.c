#include "gl3w.h"
#include <GL\freeglut.h>
#include <stdio.h>
#include <math.h>
#include "GLSLprogram.h"
#include "meshObject.h"
#include "vector.h"
#include "gMatrix.h"
#include "icp.h"
#include "arrayList.h"
#include "mouseFunctions.h"

#define windowWidth 500
#define windowHeight 500


GLSLprogram *gouraudShading;
GLuint ppBuffer;

static meshObject objectsArray[5];
static int objectsCount = 0;

static const float fzFar = 2000.0f;
static const float fzNear = 1.5f;
static float aspect = 1.0f, near_height = 1;
float ppMatrix[16], orientationMatrix[16];
Vector3f position;

const int g_projectionBlockIndex = 2;

float startPickRay[4], endPickRay[4], pickOrigo[4];
GLint shiftButtonDown;

int window_height, window_width;


static void loadLights(void){
	glUseProgram(gouraudShading->ID);

	glUniform4f(gouraudShading->ambientIntensityUnif, 0.1f, 0.1f, 0.1f, 1.0f);
	glUniform4f(gouraudShading->lightIntensityUnif, 0.8f, 0.8f, 0.8f, 1.0f);
	glUniform3f(gouraudShading->dirToLightUnif, -0.534f, 0.233f, 0.233f);

	glUseProgram(0);
}

static void setPrespectiveProjection(void){
	memset(ppMatrix, 0, sizeof(float) * 16);

	ppMatrix[0] = (2 * fzNear) / ((near_height + near_height)*aspect);
	ppMatrix[5] = (2 * fzNear) / (near_height + near_height);
	ppMatrix[10] =  - (fzFar + fzNear) / (fzFar - fzNear);
	ppMatrix[14] =  - (2 * fzFar * fzNear) / (fzFar - fzNear);
	ppMatrix[11] = -1.0f;
}

void genUniformBuffer(void){

	glGenBuffers(1, &ppBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, ppBuffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(ppMatrix), NULL, GL_DYNAMIC_DRAW);
	glBindBufferRange(GL_UNIFORM_BUFFER, g_projectionBlockIndex, ppBuffer, 0, sizeof(ppMatrix));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

}

static int loadPointMeshFile(void){
	char fileName[256];
	
	printf("Please type the name og the file you want to open \n");
	gets(fileName);
	objectsArray[objectsCount++] = *(createMeshObject(fileName, gouraudShading));

	return 0;
}

void init(void){
	gouraudShading = createShaderProgram("vertexShaderSource.vert","fragmentShaderSource.frag", g_projectionBlockIndex);
	if(gouraudShading->linkStatus == GL_FALSE)
		exit(0);

	setPrespectiveProjection();
	genUniformBuffer();

	loadLights();

	gLoadIdentity();
	gSaveTop(orientationMatrix);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	glDepthRange(0.0f, 1.0f);
	glEnable(GL_DEPTH_CLAMP);

	loadPointMeshFile();
	//loadPointMeshFile();
}

void display(void)
{
	int i;
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	gPushMatrix();
	gTranslate3f(-position.x,-position.y,-position.z);
	gStackMultiply(orientationMatrix);

	for(i = 0; i < objectsCount; i++){
		drawMeshObject(&objectsArray[i]);
	}
	
	gPopMatrix();
	glutSwapBuffers();
}

void reshape (int w, int h)
{
	aspect = w/(float)h;
	ppMatrix[0] = 2 * fzNear / ((near_height + near_height)*aspect);
	ppMatrix[5] = 2 * fzNear / (near_height + near_height);
	window_height = h;
	window_width = w;

	glBindBuffer(GL_UNIFORM_BUFFER, ppBuffer);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ppMatrix), ppMatrix);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glViewport(0, 0, (GLsizei) w, (GLsizei) h);
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
	float *testReturn;
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
	}
	if(subMeshCount>0){
		testReturn = globalICPRegistration(objectsArray[0].vertexArray, objectsArray[0].vertexCount, min, max, subMeshArray->data, subMeshArray->lenght, 50);
	}
	free(subMeshArray);
}
void mouseFcn(GLint button, GLint action, GLint x, GLint y)
{
	if( button == GLUT_LEFT_BUTTON && shiftButtonDown){
		if(action == GLUT_DOWN){
			userPickTransform(x,y,startPickRay,pickOrigo);
		}else if(action == GLUT_UP){
			userPickTransform(x,y,endPickRay,pickOrigo);
			selectSubMesh(startPickRay,endPickRay,pickOrigo);
			}
		else{}
		}
	else{
		mouseFunction(button, action, x, y);
	}
}
void mouseMoveFnc(GLint x, GLint y){
	if(!shiftButtonDown){
		mouseMoveFunction(x, y,&position, orientationMatrix);
	}
}
void mouseWheelFnc(GLint button, GLint dir, GLint x, GLint y){
	mouseZoomFnc(dir, &position);
}
void keyboardS(GLint key, GLint x, GLint y)
{
	switch (key) {
	case GLUT_KEY_UP:
		{

			position.z += 5;
			glutPostRedisplay();
		}
		break;
	case GLUT_KEY_DOWN:
		{
			position.z -= 5;
			glutPostRedisplay();
		};
		break;
	case GLUT_KEY_RIGHT:
		{
			//rotateY(15);
			glutPostRedisplay();
		};
		break;
	case GLUT_KEY_LEFT:
		{
			//rotateY(-15);
			glutPostRedisplay();
		};
		break;
	case GLUT_KEY_PAGE_UP:
		{

		};
		break;
	case GLUT_KEY_PAGE_DOWN:
		{
		};
		break;
	case GLUT_KEY_SHIFT_L:
		{
			shiftButtonDown = 1;
		};
	default:
		break;
	}
}
void keyboardSpecialUpFunc(GLint key, GLint x, GLint y){
	switch (key) {
		case GLUT_KEY_SHIFT_L:
			{
			shiftButtonDown = 0;
		};
			default:
		break;
	}
}
int main(int argc, char** argv){

	glutInit(&argc, argv);	

	glutInitDisplayMode (GLUT_DOUBLE | GLUT_ALPHA );
	glutInitContextVersion (3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	glutInitWindowSize (windowWidth, windowHeight); 
	glutInitWindowPosition (300, 200);
	glutCreateWindow ("My window GL");

	if(gl3wInit()<0)
		return 1;

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);
	
	init();
	glutDisplayFunc(display); 
	glutReshapeFunc(reshape);
	glutSpecialFunc(keyboardS);
	glutSpecialUpFunc(keyboardSpecialUpFunc);
	glutMouseFunc(mouseFcn);
	glutMotionFunc(mouseMoveFnc);
	glutMouseWheelFunc(mouseWheelFnc);
	glutMainLoop();

	finalizeProgram(gouraudShading);
	return(0);
}