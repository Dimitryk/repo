#include <stdlib.h>
#include <string.h>
#include "arrayList.h"

#define INITDATASIZE 1024

static int growArrayListf(arrayListf *list){
	list->data = (float*)realloc(list->data, sizeof(float) * (list->size * 2));
	if(list->data == NULL)
		return 0;
	list->size *= 2;
	return 1;
}

int addToArrayListf(arrayListf *list, float data){
	if(list->index >= list->size){
		if(!growArrayListf(list))
			return 0;
		
	}
	list->data[list->index] = data;
	list->index++;
	list->lenght++;
	return 1;
}
int addToArrayListfv(arrayListf *list, float *vector, int size){
	if(list->size <= list->index + size){
		if(!growArrayListf(list))
			return 0;
		
	}
	memcpy(list->data + list->index, vector, sizeof(float) * size);
	list->index += size;
	list->lenght += size;
	return 1;
}

arrayListf * createArrayListf(void){
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
}

void deleteArrayListf(arrayListf *list){
	free(list->data);
	free(list);
}

static int growArrayListui(arrayListui *list){
	list->data = (unsigned int*)realloc(list->data, sizeof(unsigned int) * list->size * 2);
	if(list->data == NULL)
		return 0;
	list->size *= 2;
	return 1;
}

int addToArrayListui(arrayListui *list, int data){
	if(list->index >= list->size){
		if(!growArrayListui(list))
			return 0;
		
	}
	list->data[list->index] = data;
	list->index++;
	list->lenght++;
	return 1;
}
int addToArrayListuiv(arrayListui *list, unsigned int *vector, int size){
	if(list->size <= list->index + size){
		if(!growArrayListui(list))
			return 0;
		
	}
	memcpy(list->data + list->index, vector, sizeof(unsigned int) * size);
	list->index += size;
	list->lenght += size;
	return 1;
}

arrayListui * createArrayListui(void){
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
}

void deleteArrayListui(arrayListui *list){
	free(list->data);
	free(list);
}
