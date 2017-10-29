#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "sorter.h"
#define EMPTY_STRING ""
#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

int weird_char_check(char src){//return 1 means found weird character
  if((src>=65&&src<=90)||(src>=97&&src<=122)||src==34||src==44)//double quotes and coma and other alphabeta characters
    return 0;
  else
    return 1;
}
char* double_quote_elimination(char* src){
    if(src[0]=='"'){
      char* temp = (char*)malloc(strlen(src)-1);
      int c = 0;
      for(;c<strlen(src)-2;c++){
        *(temp+c)=*(src+c+1);
      }
      *(temp+c+1)='\0';
      return temp;
    }
    else return src;
}
int hyper_strcmp(char* str1,char* str2){
    char* agentStr1 = (char*)malloc(strlen(str1)+1);
    char* agentStr2 = (char*)malloc(strlen(str2)+1);
    strcpy(agentStr1,double_quote_elimination(str1));
    strcpy(agentStr2,double_quote_elimination(str2));


    int init_counter = 0;
    for(;init_counter<max(strlen(agentStr1),strlen(agentStr2));init_counter++){
      char a = *(agentStr1+init_counter);
      char b = *(agentStr2+init_counter);
      int weird_a = weird_char_check(a);//1 weird ,0 not
      int weird_b = weird_char_check(b);
      if(weird_a&&weird_b){
        if(a>b) return 1;
        else if(a<b)return -1;
        else continue;
      }else if(weird_a){
        return -1;
      }else if(weird_b){
        return 1;
      }else{//both are regular character
        char tempA = a;
        char tempB = b;
        //convert into uppercase then compare
        if(tempA==tempB)continue;
        if(tempA>=97)tempA-=32;
        if(tempB>=97)tempB-=32;


        //after convert into uppercase,if same,then if a is lowercase before,b must be uppercase
        //vice versa
        if(tempA==tempB)return a>=97?1:-1;
        else if(tempA>tempB)return 1;
        else return -1;
      }
    }
    if((strlen(agentStr1)-init_counter)>1) return 1;
    else if((strlen(agentStr2)-init_counter)>1) return -1;
    else return 0;

}
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
            int cmpResult = hyper_strcmp(L[i].str,R[j].str);
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
