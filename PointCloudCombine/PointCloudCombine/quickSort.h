#ifndef __quickSort_h_
#define __quickSort_h_

/* Extracted from http://rosettacode.org minor changes to types and names
 * 
 */

void quickSortf(float *list, int n){
    if (n < 2)
        return;
    float p = list[n / 2];
    float *l = list;
    float *r = list + n - 1;
    while (l <= r) {
        if (*l < p) {
            l++;
            continue;
        }
        if (*r > p) {
            r--;
            continue; // we need to check the condition (l <= r) every time we change the value of l or r
        }
        float t = *l;
        *l++ = *r;
        *r-- = t;
    }
    quickSortf(list, r - list + 1);
    quickSortf(l, list + n - l);
}

void quickSorti(int *list, int n){
    if (n < 2)
        return;
    int p = list[n / 2];
    int *l = list;
    int *r = list + n - 1;
    while (l <= r) {
        if (*l < p) {
            l++;
            continue;
        }
        if (*r > p) {
            r--;
            continue; // we need to check the condition (l <= r) every time we change the value of l or r
        }
        int t = *l;
        *l++ = *r;
        *r-- = t;
    }
    quickSorti(list, r - list + 1);
    quickSorti(l, list + n - l);
}


#endif