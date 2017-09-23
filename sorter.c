#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#define sorter_header "sorter.h"
#include sorter_header

struct node{
    int marker; // hold the index number of the row
    struct node *next;  // pointer points to the next row
    char **line_array;  // hold the entire row
};

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
    int rowNumber=0;    // hold the number of lines of the csv file.
    int value_type_number;    // hold the number of value types(column numbers).
    char* headerLine;   // hold the first line of the csv file, which is the value types.
    int isFirstElement = 0; // mark the first element in LL
    while (fgets(line, 1024, stdin)){
        rowNumber++;
        char* tmp = strdup(line);        
        // first row
        // Returns first token 
        char *token = strtok(tmp, ",");
        if(rowNumber==1){
            headerLine = strdup(line);
            value_type_number = 0;
            while (token != NULL){
                if(token[strlen(token)-1] == '\n'){
                    token[strlen(token)-1]=0;//make it end of string         
                }
                token = strtok(NULL, ",");
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
            token = strtok(NULL, ",");
            new_array[counter] = token; // store token into array
            counter++;
        }


        // create a new node
        struct node *temp = (struct node*) malloc(sizeof(struct node));
        temp-> marker = rowNumber;
        temp-> line_array = new_array;
        temp-> next = NULL;

        //printf("%s\n", temp-> line_array[0]);

        if (isFirstElement == 0){
            temp-> next = head;
            head = temp;
            isFirstElement = 1;
            prev = head;
            continue;
        }
        prev-> next = temp;
        prev = temp;
        
        //printf("%s \n",prev-> line_array);
		//printf("Field 1 would be %s\n", getfield(tmp, 1));
		// NOTE strtok clobbers tmp
    }
    
    //may need to make sure attrCount are same for all string..
    //or may not....
    char* headerArray[value_type_number];
    initValueTypesArray(headerArray,value_type_number,headerLine);    
    
    // free Linked List
    struct node *tmp;
    while (head != NULL){
        tmp = head;
        head = head->next;
        free(tmp);
    }

    return 0;
}