#include <stdio.h>
#include<stdlib.h>
#include <string.h>

int main(int argc, char** argv){
    // check for command line input
    if (argc < 3){
        printf("error");
        exit(0);
    }
    // get the sort value type
    char* sort_value_type = argv[2];

    char line[1024];
    int isFirstRow = 0; // flag for marking the first row
    int columnNumber = 0;   // holding the number of columns
    while (fgets(line, 1024, stdin)){
        char* str = strdup(line);   // read file line by line
        // get the column number
        if (isFirstRow == 0){
            int i = 0;
            for (; i < strlen(str); i++){
                if (str[i] == ','){
                    columnNumber++;
                }
            }
            columnNumber += 1;
            isFirstRow = 1;
        }
    }
    
    return 0;
}