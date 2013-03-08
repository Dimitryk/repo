#ifndef GLFW_WRAPPER_HPP_GUARD
#define GLFW_WRAPPER_HPP_GUARD
/* Avoid having GLFW include gl.h or glu.h
   since we're using glcorearb.h */
#ifndef GLFW_NO_GLU
#define GLFW_NO_GLU
#endif
#ifndef __gl_h_
#define __gl_h_
#endif
#include <GL/glfw.h>
#endif
