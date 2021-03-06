#include <stdlib.h>
#include <string.h>
#include "arrayList.h"

#define INITDATASIZE 1024

static int growArrayListf(arrayListf* list){
	list->data = (float*)realloc(list->data, sizeof(float) * (list->size * 2));
	if(list->data == NULL)
		return 0;
	list->size *= 2;
	return 1;
}

int addToArrayListf(arrayListf* list, float data){
	if(list->size <= list->lenght){
		if(!growArrayListf(list))
			return 0;
		
	}
	list->data[list->lenght++] = data;
	return 1;
}
int addToArrayListfv(arrayListf* list, float* vector, int size){
	if(list->size <= list->lenght + size){
		if(!growArrayListf(list))
			return 0;
	}
	memcpy(list->data + list->lenght, vector, sizeof(float) * size);
	list->lenght += size;
	return 1;
}

arrayListf* createArrayListf(void){
	arrayListf *newList;
	newList = (arrayListf*)malloc(sizeof(arrayListf));
	if(newList == NULL)
		return NULL;
	newList->data = (float*)malloc(sizeof(float) * INITDATASIZE);
	if(newList->data == NULL)
		return NULL;
	newList->size = INITDATASIZE;
	newList->index = 0;
	newList->lenght = 0;
	return newList;
}

void deleteArrayListf(arrayListf *list){
	if(list->data != NULL)
		free(list->data);
	free(list);
}

/* Unsigned integer functions */
static int growArrayListui(arrayListui* list){
	list->data = (unsigned int*)realloc(list->data, sizeof(unsigned int) * list->size * 2);
	if(list->data == NULL)
		return 0;
	list->size *= 2;
	return 1;
}

int addToArrayListui(arrayListui* list, unsigned int data){
	if(list->size <= list->lenght){
		if(!growArrayListui(list))
			return 0;
	}
	list->data[list->lenght++] = data;
	return 1;
}
int addToArrayListuiv(arrayListui* list, unsigned int* vector, int size){
	if(list->size <= list->lenght + size){
		if(!growArrayListui(list))
			return 0;
	}
	memcpy(list->data + list->lenght, vector, sizeof(unsigned int) * size);
	list->lenght += size;
	return 1;
}

arrayListui* createArrayListui(void){
	arrayListui *newList;
	newList = (arrayListui*)malloc(sizeof(arrayListui));
	if(newList == NULL)
		return NULL;
	newList->data = (unsigned int*)malloc(sizeof(int) * INITDATASIZE);
	if(newList->data == NULL)
		return NULL;
	newList->size = INITDATASIZE;
	newList->index = 0;
	newList->lenght = 0;
	return newList;
}

int hasNextArrayListui(arrayListui* list){
	return (list->index < list->lenght);
}

unsigned int* getNextArrayListui(arrayListui* list){
	return list->data + (list->index++);
}

void deleteArrayListui(arrayListui* list){
	if(list->data != NULL)
		free(list->data);
	free(list);
}
