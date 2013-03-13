
#ifndef __randomHelper_h_
#define __randomHelper_h_
#include <math.h>
#include <stdlib.h>
#include <time.h>

int randomNumber(int limit)
{
  return rand()%limit;
}
float randomFloat()
{
  return ((float)rand())/RAND_MAX;
}

float randomInLimitf(float lower, float upper){
	if(lower > upper){
		float temp = lower;
		lower = upper;
		upper = temp;
	}
	return lower + (upper-lower)*(((float)rand())/RAND_MAX);
}

void seedRandomGen(){
  unsigned int iseed = (unsigned int)time(NULL);
  srand (iseed);
}

#endif