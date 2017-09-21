#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

const char* getfield(char* line, int num)
{
	const char* tok;
	for (tok = strtok(line, ",");
			tok && *tok;
			tok = strtok(NULL, ",\n"))
	{
		if (!--num)
			return tok;
	}
	return NULL;
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
    while (fgets(line, 1024, stdin))
	{
        lineCounter++;
        char* tmp = strdup(line);        
        
        // Returns first token 
        char *token = strtok(tmp, ",");
          
           // Keep printing tokens while one of the
           // delimiters present in str[].
        while (token != NULL)
           {
               printf("%s\n", token);
               token = strtok(NULL, ",");
           }

		//printf("Field 1 would be %s\n", getfield(tmp, 1));
		// NOTE strtok clobbers tmp
        free(tmp);
        free(token);
    }
    printf("%d lines",lineCounter);
    return 0;
}