/*
    C ECHO client example using sockets
*/
#include<stdio.h> //printf
#include<string.h>    //strlen
#include<sys/socket.h>    //socket
#include<arpa/inet.h> //inet_addr
//new
#include <netdb.h>
#include <stdio.h>
#include <sys/time.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "mergesort.c"
#include <ctype.h>
#include <unistd.h>
#include <dirent.h>
#include <locale.h>
#include <ftw.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <pthread.h>
#define EMPTY_STRING ""
#define VALID_USAGE   "invalid argument numbers\ncorrect usage :./sample -c <column> (other args are optional after this)-d<directory to start> -o<outputdirectory>"
#define VALID_MOVIE_HEADER_NUMBER 28
//socket global
int sock;
//session global
int sessionID;
//socket pool
int poolSize=0;
int* socketpool;
//global
char* sort_value_type;
pthread_mutex_t path_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t threadlock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t csv_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t count_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sort_lock = PTHREAD_MUTEX_INITIALIZER;

char **final_sorted[50000];
char thread_path[1000][300];
int sentCounter=0;

char *strtok_single (char * str, char const * delims) {
    static char  * src = NULL;
    char * p,  * ret = 0;

    if (str != NULL)
        src = str;

    if (src == NULL)
        return NULL;

    if ((p = strpbrk (src, delims)) != NULL) {
        *p  = 0;
        ret = src;
        src = ++p;

    } else if (*src) {
        ret = src;
        src = NULL;
    }
    return ret;
}
int available_socket(){
  int start = 0;
  int writeVal = 0;
  for(;start<poolSize;start++){
    writeVal=write(socketpool[start] , "" , strlen(""));
    if(writeVal>=0)return socketpool[start];
  }
  //need tell the thread to sleep
  //wake it up until there is available one
}

void *send_request(char* send_file_path)
{
    pthread_mutex_lock(&sort_lock);
    char* tmp_path=strdup(send_file_path);

    FILE    *input_file = fopen(tmp_path, "r");
    //input_file

    if (input_file == NULL){
        fprintf(stderr, "Error : Failed to open entry file - %s\n", strerror(errno));
        fclose(input_file);
        return NULL;
    }

    int currentSocket = available_socket();
    sentCounter++;
    char line[1024];    // temp array for holding csv file lines.
    char buf[1024];
    char receive[1024];
    int row = 0;
    while (fgets(line, 1024, input_file)){
        //printf("ready to sent strlen %d \n",strlen(line));
        row++;
        if(write(currentSocket, line , strlen(line) ) < 0)
        {
            puts("Send failed");
            return 1;
        }
        if(read(currentSocket, buf , 1024) < 0)
        {
            puts("recv failed");
            return 2;
        }
        //
        // strcpy(receive,buf);
        // char *p = strchr(receive, '\n');
        // if (!p) /* deal with error: / not present" */;
        // *(p+1) = 0;

    }
    pthread_mutex_unlock(&sort_lock);

    char* infoString = malloc(sizeof(SORT_REQUEST)+sizeof(sort_value_type)+sizeof(int));
    sprintf(infoString,"%d_%d-%s|%s",sessionID,row,sort_value_type,SORT_REQUEST);
    write(currentSocket, infoString , strlen(infoString));
    fclose(input_file);

    return NULL;
}

int count_header(char* input_path){
  FILE    *input_file;
  input_file = fopen(input_path, "r");
  if (input_file == NULL)
  {
      fprintf(stderr, "Error : Failed to open entry file - %s\n", strerror(errno));
      fclose(input_file);
      return 0;
  }
  int rowNumber=0;
  int value_type_number;
  char * headerLine=NULL;
  char line[1024];
  while (fgets(line, 1024, input_file)){
      char* tmp = strdup(line);
      char *token = strtok_single(tmp, ",");
      if(rowNumber==0){
          headerLine = strdup(line);
          value_type_number = 0;
          while (token != NULL){
              if(token[strlen(token)-1] == '\n'){
                int len = strlen(token);
                  token[len-1]='\0';//make it end of string
              }
              token = strtok_single(NULL, ",");
              value_type_number++;    // update the number of columns(value types).
          }
          // free(headerLine);
          // free(tmp);
          break;
      }
    }
  fclose(input_file);
  return value_type_number;
}

