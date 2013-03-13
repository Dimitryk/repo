/*
 * Read Obj header
 * 
 *
 *
 */

#ifndef __robj_h_
#define __robj_h_

typedef struct obj_t_ *obj_p;

obj_p openOBJ(char*);

void setVertex_cb(obj_p obj, int (*vertexFunction)(float *vertex, int));
void setNormal_cb(obj_p obj, int (*normalFunction)(float *normal, int));
void setTexture_cb(obj_p obj, int (*textureFunction)(float *texture, int));
void setFace_cb(obj_p obj, int (faceFunction)(unsigned int* vertexIndex, unsigned int* textureIndex,unsigned int* normalIndex,int));

int readOBJ(obj_p);
int closeOBJ(obj_p);

#endif