/*****
*
*	Define structures and function prototypes for your sorter
*
*
*
******/



//Suggestion: define a struct that mirrors a record (row) of the data set
/*typedef struct _node {
    char *name;
    char *desc;
    struct _node *next;
} node;
*/
struct node{
    struct node *next;  // pointer points to the next row
    char **line_array;  // hold the entire row
};

typedef struct{
    int index;  // hold the original index number
    char** str;  // hold the string value
} SortArray;

// //Suggestion: prototype a mergesort function
// #include "mergesort.c"


/* global variables */
int dataRow,dataCol;

/************************/
void *connection_handler(void *);
char *strtok_single (char * str, char const * delims);
void initValueTypesArray(char** array,int arraySize,char* line);
void initDataArray(char* array[dataRow][dataCol],struct node * data);
int isNumeric(char* str);
char *trimwhitespace(char *str);
void merge(SortArray* sort_array, int left, int middle, int right,int numeric);
void mergeSort(SortArray* sort_array, int left, int right,int numeric);
