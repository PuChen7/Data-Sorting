#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "sorter.h"
#define EMPTY_STRING ""

/* Function for locating the range of the first letter of the string
   There are 4 ranges based on ASCII values.
   range0   -->   0                            -->   NULL
   range1   -->   -1-(-128)                    -->   Negative ascii 
   range2   -->   1-47, 58-64, 91-96, 123-127  -->   Special Characters
   range3   -->   48-57                        -->   Numerical values
   range4   -->   65-90                        -->   Uppercase Letters
   range5   -->   97-122                       -->   Lowercase letters
   The order follows the above pattern.
   * Return Values: 0 for range0, 1 for range1, 2 for range2, 3 for range3, etc.
*/
int getRangeofString(char str){
    if ((int)str == 0){return 0;}
    else if ((int)str >=-128 && (int)str <= -1){return 1;}
    else if (((int)str >= 58 && (int)str <= 64) || ((int)str >= 1 && (int)str <= 47) || 
            ((int)str >= 91 && (int)str <= 96) || ((int)str >= 123 && (int)str <= 127)){return 2;}
    else if ((int)str >= 48 && (int)str <= 57){return 3;}
    else if((int)str >= 65 && (int)str <= 90){return 4;}
    else if((int)str >= 97 && (int)str <= 122){return 5;}
    else{return -1;}
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
        int cmpResult = 0;
        if(numeric!=0){ //if str
            int len1 = strlen(L[i].str);
            int len2 = strlen(R[j].str);

            if (len1 == 0 || len2 == 0){
              if (len1 == 0){cmpResult = -1;}
              else {cmpResult = 0;}
            } else {
              // trim ""
              int trim_len1 = 0;
              int trim_len2 = 0;
              char str1[len1];
              char str2[len2];
              int iterator = 0;
              for (; trim_len1<len1; trim_len1++){
                if (L[i].str[trim_len1] != '"'){
                  str1[iterator] = L[i].str[trim_len1];
                  iterator++;
                }
              }

              int str1_len1 = iterator+1;

              iterator = 0;
              for (; trim_len2<len2; trim_len2++){
                if (R[j].str[trim_len2] != '"'){
                  str2[iterator] = R[j].str[trim_len2];
                  iterator++;
                }
              }
              int str2_len2 = iterator+1;
              char* str1_ptr = str1;
              char* str2_ptr = str2;
              

              int index = 0;
              while (index<str1_len1 && index<str2_len2 && str1_ptr[0] != str2_ptr[0]){
                if ((int)str1_ptr[index] != (int)str2_ptr[index]){break;}
                index++;
                str1_len1--;
                str2_len2--;
              }
              int range_of_str1 = getRangeofString(str1_ptr[index]);
              int range_of_str2 = getRangeofString(str2_ptr[index]);
              int loopflag = 0;

              if ((range_of_str1 == 4 || range_of_str1 == 5)&&(range_of_str2 == 4 || range_of_str2 == 5)){
                loopflag = -1;
                int str1_char = (int)str1_ptr[index], str2_char = (int)str2_ptr[index];
                if (range_of_str1 == 5){str1_char = (int)str1_ptr[index]-32;}
                if (range_of_str2 == 5){str2_char = (int)str2_ptr[index]-32;}

                if (str1_char == str2_char){  // check if the original are equal
                  if ((int)str1_ptr[index] != (int)str2_ptr[index]){  // the original is NOT equal
                    if ((int)str1_ptr[index] - (int)str2_ptr[index] > 0){cmpResult=0;}
                    else{cmpResult=-1;}
                  }
                } else {
                  if (str1_char < str2_char){cmpResult = -1;}   // str1 < str2
                  else{cmpResult=0;}
                }
              }

              // if str1 and str2 are in the SAME range, then determine based on own values
              if (range_of_str1 == range_of_str2 && loopflag == 0){
                  
                  if ((int)str1_ptr[index] <= (int)str2_ptr[index]){
                    
                    cmpResult = -1;   // * -1 indicates the str1 is less than or equal to str2 (<=)
                  }
              } else {
                  //printf("str1 %s %d, str2 %s %d\n", L[i].str, range_of_str1, R[j].str, range_of_str2);
                  if ((range_of_str1 < range_of_str2) && loopflag == 0){
                    cmpResult = -1;
                  }
              }
            }

            //int cmpResult = strcmp(L[i].str,R[j].str);
            if (cmpResult == -1) {
                sort_array[k].str = strdup(L[i].str);
                sort_array[k].index = L[i].index;
                i++;
            }
            else {
                sort_array[k].str = strdup(R[j].str);
                sort_array[k].index = R[j].index;
                j++;
            }
        }
        else{//if numeric
            if (atoi(L[i].str) <= atoi(R[j].str)) {
                sort_array[k].str = strdup(L[i].str);
                sort_array[k].index = L[i].index;
                i++;
            }
            else {
                sort_array[k].str = strdup(R[j].str);
                sort_array[k].index = R[j].index;
                j++;
            }
        }
        k++;
    }

    while (i < n1) {
        sort_array[k].str = strdup(L[i].str);
        sort_array[k].index = L[i].index;
        i++;
        k++;
    }

    while (j < n2) {
        sort_array[k].str = strdup(R[j].str);
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