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
#include <errno.h>

static int *stdoutFlag;
static int *pCounter;

pid_t init_pid;


const char *get_filename_ext(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot + 1;
}

char *remove_ext(char* mystr) {
    char *retstr;
    char *lastdot;
    if (mystr == NULL)
         return NULL;
    if ((retstr = malloc (strlen (mystr) + 1)) == NULL)
        return NULL;
    strcpy (retstr, mystr);
    lastdot = strrchr (retstr, '.');
    if (lastdot != NULL)
        *lastdot = '\0';
    return retstr;
}


void recur(DIR *pDir, struct dirent *pDirent, char* path){
    pid_t pid,fpid=getppid();
    FILE    *input_file;
    FILE    *output_file;
    char line[1024];    // temp array for holding csv file lines.


    while ((pDirent = readdir(pDir)) != NULL) {
        if (pDirent->d_name[0] == '.'){continue;}
        //printf("scanning file %s  with pid [%d] +[%d]|", pDirent->d_name,getpid(),*pidArrayCursor);
        if(strchr(pDirent->d_name, '.') != NULL ){
          if(strcmp("csv",get_filename_ext(pDirent->d_name))==0){//found csv
            fpid = fork();
            if(fpid<0){}
            else if(fpid==0){//child

              char * outputPath = strcat(strcat(remove_ext(pDirent->d_name),"-out"),".csv");
              //printf("outpath: %s\n",outputPath);
              /* Openiing common file for writing */
              output_file = fopen(outputPath, "w");
               if (output_file == NULL)
               {
                    fprintf(stderr, "Error : Failed to open output_file - %s\n", strerror(errno));
                    fclose(output_file);
                    exit(0);
               }
              char* currentDir = strcat(strcat(path,"/"),pDirent->d_name);
              printf("%s\n",currentDir);
              input_file = fopen(currentDir, "r");
              if (input_file == NULL)
              {
                  fprintf(stderr, "Error : Failed to open entry file - %s\n", strerror(errno));
                  fclose(input_file);
                  exit(0);
              }

              /* Doing some struf with input_file : */
              /* For example use fgets */
              while (fgets(line, 1024, input_file))
              {
                  printf("%s",line);
                  /* Use fprintf or fwrite to write some stuff into output_file*/
              }

              /* When you finish with the file, close it */
              fclose(input_file);
              fclose(output_file);
              //printf("found file %s with pid [%d] [%d] parent[%d] \n", pDirent->d_name,getpid(),pid,getppid());
              // !important printf
              printf("[%d],%s\n",getpid(),pDirent->d_name);
              sleep(getpid()-getppid());
              exit(0);
            }
            else{//parent

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
      // munmap(pidArray, sizeof *pidArray);
      // munmap(pidArrayCursor, sizeof *pidArrayCursor);
      // munmap(fileArray, sizeof *fileArray);
      // munmap(fileArrayCursor, sizeof *fileArrayCursor);
    }

    return 0;
}
