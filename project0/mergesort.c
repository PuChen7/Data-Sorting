#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "sorter.h"
#define EMPTY_STRING ""

// helper function for sorting numbers
void merge(SortArray* sort_array, int left, int middle, int right,int numeric){
    int i, j, k;
    int n1 = middle - left + 1;
    int n2 =  right - middle;
    
    // temporary arrays
    SortArray *L;
    L = (SortArray*) malloc(n1 * sizeof(SortArray));
    SortArray *R;
    R = (SortArray*) malloc(n2 * sizeof(SortArray));

    // copy data into temporary arrays
    for (i = 0; i < n1; i++){
        L[i].str = sort_array[left + i].str;
        L[i].index = sort_array[left + i].index;
    }
    for (j = 0; j < n2; j++){
        R[j].str = sort_array[middle + 1+ j].str;
        R[j].index = sort_array[middle + 1 + j].index;
    }

    i = 0; 
    j = 0; 
    k = left;

    while (i < n1 && j < n2) {
        if(numeric==0){//if str
            int cmpResult = strcmp(L[i].str,R[j].str);
            if (cmpResult<=0) {
                sort_array[k].str = L[i].str;
                sort_array[k].index = L[i].index;
                i++;
            }
            else {
                sort_array[k].str = R[j].str;
                sort_array[k].index = R[j].index;
                j++;
            }
        }
        else{//if numeric
            if (atoi(L[i].str) <= atoi(R[j].str)) {
                sort_array[k].str = L[i].str;
                sort_array[k].index = L[i].index;
                i++;
            }
            else {
                sort_array[k].str = R[j].str;
                sort_array[k].index = R[j].index;
                j++;
            }
        }
        k++;
    }

    while (i < n1) {
        sort_array[k].str = L[i].str;
        sort_array[k].index = L[i].index;
        i++;
        k++;
    }

    while (j < n2) {
        sort_array[k].str = R[j].str;
        sort_array[k].index = R[j].index;
        j++;
        k++;
    }

    free(L);
    free(R);

}

// function for sorting numbers
void mergeSort(SortArray* sort_array, int left, int right,int numeric){
    if (left < right){
        int middle = left + (right - left) / 2;
        mergeSort(sort_array, left, middle,numeric);
        mergeSort(sort_array, middle+1, right,numeric);
        merge(sort_array, left, middle, right,numeric);
    }
} 