#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#define sorter_header "sorter.h"
#include sorter_header
#include <ctype.h>

struct node{
    struct node *next;  // pointer points to the next row
    char **line_array;  // hold the entire row
};

typedef struct{
    int index;  // hold the original index number
    char* str;  // hold the string value
} SortArray;

char *strtok_single (char * str, char const * delims) {
    static char  * src = NULL;
    char * p,  * ret = 0;

    if (str != NULL)
        src = str;

    if (src == NULL)
        return NULL;

    if ((p = strpbrk (src, delims)) != NULL) {
        *p  = 0;
        ret = src;
        src = ++p;

    } else if (*src) {
        ret = src;
        src = NULL;
    }
    return ret;
}


/* This array contains the first row of the csv file.
It stores all the value types. */
void initValueTypesArray(char** array,int arraySize,char* line){
    char *token = strtok(line, ",");
    int counter=0;
    while (token != NULL && counter<arraySize){
        if(token[strlen(token)-1] == '\n') token[strlen(token)-1]=0;      
        array[counter]=token;
        token = strtok(NULL, ",");
        counter++;
    }
}

//global value to for column and row
int static dataRow,dataCol;
void initDataArray(char* array[dataRow][dataCol],struct node * data){
    for(int i = 0;i<dataRow;i++){
        for(int j = 0; j< dataCol ;j++){
            array[i][j]=data->line_array[j];
        }
        data = data->next;
    }
}

// check if a string is numeric, 0 -> false, non-zero -> true
int isNumeric(char* str){
    if (str == NULL || *str == '\0' || isspace(*str))
      return 0;
    char * p;
    strtod (str, &p);
    return *p == '\0';
}

int main(int argc, char** argv){
    // check for command line input
    if (argc < 3){
        printf("error");
        exit(0);
    }
    // get the sort value type
    char* sort_value_type = argv[2];

    // head of the Linked List
    struct node *head = NULL;
    struct node *prev = NULL;

    char line[1024];    // temp array for holding csv file lines.
    int rowNumber=-1;    // hold the number of lines of the csv file.
    int value_type_number;    // hold the number of value types(column numbers).
    char* headerLine;   // hold the first line of the csv file, which is the value types.
    int isFirstElement = 0; // mark the first element in LL
    while (fgets(line, 1024, stdin)){
        rowNumber++;
        
        char* tmp = strdup(line);        
        // first row
        // Returns first token 
      
      
        char *token = strtok_single(tmp, ",");
        if(rowNumber==0){
            headerLine = strdup(line);
            value_type_number = 0;
            while (token != NULL){
                if(token[strlen(token)-1] == '\n'){
                    token[strlen(token)-1]=0;//make it end of string         
                }
                token = strtok_single(NULL, ",");
                value_type_number++;    // update the number of columns(value types).
            }
            continue;
        }

        /* malloc array for holding tokens.*/
        char** new_array = malloc(value_type_number * sizeof(char*));

        // using token to split each line.
        // store each token into corresponding array cell.
        int counter = 0;
        while (token != NULL){
            if(token[strlen(token)-1] == '\n'){
                token[strlen(token)-1]=0;//make it end of string         
            }
            new_array[counter] = *token ? token : "<empty>"; // store token into array
            token = strtok_single(NULL, ",");
            counter++;
        }

        // create a new node
        // rowNumber starts from 1
        struct node *temp = (struct node*) malloc(sizeof(struct node));
        temp-> line_array = new_array;
        temp-> next = NULL;

        if (isFirstElement == 0){
            temp-> next = head;
            head = temp;
            isFirstElement = 1;
            prev = head;
            continue;
        }
        prev-> next = temp;
        prev = temp;
    }
    
    //may need to make sure attrCount are same for all string..
    //or may not....
    char* headerArray[value_type_number];   // this array hold the first row.
    initValueTypesArray(headerArray,value_type_number,headerLine);    
    

    //resuage of temp to 'copy' a head, then pass it to initDataArray to store 2d data array
    struct node *temp = (struct node*) malloc(sizeof(struct node));
    temp-> line_array = head->line_array;
    temp-> next = head -> next;
    //[row][column]
    dataCol= value_type_number;
    dataRow = rowNumber;
    char* dataArray[dataRow][dataCol];
    initDataArray(dataArray,temp);

    // free Linked List
    struct node *tmp;
    while (head != NULL){
        tmp = head;
        head = head->next;
        free(tmp);
    }

    /* sorintg */
    // first need to decide which column to sort. Do the search on headerArray.
    int i = 0;
    int isFound = 1;
    for (; i < value_type_number; i++){
        if (strcmp(headerArray[i], sort_value_type) == 0){
            isFound = 0;    // need to check if the csv file has this type.
            break;  // i is the index of the column
        }
    }
    // the type is not found in the file. ERROR.
    if (isFound == 1){
        printf("Error:The value type was not found in csv file!\n");
        exit(0);
    }

    // store the column as an array
    SortArray *sort_array;
    sort_array = (SortArray*) malloc(rowNumber * sizeof(SortArray));
    int count = 0;
    for (; count < rowNumber; count++){
        sort_array[count].index = count;
        sort_array[count].str = dataArray[count][i];
    }

    // check if the value is digits or string
    int numeric = isNumeric(sort_array[0].str);

    return 0;
}