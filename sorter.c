#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#define sorter_header "sorter.h"
#include sorter_header

void initArray(char** array,int arraySize,char* line){
    char *token = strtok(line, ",");
    int counter=0;
    while (token != NULL && counter<arraySize)
    {
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
    printf("%s\n", sort_value_type);

    char line[1024];
    int lineCounter=0;
    int attributesCount;
    char* tempLine;
    while (fgets(line, 1024, stdin))
	{
        lineCounter++;
        char* tmp = strdup(line);        
        
        // Returns first token 
        char *token = strtok(tmp, ",");
        if(lineCounter==1){
            tempLine = strdup(line);
        }
           attributesCount = 0;
        while (token != NULL)
           {
            if(token[strlen(token)-1] == '\n'){
                token[strlen(token)-1]=0;//make it end of string         
            }
            printf("%s \n", token);

            token = strtok(NULL, ",");
            attributesCount++;
           }
        //printf("%d atr\n",attributesCount);
		//printf("Field 1 would be %s\n", getfield(tmp, 1));
		// NOTE strtok clobbers tmp
        free(tmp);
        free(token);
    }
    
    //may need to make sure attrCount are same for all string..
    //or may not....
    printf("%d lines ",lineCounter);
    char* headerArray[attributesCount];
    initArray(headerArray,attributesCount,tempLine);


    printf("\n5th header is %s ",headerArray[4]);
    free(tempLine);    
    free(headerArray);
    
    return 0;
}