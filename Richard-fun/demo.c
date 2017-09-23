#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


int main(int argc, char** argv){
    
    char delims[] = ",";
    char data  [] = "foo,bar,,baz,biz";
  
    char * p    = strtok_single (data, delims);
  
    while (p) {
      printf ("%s\n", *p ? p : "<empty>");
  
      p = strtok_single (NULL, delims);
    }
    return 0;
}