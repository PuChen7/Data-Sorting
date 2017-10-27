#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>
#include <ftw.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/types.h>

static int *stdoutFlag;
static int *pCounter;
static pid_t *pidArray;
static int *pidArrayCursor;
static int *fileArrayCursor;
static char **fileArray;
pid_t init_pid;


const char *get_filename_ext(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot + 1;
}

int indexOfPidArray(pid_t pid){
    int i=0;
    for(;i<256;i++){
      if(pidArray[i]==pid)break;
    }
    return i;
}

void recur(DIR *pDir, struct dirent *pDirent, char* path){
    pid_t pid,fpid=getppid();


    int csvCounter=0;

    while ((pDirent = readdir(pDir)) != NULL) {
        if (pDirent->d_name[0] == '.'){continue;}
        //printf("scanning file %s  with pid [%d] +[%d]|", pDirent->d_name,getpid(),*pidArrayCursor);
        if(strchr(pDirent->d_name, '.') != NULL ){
          if(strcmp("csv",get_filename_ext(pDirent->d_name))==0){
            fpid = fork();
            if(fpid<0){}
            else if(fpid==0){//child

              //printf("found file %s with pid [%d] [%d] parent[%d] \n", pDirent->d_name,getpid(),pid,getppid());
              // !important printf
              int index = indexOfPidArray(getpid());
              printf("%d,fileArray[%d]:%s\n",getpid(),index,fileArray[index]);

              exit(0);
            }
            else{//parent
              pidArray[*pidArrayCursor]=fpid;
              *pidArrayCursor=*pidArrayCursor+1;
              fileArray[*fileArrayCursor]=pDirent->d_name;
              *fileArrayCursor=*fileArrayCursor+1;
              // int counter=0;
              // for(;counter<*fileArrayCursor;counter++){
              //   printf("%d \n",pidArray[counter]);
              // }
              // for(counter=0;counter<*fileArrayCursor;counter++){
              //   printf("file:%s\n",fileArray[counter] );
              // }
              continue;
            }

              // if(getpid()==init_pid+*fileArrayCursor){
              //   printf("found file %s with pid [%d] [%d] parent[%d]", pDirent->d_name,getpid(),pid,getppid());
              //   fileArray[*fileArrayCursor]=pDirent->d_name;
              //   printf("current cursor [%d]\n",*fileArrayCursor);
              //   *fileArrayCursor=*fileArrayCursor+1;
              // }

          }
      }

        if (strchr(pDirent->d_name, '.') == NULL){//search for folder
          pid = fork();
            if (pid < 0){
                printf("error\n");
                exit(0);
            } else if (pid == 0){   // child,aka entered directory

                // printf("enter directory %s with pid[%d]\n", pDirent->d_name,getpid()); !important printf


                strcat(path, "/");
                DIR *newdir = opendir(strcat(path,pDirent->d_name));

                recur(newdir, pDirent, path);

                break;
            } else if (pid > 0){    // parent ,aka found the directory

                //printf("found directory %s with pid[%d]\n", pDirent->d_name,getpid()); !important printf
            }
        }
    }
    closedir (pDir);

}


int main(int c, char *v[]){
    struct dirent *pDirent;
    DIR *pDir;
    char cwd[1024];
    pCounter = mmap(NULL, sizeof *pCounter, PROT_READ | PROT_WRITE,
                        MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *pCounter = 0;

    stdoutFlag = mmap(NULL, sizeof *stdoutFlag, PROT_READ | PROT_WRITE,
                        MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *stdoutFlag = 0;

    pidArray = mmap(NULL, sizeof *pidArray * 256, PROT_READ | PROT_WRITE,
                        MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    fileArray = mmap(NULL, sizeof *fileArray * 256, PROT_READ | PROT_WRITE,
                        MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    pidArrayCursor= mmap(NULL, sizeof *pidArrayCursor, PROT_READ | PROT_WRITE,
                        MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *pidArrayCursor=0;

    fileArrayCursor= mmap(NULL, sizeof *fileArrayCursor, PROT_READ | PROT_WRITE,
                        MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *fileArrayCursor=0;

    if(c==1){
      if (getcwd(cwd, sizeof(cwd)) != NULL){
      //fprintf(stdout, "Current working dir: %s\n", cwd);
      }
      else
      perror("getcwd() error");
    }
    /*
    here should code scanning current directory that is
    when -d is optional
    */
    pDir = opendir (v[1]);
    char path[200];
    strncpy(path, v[1], strlen(v[1]));
    char* ptr = path;
    printf("Initial PID : %d\n", (init_pid=getpid()));

    /*
    This header will need more inspection
    // if(*stdoutFlag==0){
    //   *stdoutFlag=1;
    //   printf("[%d] Child Processes: ",*stdoutFlag);
    // }
    */
    recur(pDir, pDirent, ptr);


    int status = 0;
    pid_t wpid;
    while ((wpid = wait(&status)) > 0)
    {
        printf("%d,",wpid);
        *pCounter+=1;
    }


    if(getpid()==init_pid){
      wait(NULL);
      printf("\nTotal Child Processes Number: %d \n",*pCounter);
      int counter=0;
      // for(;counter<2;counter++){
      //   printf("%d \n",pidArray[counter]);
      // }
      // for(counter=0;counter<6;counter++){
      //   printf("file:%s\n",fileArray[counter] );
      // }

      munmap(pCounter, sizeof *pCounter);
      munmap(stdoutFlag, sizeof *stdoutFlag);
      munmap(pidArray, sizeof *pidArray);
      munmap(pidArrayCursor, sizeof *pidArrayCursor);
      munmap(fileArray, sizeof *fileArray);
      munmap(fileArrayCursor, sizeof *fileArrayCursor);
    }

    return 0;
}