void *recur(void *arg_path){
    //pthread_mutex_lock(&count_lock);
    char* tmppath = arg_path;
    DIR *dir = opendir(tmppath);
    //printf("enter dir %s\n",tmppath );
    //pthread_mutex_unlock(&count_lock);

    int path_index = 0;

    struct dirent *pDirent;

    if (dir == NULL){
        printf("No such directory\n");
        return NULL;
    }

    pthread_t tidgroup [1000];
    int tidlocalindex = 0;
    while (pDirent =readdir(dir)){
        if (strcmp(pDirent->d_name, ".") == 0 || strcmp(pDirent->d_name, "..") == 0)
            continue;


        char* tmp_path=(char*)malloc(sizeof tmppath+ sizeof pDirent->d_name);
        sprintf(tmp_path, "%s/%s", tmppath, pDirent->d_name);


        if (pDirent->d_type == DT_DIR){
            pthread_create(&tidgroup[tidlocalindex++], NULL, (void *)&recur, (void *)tmp_path);
            continue;
        } else {
            // if it is a new csv, sort it.
            if(strlen(tmp_path) < 4 || strcmp(tmp_path + strlen(tmp_path) - 4,".csv") != 0
            || strstr(tmp_path,"-sorted")){//found csv
                continue;
            }
            pthread_mutex_lock(&sort_lock);
            char headerfile[200];
            strcpy(headerfile,tmp_path);
            int flag=0;
            if(count_header(headerfile)==28){
              flag =1 ;
            }
            pthread_mutex_unlock(&sort_lock);
            if(flag){
              pthread_create(&tidgroup[tidlocalindex++], NULL, (void *)&send_request, (void *)tmp_path);
            }
            continue;
        }
    }
    int i = 0;
    for(;i<tidlocalindex;i++){
      //printf("path : %s  tid : %ld\n",thread_path[path_index],tidgroup[i] );
      pthread_join(tidgroup[i],NULL);
    }
    closedir(dir);
    return NULL;
}

void ifPathCorrect(char* path){
    if (strstr(path, "//") != NULL ||(strlen(path)==1&&strstr(path, "/") != NULL) ){
        printf("Error: incorrect path!\n");
        exit(0);
    }
}

