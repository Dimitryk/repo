#include "gl3w.h"
#include <GL\freeglut.h>
#include <stdio.h>
#include <math.h>
#include "globalVariables.h"
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

const int g_projectionBlockIndex = 2;



void initWireRecEdges(int x, int y){
	int window_y = (window_height - y) - window_height/2;
	float norm_y = window_y/(float)(window_height/2);
	int window_x = x - window_width/2;
	float norm_x = window_x/(float)(window_width/2);

	wireRect.edgeArray[0] = norm_x;
	wireRect.edgeArray[1] = norm_y;
	wireRect.edgeArray[2] = norm_x;
	wireRect.edgeArray[3] = norm_y;
	wireRect.edgeArray[4] = norm_x;
	wireRect.edgeArray[5] = norm_y;
	wireRect.edgeArray[6] = norm_x;
	wireRect.edgeArray[7] = norm_y;
}

static void loadLights(void){
	glUseProgram(gouraudShading->ID);

	glUniform4f(gouraudShading->ambientIntensityUnif, 0.05f, 0.05f, 0.05f, 1.0f);
	glUniform4f(gouraudShading->lightIntensityUnif, 0.7f, 0.7f, 0.7f, 1.0f);
	glUniform3f(gouraudShading->dirToLightUnif, -0.57735f, 0.57736f, 0.57735f);

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

static void initWireRect(void){
	GLint artibuteLocation;
	
	wireRect.simpleProgram = createShaderProgram("toClipSpace.vert","fragmentShaderSource.frag", g_projectionBlockIndex);
	initWireRecEdges(0, 0);
	glGenBuffers(1, &(wireRect.edgeBuffer));
	glBindBuffer(GL_ARRAY_BUFFER, wireRect.edgeBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(wireRect.edgeArray), wireRect.edgeArray, GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenVertexArrays(1, &(wireRect.recVOA));
	glBindVertexArray(wireRect.recVOA);

	glUseProgram(wireRect.simpleProgram->ID);

	artibuteLocation = glGetAttribLocation(wireRect.simpleProgram->ID, "position");
	glBindBuffer(GL_ARRAY_BUFFER, wireRect.edgeBuffer);
	glEnableVertexAttribArray(artibuteLocation);
	glVertexAttribPointer(artibuteLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
	glBindVertexArray(0);
}

void init(void){
	gouraudShading = createShaderProgram("vertexShaderSource.vert","fragmentShaderSource.frag", g_projectionBlockIndex);
	if(gouraudShading->linkStatus == GL_FALSE)
		exit(0);
	initWireRect();

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

void drawWireRect(void){
	glBindVertexArray(wireRect.recVOA);
	glDrawArrays(GL_LINE_LOOP, 0, 4);
	glBindVertexArray(0);
}

void wireRectUpdateBuffer(void){
	glBindBuffer(GL_ARRAY_BUFFER, wireRect.edgeBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(wireRect.edgeArray), wireRect.edgeArray, GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
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
	if(shiftButtonDown)
		drawWireRect();
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

#define cross3v(dest, v1, v2) \
			(dest)[0] = (v1)[1] * (v2)[2] - (v1)[2] * (v2)[1];\
			(dest)[1] = (v1)[2] * (v2)[0] - (v1)[0] * (v2)[2];\
			(dest)[2] = (v1)[0] * (v2)[1] - (v1)[1] * (v2)[0]
#define dot3v(v1, v2) ((v1)[0]*(v2)[1] + (v1)[0]*(v2)[1] + (v1)[2]*(v2)[2])
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
											(normal)[2]*(point)[2] + d)

static void selectSubMeshFoustum(	float* v1, float* v2, 
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
	arrayListf *subMeshArray;
	
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

	subMeshArray = createArrayListf();

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
			//addToArrayListfv(subMeshArray, vertexArray, 3);
			index = (vertexArray - object->vertexArray) / 3 * 4;
			object->colorArray[index + 1] = -0.7f;
			object->colorArray[index + 2] = -0.7f;
		}
		meshObjectUpdateColorBuffer(object);
	}

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
			userPickVectors(wireRect.edgeArray, pickV1, pickV2, pickV3, pickV4, pickOrigo);
			selectSubMeshFoustum(pickV1, pickV2, pickV3, pickV4, pickOrigo, 100.0f);
			glutPostRedisplay();
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
	}else{
		int window_y = (window_height - y) - window_height/2;
		float norm_y = window_y/(float)(window_height/2);
		int window_x = x - window_width/2;
		float norm_x = window_x/(float)(window_width/2);
		wireRect.edgeArray[3] = norm_y;
		wireRect.edgeArray[4] = norm_x;
		wireRect.edgeArray[5] = norm_y;
		wireRect.edgeArray[6] = norm_x;
		wireRectUpdateBuffer();
		glutPostRedisplay();
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