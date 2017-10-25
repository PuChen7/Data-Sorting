#include<stdio.h>
#include<sys/types.h>
#include<unistd.h>
 

int main() {
    extern static int x = 1; 
    // make two process which run same
    // program after this instruction
    if(fork()==0){//child
        x++;
        printf("%d\n",x);
    }
    else{//parent   
        x--;
        printf("%d\n",x);
    }
 
    printf("%d\n",x);
    return 0;
}