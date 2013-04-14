#ifndef __arrayList_h_
#define __arrayList_h_
/* Library for dynamic array handling
 * 
 */


/* Linked List structure holding data of type float
 * the structures are publicly visible 
 */
typedef struct arrayListf_t{
	int lenght;
	int size;
	int index;
	float *data;
}arrayListf;
/* Linked List structure holding data of type int
 */
typedef struct arrayListui_t{
	int lenght;
	int size;
	int index;
	unsigned int *data;
}arrayListui;
/* Creates a array list of floats
 * and returns a pointer to the list
 */
arrayListf * createArrayListf(void);
/* Adds a single value to the list of type float
 * returns 1 on sucsess and 0 on failure
 */
int addToArrayListf(arrayListf *list, float vector);
/* Adds an array of values of type float with a given size to the arraylist
 * returns 1 on sucsess 0 failure
 */
int addToArrayListfv(arrayListf *list, float *vector, int size);
/* Deletes the list and frees the allocated memmory 
 */
void deleteArrayListf(arrayListf *list);

/* Functions for the arraylist contaning values in the type of unsigned int
 * for information reffer to the float part discription
 */
arrayListui * createArrayListui(void);
int addToArrayListui(arrayListui *list, unsigned int data);
int addToArrayListuiv(arrayListui *list, unsigned int *vector, int size);
void deleteArrayListui(arrayListui *list);

#endif