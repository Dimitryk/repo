#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include "rply.h"
#include "rplytogl.h"

static unsigned long nvertices, ntriangles;
static GLfloat *vertexArray;
static GLuint *triArray;
static p_ply ply;

static int vertex_cb(p_ply_argument argument) {
    long element, index_nr;
	ply_get_argument_element(argument, NULL, &index_nr);
    ply_get_argument_user_data(argument, NULL, &element);
	switch (element){
	case 0:
		vertexArray[index_nr*3] = (GLfloat) (ply_get_argument_value(argument));
		break;
	case 1:
		vertexArray[index_nr*3 + 1 ] = (GLfloat) (ply_get_argument_value(argument));
		break;
	case 2:
		vertexArray[index_nr*3 + 2] = (GLfloat) (ply_get_argument_value(argument));
	}
    return 1;
}

static int face_cb(p_ply_argument argument) {
    long length, value_index, index_nr;
	ply_get_argument_element(argument, NULL, &index_nr);
    ply_get_argument_property(argument, NULL, &length, &value_index);
    switch (value_index) {
        case 0:
			triArray[index_nr * 3] = (GLuint)ply_get_argument_value(argument);
			break;
        case 1:
            triArray[index_nr *3 +1] = (GLuint)ply_get_argument_value(argument);
			break;
        case 2:
            triArray[index_nr *3 +2] = (GLuint)ply_get_argument_value(argument);
			break;
        default: 
            break;
    }
    return 1;
}

int openPLY_file(char* filename){
	p_ply_element element;
	long ninstances;
	char* name;
	ply = ply_open(filename, NULL, 0, NULL);
    if (!ply) return 0;
    if (!ply_read_header(ply)) return 0;
	for(element = ply_get_next_element(ply,  NULL); element != NULL; element = ply_get_next_element(ply,  element)){
		ply_get_element_info(element, &name, &ninstances);
		if(!strcmp(name, "vertex"))
			nvertices = 3 * ninstances;
		else if(!strcmp(name, "face"))
			ntriangles = 3 * ninstances;
	}
	return 1;
}

long createVertexArrayPLY(GLfloat **vArray) {
	if (ply==NULL)
		return 0;
	*vArray = (GLfloat*) malloc(nvertices * sizeof(GLfloat));
	if(*vArray == NULL)
		return -1;

	vertexArray = *vArray;
	ply_set_read_cb(ply, "vertex", "x", vertex_cb, NULL, 0);
    ply_set_read_cb(ply, "vertex", "y", vertex_cb, NULL, 1);
    ply_set_read_cb(ply, "vertex", "z", vertex_cb, NULL, 2);


	return nvertices;
	
}

long createElementArrayPLY(GLuint **tArray){
	if (ply==NULL)
		return 0;
	*tArray = (GLuint*) malloc(ntriangles *  sizeof(GLuint));
	if(*tArray == NULL)
		return -1;

	triArray = *tArray;
	ply_set_read_cb(ply, "face", "vertex_indices", face_cb, NULL, 0);

	return ntriangles;
}

int read_PLY(void){
	if (!ply_read(ply)) return 0;
    return ply_close(ply);
}
