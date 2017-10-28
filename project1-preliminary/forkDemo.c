#include<stdio.h>
#include<sys/types.h>
#include<unistd.h>

static int x =1;
int main() {
    // make two process which run same
    // program after this instruction
    pid_t init_pid;
    pid_t pid = fork();
    init_pid= pid;
    for(int i=1;i<2;i++){
      pid = fork();
    }
    if(pid==0){//child
        x++;
        //printf("child x%d\n",x);
    }
    else{//parent
        x--;
        //printf("parent %d\n",x);
    }
    printf("init pid %d\n",init_pid);
    printf("main %d\n",pid);
    return 0;
}
