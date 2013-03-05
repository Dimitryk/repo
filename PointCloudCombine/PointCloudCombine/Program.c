#include "gl3w.h"
#include <GL\freeglut.h>
#include <stdio.h>
#include <math.h>
#include "GLSLprogram.h"
#include "robjtogl.h"
#include "rPPoints.h"
#include "vector.h"
#include "gMatrix.h"
#include "octTree.h"
#include "mouseFunctions.h"

#define windowWidth 500
#define windowHeight 500


GLSLprogram *shaderProgram;

GLuint VertexBuffer, colorBuffer, elementBuffer, normalsBuffer;
GLuint ppBuffer;
GLuint vao;
GLuint ppMatrixBlock, modelToCameraMatrixUnif, normalModelToCameraMatrixUnif;
GLuint ambientIntensityUnif, lightIntensityUnif, dirToLightUnif;

static const float fzFar = 2000.0f;
static const float fzNear = 1.5f;
static float aspect = 1.0f, near_height = 1;
float ppMatrix[16], orientationMatrix[16];
Vector3f position;

GLfloat *vertexArray, *normalsArray;
GLuint *elementArray;
unsigned int vertexCount, elementCount;

const int g_projectionBlockIndex = 2;

float startPickRay[4], endPickRay[4], pickOrigo[4];
GLint shiftButtonDown;

int window_height, window_width;

octTreeNode *subMeshTree = NULL;

static void loadLights(void){
	glUseProgram(shaderProgram->ID);

	glUniform4f(shaderProgram->ambientIntensityUnif, 0.1f, 0.1f, 0.1f, 1.0f);
	glUniform4f(shaderProgram->lightIntensityUnif, 0.8f, 0.8f, 0.8f, 1.0f);
	glUniform3f(shaderProgram->dirToLightUnif, -0.534f, 0.233f, 0.233f);

	glUseProgram(0);
}


static Vector3f calculateNormal(GLfloat triagleVertexArray[]){
	Vector3f Normal; 
	GLfloat l;

	Vector3f U = {	triagleVertexArray[3] - triagleVertexArray[0],
		triagleVertexArray[4] - triagleVertexArray[1],
		triagleVertexArray[5] - triagleVertexArray[2]};

	Vector3f V = {	triagleVertexArray[6] - triagleVertexArray[0],
		triagleVertexArray[7] - triagleVertexArray[1],
		triagleVertexArray[8] - triagleVertexArray[2]};

	Normal.x = U.y * V.z - U.z * V.y;
	Normal.y = U.z * V.x - U.x * V.z;
	Normal.z = U.x * V.y - U.y * V.x;
	l = (GLfloat)sqrt(Normal.x * Normal.x + Normal.y * Normal.y + Normal.z * Normal.z);
	Normal.x /= l;
	Normal.y /= l;
	Normal.z /= l;

	return Normal;
}
static int createNormArray(GLfloat **normArray){
	unsigned int i;
	Vector3f tempNorm;
	GLfloat triVertexArray[9];

	if(normArray==NULL){
		*normArray = (GLfloat*) malloc(vertexCount *  sizeof(Vector3f));
	}else{
		*normArray = (GLfloat*) realloc(*normArray, vertexCount * sizeof(Vector3f));
	}
	for(i=0;i<elementCount;i+=3){
		triVertexArray[0] = vertexArray[elementArray[i] * 3 + 0];
		triVertexArray[1] = vertexArray[elementArray[i] * 3 + 1];
		triVertexArray[2] = vertexArray[elementArray[i] * 3 + 2];
		triVertexArray[3] = vertexArray[elementArray[i + 1] * 3 + 0];
		triVertexArray[4] = vertexArray[elementArray[i + 1] * 3 + 1];
		triVertexArray[5] = vertexArray[elementArray[i + 1] * 3 + 2];
		triVertexArray[6] = vertexArray[elementArray[i + 2] * 3 + 0];
		triVertexArray[7] = vertexArray[elementArray[i + 2] * 3 + 1];
		triVertexArray[8] = vertexArray[elementArray[i + 2] * 3 + 2];

		tempNorm = calculateNormal(triVertexArray);
		(*normArray)[elementArray[i] * 3 + 0] = tempNorm.x;
		(*normArray)[elementArray[i] * 3 + 1] = tempNorm.y;
		(*normArray)[elementArray[i] * 3 + 2] = tempNorm.z;
		(*normArray)[elementArray[i + 1] * 3 + 0] = tempNorm.x;
		(*normArray)[elementArray[i + 1] * 3 + 1] = tempNorm.y;
		(*normArray)[elementArray[i + 1] * 3 + 2] = tempNorm.z;
		(*normArray)[elementArray[i + 2] * 3 + 0] = tempNorm.x;
		(*normArray)[elementArray[i + 2] * 3 + 1] = tempNorm.y;
		(*normArray)[elementArray[i + 2] * 3 + 2] = tempNorm.z;
	}
	return 1;
}

static int loadPLY(char *filename){
	if (!openPLY_file(filename))
		return 0;
	vertexCount = createVertexArray(&vertexArray);
	elementCount = createTriList(&elementArray);
	return read_PLY();
}

static int loadOBJ(char *filename){
	if (!openOBJ_file(filename))
		return 0;
	setVertexArray(&vertexArray, &vertexCount);
	setElementArray(&elementArray, &elementCount);
	if(!readOBG_file())
		return 0;

}

