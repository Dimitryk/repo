/* Entry File
 * This is main entry file that starts upp freeGlut window/s
 * Contaning Glut main loop functions callbacks, and OpenGL functions and shaders
 * TODO encapsulate window dependent variables to reduce dependency and suport multi threading (multi window)
 */

#include "gl3w.h" /*Must be befor other GL heades*/
#include <GL\freeglut.h>
#include <stdio.h>
#include <math.h>
#include "globalVariables.h"
#include "GLSLprogram.h"
#include "meshObject.h"
#include "vector.h"
#include "gMatrix.h"
#include "arrayList.h"
#include "mouseFunctions.h"
#include "pointCloudCombine.h"
#include "objectGeometryMeshConverter.h"
#include "geometryMesh.h"
#include "pointCloudCombine.h"

#define windowWidth 500
#define windowHeight 500

GLSLprogram *gouraudShading;
GLuint ppBuffer;

int startDraw = 0;

const int g_projectionBlockIndex = 2;

arrayListui *vertexIndex;

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
	
	wireRect.updated = 0;
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

void updateWireRecEdges(int x, int y){
	int window_y = (window_height - y) - window_height/2;
	float norm_y = window_y/(float)(window_height/2);
	int window_x = x - window_width/2;
	float norm_x = window_x/(float)(window_width/2);
	wireRect.edgeArray[3] = norm_y;
	wireRect.edgeArray[4] = norm_x;
	wireRect.edgeArray[5] = norm_y;
	wireRect.edgeArray[6] = norm_x;
	wireRect.updated = ( (wireRect.edgeArray[0] == norm_x) && (wireRect.edgeArray[1] == norm_y) ) ? 0 : 1 ;
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
	meshObject *mesh;
	
	printf("Please type the name of the file you want to open \n");
	gets(fileName);
	if( (mesh = createMeshObject(fileName, gouraudShading)) != NULL){
		objectsArray[objectsCount++] = mesh;
	}else
		printf("Error: file could not be opened");

	return 0;
}

static void setMarkingColor(void){
	markingColor[0] = 0.0f;
	markingColor[1] = -0.70f;
	markingColor[2] = -0.70f;
	markingColor[3] = 0.0f;

	reverseMarkingColor[0] = 0.0f;
	reverseMarkingColor[1] = 0.00f;
	reverseMarkingColor[2] = 0.00f;
	reverseMarkingColor[3] = 0.0f;
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

	objectsArray = (meshObject**)malloc(sizeof(meshObject*) * MAX_OBJECTS);
	loadPointMeshFile();
	loadPointMeshFile();
	userDefinedSegmentVertex = createArrayListf();
	userDefinedSegmentColor = createArrayListui();
	setMarkingColor();
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

	for(i = startDraw; i < objectsCount; i++){
		drawMeshObject(objectsArray[i]);
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

void colorIndexes( meshObject *obj, arrayListui* list, float* color ){
	int i;
	if (list == NULL)
		return;

	for ( i = 0; i< list->lenght; i++){
		obj->colorArray[list->data[i] + 0] = color[0];
		obj->colorArray[list->data[i] + 1] = color[1];
		obj->colorArray[list->data[i] + 2] = color[2];
		obj->colorArray[list->data[i] + 3] = color[3];
	}
	meshObjectUpdateColorBuffer(obj);

	glutPostRedisplay();
}

void keyboardS(GLint key, GLint x, GLint y)
{
	switch (key) {
	case GLUT_KEY_UP:
		{
			startDraw++;
			glutPostRedisplay();
		}
		break;
	case GLUT_KEY_DOWN:
		{
			startDraw--;
			glutPostRedisplay();
		};
		break;
	case GLUT_KEY_RIGHT:
		{
			geometryMesh *model, *data, *out;
			meshObject *temp;
			model = convertObjectGeometryMesh(objectsArray[0]);
			data = convertObjectGeometryMesh(objectsArray[1]);
			out = pointCloudCombine(model, data, userDefinedSegmentVertex);
			temp = objectsArray[0];
			deleteMeshObject(objectsArray[0]);
			objectsArray[0] = meshObjectFromGeometry(out, gouraudShading);
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
			objectsCount++;
			glutPostRedisplay();
		};
		break;
	case GLUT_KEY_PAGE_DOWN:
		{
			objectsCount--;
			glutPostRedisplay();
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
			glutPostRedisplay();
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