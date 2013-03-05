//#include <GL3\GL3.h>
//#include "gl3w.h"
//#include <GL\freeglut.h>
//#include <stdio.h>
//
//
//#define windowWidth 500
//#define windowHeight 500
//
//GLuint VertexBuffer, colorBuffer, shaderProgram, vertexShader, fragmentShader, vao;
//
//GLchar* strVertexShader = {
//	"#version 330\n"
//	"layout(location = 0) in vec4 position;\n"
//	"void main()\n"
//	"{\n"
//	"   gl_Position = position;\n"
//	"}\n"
//};
//
//GLchar* strFragmentShader = {
//	"#version 330\n"
//	"out vec4 outputColor;\n"
//	"void main()\n"
//	"{\n"
//	"   outputColor = vec4(1.0f, 1.0f, 0.0f, 1.0f);\n"
//	"}\n"
//};
//
//const float vertexPositions[] = {
//	0.75f, 0.75f, 0.0f, 1.0f,
//	0.75f, -0.75f, 0.0f, 1.0f,
//	-0.75f, -0.75f, 0.0f, 1.0f,
//};
//
//const float vertexColors[] = {
//	1.0f, 0.00f, 0.0f, 1.0f,
//	0.00f, 1.00f, 0.0f, 1.0f,
//	0.00f, 0.00f, 1.0f, 1.0f,
//};
//
//GLchar* fileToShaderString(char *file)
//{
//    FILE *fptr;
//    long length;
//    char *buf;
// 
//    fptr = fopen(file, "rb"); /* Open file for reading */
//    if (!fptr) /* Return NULL on failure */
//        return NULL;
//    fseek(fptr, 0, SEEK_END); /* Seek to the end of the file */
//    length = ftell(fptr); /* Find out how many bytes into the file we are */
//    buf = (char*)malloc(length+1); /* Allocate a buffer for the entire length of the file and a null terminator */
//    fseek(fptr, 0, SEEK_SET); /* Go back to the beginning of the file */
//    fread(buf, length, 1, fptr); /* Read the contents of the file in to the buffer */
//    fclose(fptr); /* Close the file */
//    buf[length] = 0; /* Null terminator */
// 
//    return buf; /* Return the buffer */
//}
//
//int loadShaders(void){
//	char *vertexSourceString, *fragmentSourceString;
//	GLint vShaderCompiled = GL_FALSE;
//	GLint fShaderCompiled = GL_FALSE;
//
//	vertexShader = glCreateShader( GL_VERTEX_SHADER );
//
//	vertexSourceString = fileToShaderString("vertexShaderSource.vert");
//	glShaderSource( vertexShader, 1,(const GLchar**)&(vertexSourceString), NULL );
//	glCompileShader( vertexShader );
//    glGetShaderiv( vertexShader, GL_COMPILE_STATUS, &vShaderCompiled );
//    if( vShaderCompiled != GL_TRUE )
//        return 0;
//	free(vertexSourceString);
//
//	fragmentShader = glCreateShader( GL_FRAGMENT_SHADER );
//
//	fragmentSourceString = fileToShaderString("fragmentShaderSource.frag");
//	glShaderSource( fragmentShader, 1, (const GLchar**)&fragmentSourceString, NULL );
//    glCompileShader( fragmentShader );
//    glGetShaderiv( fragmentShader, GL_COMPILE_STATUS, &fShaderCompiled );
//    if( fShaderCompiled != GL_TRUE )
//        return 0;
//	free(fragmentSourceString);
//	return 1;
//}
//
//int createShaterProgram(void){
//	GLint programSuccess, maxL;
//	char *err;
//
//	shaderProgram = glCreateProgram();
//
//	glAttachShader( shaderProgram, vertexShader );
//    glAttachShader( shaderProgram, fragmentShader );
//
//    glLinkProgram( shaderProgram );
//
//    glGetProgramiv( shaderProgram, GL_LINK_STATUS, &programSuccess );
//    if( programSuccess != GL_TRUE )
//         {
//       /* Noticed that glGetProgramiv is used to get the length for a shader program, not glGetShaderiv. */
//       glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &maxL);
// 
//       /* The maxLength includes the NULL character */
//       err = (char *)malloc(maxL);
// 
//       /* Notice that glGetProgramInfoLog, not glGetShaderInfoLog. */
//       glGetProgramInfoLog(shaderProgram, maxL, &maxL, err);
// 
//       /* Handle the error in an appropriate way such as displaying a message or writing to a log file. */
//       /* In this simple program, we'll just leave */
//       free(err);
//       return 0;
//    }
//	glDetachShader(shaderProgram, vertexShader);
//	glDetachShader(shaderProgram, fragmentShader);
//	return 1;
//	glDeleteShader(vertexShader);
//	glDeleteShader(fragmentShader);
//}
//
//void genBuffers(void){
//
//	glGenBuffers(1, &VertexBuffer);
//
//	glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
//	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPositions), vertexPositions, GL_STATIC_DRAW);
//
//	glGenBuffers(1, &colorBuffer);
//
//	glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
//	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexColors), vertexColors, GL_STATIC_DRAW);
//	glBindBuffer(GL_ARRAY_BUFFER, 0);
//	
//}
//
//void init(void){
//	if(!loadShaders()){
//		printf("Error in loading shaders");
//		exit(1);
//	}
//	if(!createShaterProgram()){
//		printf("Failed to link shaders");
//		exit(1);
//	}
//	genBuffers();
//
//	glGenVertexArrays(1, &vao);
//	glBindVertexArray(vao);
//}
//
//void display(void)
//{
//	GLenum err;
//	GLint artibuteLocation;
//	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
//	glClear(GL_COLOR_BUFFER_BIT);
//	
//	glUseProgram(shaderProgram);
//	
//	artibuteLocation = glGetAttribLocation(shaderProgram, "vertColor");
//
//	glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
//	glEnableVertexAttribArray(artibuteLocation);
//	glVertexAttribPointer(artibuteLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);
//
//	artibuteLocation = glGetAttribLocation(shaderProgram, "position");
//	glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
//	glEnableVertexAttribArray(artibuteLocation);
//	glVertexAttribPointer(artibuteLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);
//
//	glDrawArrays(GL_TRIANGLES, 0, 3);
//
//	glDisableVertexAttribArray(0);
//	glDisableVertexAttribArray(1);
//	glUseProgram(0);
//
//	glutSwapBuffers();
//	err = glGetError();
//}
//
//void reshape (int w, int h)
//{
//	glViewport(0, 0, (GLsizei) w, (GLsizei) h);
//}
//
//int main(int argc, char** argv){
//	glutInit(&argc, argv);
//
//	glutInitDisplayMode (GLUT_DOUBLE | GLUT_ALPHA );
//	glutInitContextVersion (3, 3);
//	glutInitContextProfile(GLUT_CORE_PROFILE);
//
//	glutInitWindowSize (windowWidth, windowHeight); 
//	glutInitWindowPosition (300, 200);
//	glutCreateWindow ("My window GL");
//	
//	if(gl3wInit()<0)
//		return 1;
//
//	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);
//
//	init();
//	glutDisplayFunc(display); 
//	glutReshapeFunc(reshape);
//	glutMainLoop();
//
//	return(0);
//}