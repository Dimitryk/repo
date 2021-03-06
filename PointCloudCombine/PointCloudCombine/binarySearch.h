#ifndef __binarySearch_h_
#define __binarySearch_h_

/* Pseudo code from wikipedia.org
 * Rewritten to C
 */
int binarySearchf(float value, float* list, int size){
	float *midPntr;
	float *startPntr = list;
	float *endPntr = list + size;
	while(startPntr < endPntr){
		size /= 2; 
		midPntr = startPntr + size;
		if (*midPntr < value)
			startPntr = midPntr + 1;
		else
			endPntr = midPntr;
	}
	if ((endPntr == startPntr) && (*endPntr == value))
		return endPntr - list;
	return -1;
}

int binarySearchui(unsigned int value, unsigned int* list, int size, int* position){
	unsigned int *midPntr;
	unsigned int *startPntr = list; 
	unsigned int *endPntr = list + size;
	while(startPntr < endPntr){
		size /= 2; 
		midPntr = startPntr + size;
		if (*midPntr < value)
			startPntr = midPntr + 1;
		else
			endPntr = midPntr;
	}
	if ((endPntr == startPntr) && (*endPntr == value))
		return endPntr - list;
	*position = endPntr - list;
	return -1;
}


#endif