static void setPrespectiveProjection(void){
	memset(ppMatrix, 0, sizeof(float) * 16);

	ppMatrix[0] = (2 * fzNear) / ((near_height + near_height)*aspect);
	ppMatrix[5] = (2 * fzNear) / (near_height + near_height);
	ppMatrix[10] =  - (fzFar + fzNear) / (fzFar - fzNear);
	ppMatrix[14] =  - (2 * fzFar * fzNear) / (fzFar - fzNear);
	ppMatrix[11] = -1.0f;
}

void genBuffers(void){

	glGenBuffers(1, &ppBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, ppBuffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(ppMatrix), NULL, GL_DYNAMIC_DRAW);
	glBindBufferRange(GL_UNIFORM_BUFFER, g_projectionBlockIndex, ppBuffer, 0, sizeof(ppMatrix));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glGenBuffers(1, &VertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(vertexArray), vertexArray, GL_STATIC_DRAW);

	glGenBuffers(1, &normalsBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalsBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(normalsArray), normalsArray, GL_STATIC_DRAW);

	//glGenBuffers(1, &colorBuffer);
	//glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(vertexColors), vertexColors, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &elementBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, elementCount * sizeof(elementArray), elementArray, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

static void bindVOA(void){
	GLint artibuteLocation;

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glUseProgram(shaderProgram->ID);

	artibuteLocation = glGetAttribLocation(shaderProgram->ID, "position");
	glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
	glEnableVertexAttribArray(artibuteLocation);
	glVertexAttribPointer(artibuteLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);

	artibuteLocation = glGetAttribLocation(shaderProgram->ID, "normal");
	glBindBuffer(GL_ARRAY_BUFFER, normalsBuffer);
	glEnableVertexAttribArray(artibuteLocation);
	glVertexAttribPointer(artibuteLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);

	/*artibuteLocation = glGetAttribLocation(shaderProgram, "color");
	glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
	glEnableVertexAttribArray(artibuteLocation);
	glVertexAttribPointer(artibuteLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);*/

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);

	glUseProgram(0);

	glBindVertexArray(0);
}

static int loadPointMeshFile(void){
	char *tolken, fileName[256];
	
	printf("Please type the name og the file you want to open \n");
	gets(fileName);

	for(tolken = fileName;*tolken && *tolken != '.';tolken++);
	tolken++;
	if(strcmp(tolken, "ply") == 0){
		return loadPLY(fileName);
	}else if(strcmp(tolken, "obj") == 0){
		return loadOBJ(fileName);
	}
	printf("Bad file format: %s",tolken);
	return 0;
}

void init(void){
	loadPointMeshFile();
	shaderProgram = createShaderProgram("vertexShaderSource.vert","fragmentShaderSource.frag", g_projectionBlockIndex);
	if(shaderProgram->linkStatus == GL_FALSE)
		exit(0);

	setPrespectiveProjection();

	createNormArray(&normalsArray);
	genBuffers();
	loadLights();

	bindVOA();

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
}

static void drawMesh(void){
	GLenum err;

	glUseProgram(shaderProgram->ID);

	gPushMatrix();
	gTranslate3f(-position.x,-position.y,-position.z);
	gStackMultiply(orientationMatrix);
	

	glUniformMatrix4fv(shaderProgram->modelToCameraMatrixUnif, 1, GL_FALSE, gGetTop());
	glUniformMatrix3fv(shaderProgram->normalModelToCameraMatrixUnif, 1, GL_TRUE, gGetTopNormal3fv());

	glBindVertexArray(vao);

	glDrawElements(GL_TRIANGLES, elementCount, GL_UNSIGNED_INT, 0);

	gPopMatrix();
	glBindVertexArray(0);
	err = glGetError();
	glUseProgram(0);
}


void display(void)
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	drawMesh();

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
void selectSubMesh(float *v1, float *v2, float *origo){
	Vector3f min = {9999.0f,9999.0f,9999.0f}, max = {-9999.0f,-9999.0f,-9999.0f};
	Vector3f minBound, maxBound;
	float testPoint[3] = {0,0,0},*test;
	float scale1, scale2, temp, *subMeshArray;
	unsigned int i;
	int subMeshCount = 0;
	// Aggresive allocation
	subMeshArray = (float*)malloc(sizeof(float)*vertexCount);
	for(i=0;i<vertexCount;i+=3){
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
				subMeshArray[subMeshCount] = vertexArray[i];
				subMeshArray[subMeshCount + 1] = vertexArray[i + 1];
				subMeshArray[subMeshCount + 2] = vertexArray[i + 2];
				
				if(subMeshArray[subMeshCount]<min.x)
					min.x = subMeshArray[subMeshCount];
				if(subMeshArray[subMeshCount + 1]<min.y)
					min.y = subMeshArray[subMeshCount + 1];
				if(subMeshArray[subMeshCount + 2]<min.z)
					min.z = subMeshArray[subMeshCount + 2];
				if(subMeshArray[subMeshCount]>max.x)
					max.x = subMeshArray[subMeshCount];
				if(subMeshArray[subMeshCount + 1]>max.y)
					max.y = subMeshArray[subMeshCount + 1];
				if(subMeshArray[subMeshCount + 2]>max.z)
					max.z = subMeshArray[subMeshCount + 2];
				
				subMeshCount+=3;
		}
	}
	if(subMeshCount>0){
		if(subMeshTree==NULL){
			subMeshTree = (octTreeNode*)malloc(sizeof(octTreeNode));
			createOctTree(subMeshTree, min, max, subMeshArray, subMeshCount, 6);
			test = closestPntToPnt(subMeshTree, testPoint);
			deleteOctTree(subMeshTree);
			subMeshTree = NULL;
		}
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

	finalizeProgram(shaderProgram);
	return(0);
}