//correct parameter order : -c -h -p -s -d -o (first 4 are required)
int main(int c, char *v[]){
    // arguments less than 3, error
    if (c < 9){
        printf("Error: insufficient parameters!\n");
        exit(0);
    }
    struct dirent *pDirent;
    DIR *pDir;

    // path for output sorted csv
    char output_path[500] = ".";
    // path for starting dir
    char initial_dir[500] = ".";

    char abs_path[1024];

    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    int cwd_len = strlen(cwd);

    int isAbs = 1;

    if(c == 11){//... -d
          ifPathCorrect(v[10]);
          if (strstr(v[10], cwd) != NULL){
              memcpy(abs_path, &v[10][cwd_len+1], strlen(v[10])-cwd_len);
              abs_path[strlen(v[10]) - cwd_len] = '\0';
              isAbs = 0;
          }
          if (isAbs == 0){
              strcpy(initial_dir, abs_path);
          } else {
              strcpy(initial_dir, v[10]);
          }
    }
    else if(c == 13){//... -d -o
          ifPathCorrect(v[10]);
          ifPathCorrect(v[12]);
          if (strstr(v[10], cwd) != NULL){
              char abs_path2[1024];
              memcpy(abs_path2, &v[10][cwd_len+1], strlen(v[10])-cwd_len);
              abs_path2[strlen(v[10]) - cwd_len] = '\0';
              strcpy(initial_dir, abs_path2);
          } else {
              strcpy(initial_dir, v[10]);
          }
          if (strstr(v[12], cwd) != NULL){
              memcpy(abs_path, &v[12][cwd_len+1], strlen(v[12])-cwd_len);
              abs_path[strlen(v[12]) - cwd_len] = '\0';
              strcpy(output_path, abs_path);
          } else {
              strcpy(output_path, v[12]);
          }
    }
    //NOTE parameter printing
    // printf("-d %s -o %s\n", initial_dir,output_path);


    //NOTE v2 : sort_type
    sort_value_type=v[2];

    struct in_addr addr;
    struct hostent *hostp;
    struct sockaddr_in server;

    //NOTE v4 : hostname

    //Get Host name , either string or address
    if (inet_aton(v[4], &addr) != 0)
    hostp = gethostbyaddr((const char *)&addr, sizeof(addr), AF_INET);
    else
    hostp = gethostbyname(v[4]);

    if(hostp==NULL){
      printf("invalid ip\n" );
      exit(0);
    }

    //NOTE v6 : port number
    char **pp;
    pp = hostp->h_addr_list;
    server.sin_addr.s_addr = ((struct in_addr *)*pp)->s_addr;//default to localhost
    server.sin_family = AF_INET;
    server.sin_port = htons( atoi(v[6]) );

    //NOTE v8 : poolsize
    poolSize=atoi(v[8]);//socket pool number
    socketpool=calloc(poolSize,sizeof *socketpool);
    int init=0;
    char sessionMSG[20];
    for(;init<poolSize;init++){
      //Create socket
      socketpool[init] = socket(AF_INET , SOCK_STREAM , 0);
      if (socketpool[init] == -1)
      {
          printf("Could not create socket");
      }

      //Connect to remote server
      if (connect(socketpool[init] , (struct sockaddr *)&server , sizeof(server)) < 0)
      {
          perror("connect failed. Error");
          return 1;
      }

      if(init==0){
        read(socketpool[init],sessionMSG,20);
        *(strstr(sessionMSG,"-")+1)=0;
        sessionID=atoi(sessionMSG);
      }
    }

    char* tmpInitDir = initial_dir;
    recur((void *)tmpInitDir);

    char *cat_tmp = malloc(sizeof(char)*200);
    strcpy(cat_tmp, "/AllFiles-sorted-<");
    strcat(strcat(cat_tmp, sort_value_type),">.csv");
    strcat(output_path, cat_tmp);
    FILE *output_file = fopen(output_path, "w");
    write(socketpool[0] , DUMP_REQUEST , strlen(DUMP_REQUEST));

    int read_size;
    char server_message[2048*5];
    pthread_mutex_lock(&sort_lock);
    while( (read_size = read(socketpool[0] , server_message , 2048*5 )) > 0 ){
      printf("%s\n",server_message );
      if(strstr(server_message,"FILE_INFO")!=NULL){
        char* p = strstr(server_message,"FILE_INFO");
        *p = 0;
      }
      if(strstr(server_message,"FINISH")!=NULL){
        printf("%s\n",server_message );
        write(socketpool[0] , "FINISH" , strlen("FINISH"));
        break;
      }
      fprintf(output_file, "%s",server_message);
      write(socketpool[0] , "FINISH" , strlen("FINISH"));
    }
    fclose(output_file);
    pthread_mutex_unlock(&sort_lock);

    pthread_mutex_destroy(&path_lock);
    pthread_mutex_destroy(&threadlock);
    pthread_mutex_destroy(&csv_lock);
    pthread_mutex_destroy(&count_lock);
    init=0;
    for(;init<poolSize;init++){
      close(socketpool[init]);
      //printf("%dth th socket Disconnected\n",init);
    }
    free(cat_tmp);
    free(socketpool);
    return 0;
}
