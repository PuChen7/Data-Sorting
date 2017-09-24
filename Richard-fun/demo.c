#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


int main(int argc, char** argv){
    
    char* array[2][4] = {{"abc","aba","bbc","dd"},{"abc","aba","bbc","dd"}};

    array[0][0] = "aba";

    printf("%d",strcmp(array[0][0],array[1][1]));


    return 0;
}