#ifndef __mouseFunctions_h_
#define __mouseFunctions_h_

/* Header for mouse functions 
 * This header is strongly dependent on global variables as it can be seen as extention of Program.c file
 * TODO encapsulate window dependent variables to reduce dependency and suport multi threading (multi window)
 */
#include "globalVariables.h"

/* FreeGlut function callback for mouse wheel */
void mouseWheelFnc(GLint button, GLint dir, GLint x, GLint y);

/* Glut function callback for mouse clicks */
void mouseFcn(GLint button, GLint action, GLint x, GLint y);

/* Glut function callback for mouse movement */
void mouseMoveFnc(GLint x, GLint y);


#endif