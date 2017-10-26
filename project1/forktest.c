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

static int *pCounter;
pid_t init_pid;

void recur(DIR *pDir, struct dirent *pDirent, char* path){
    pid_t pid,wpid;
    int status = 0;
    /*
    if (pDir == NULL) {
        printf ("Cannot open directory '%s'\n", v[1]);
        return 1;
    }*/
    while ((pDirent = readdir(pDir)) != NULL) {
        if (pDirent->d_name[0] == '.'){continue;}
        //printf("scanning file %s\n", pDirent->d_name);
        if (strchr(pDirent->d_name, '.') == NULL){//search for folder
            pid = fork();
            if (pid < 0){
                printf("error\n");
                exit(0);
            } else if (pid == 0){   // child,aka entered directory
                //printf("enter directory %s\n", pDirent->d_name);
                strcat(path, "/");
                DIR *newdir = opendir(strcat(path,pDirent->d_name));
                recur(newdir, pDirent, path);
                /*if ((pDirent = readdir(newdir)) != NULL){
                    printf("KKKK");
                }*/
            } else if (pid > 0){    // parent ,aka found the directory
                //printf("found directory %s\n", pDirent->d_name);
            }
        }
    }
    closedir (pDir);


    while ((wpid = wait(&status)) > 0)
    {
        if(*pCounter==0)printf("Child Processes: ");
        printf("%d,",wpid);
        *pCounter+=1;
        // printf("Exit status of %d was %d (%s)\n", (int)wpid, status,
        //        (status > 0) ? "accept" : "reject");
    }

    if(getpid()==init_pid){
      wait(NULL);
      printf("\nTotal Child Processes Number: %d \n",*pCounter);
      munmap(pCounter, sizeof *pCounter);
    }


}

int main(int c, char *v[]){
    struct dirent *pDirent;
    DIR *pDir;
    char cwd[1024];
    pCounter = mmap(NULL, sizeof *pCounter, PROT_READ | PROT_WRITE,
                        MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *pCounter = 0;

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

    recur(pDir, pDirent, ptr);

    return 0;
}
