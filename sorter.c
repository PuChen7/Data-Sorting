#include <stdio.h>
#include<stdlib.h>

int main(int argc, char** argv){
    // check for command line input
    if (argc < 3){
        printf("error");
        exit(0);
    }
    // get the sort value type
    char* sort_value_type = argv[2];
    printf("%s", sort_value_type);

    
    return 0;
}