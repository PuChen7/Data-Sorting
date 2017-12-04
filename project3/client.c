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

void *send_request(char* send_file_path)
{

    //printf("sent %s \n\n",message);

    pthread_mutex_lock(&sort_lock);
    char* tmp_path=strdup(send_file_path);

    //output_path = tmp_path;
    FILE    *input_file = fopen(tmp_path, "r");
    //input_file

    if (input_file == NULL){
        fprintf(stderr, "Error : Failed to open entry file - %s\n", strerror(errno));
        fclose(input_file);
        return NULL;
    }
    sentCounter++;

    char line[1024];    // temp array for holding csv file lines.
    while (fgets(line, 1024, input_file)){
        if(send(sock , line , strlen(line) , 0) < 0)
        {
            puts("Send failed");
            return 1;
        }

        if(recv(sock , line , 2000, 0) < 0)
        {
            puts("recv failed");
            return 2;
        }

        puts("Server replies :");
        puts(line);
    }

    pthread_mutex_unlock(&sort_lock);
    fclose(input_file);
    free(tmp_path);
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
          free(headerLine);
          free(tmp);
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
    //free(tmppath);
    //return NULL;
}

void ifPathCorrect(char* path){
    if (strstr(path, "//") != NULL){
        printf("Error: incorrect path!\n");
        exit(0);
    }
}


//correct parameter order : -c -h -p -d -o (first 3 are required)
int main(int c, char *v[]){
    struct in_addr addr;
    struct hostent *hostp;
    struct sockaddr_in server;
    char *server_reply;

    //Get Host name , either string or address
    if (inet_aton(v[4], &addr) != 0)
    hostp = gethostbyaddr((const char *)&addr, sizeof(addr), AF_INET);
    else
    hostp = gethostbyname(v[4]);


    //Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");

    char **pp;
    pp = hostp->h_addr_list;
    server.sin_addr.s_addr = ((struct in_addr *)*pp)->s_addr;//default to localhost
    server.sin_family = AF_INET;
    server.sin_port = htons( atoi(v[6]) );

    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        return 1;
    }

    puts("Connected\n");
    server_reply = malloc(2000);
    if( recv(sock , server_reply , 2000 , 0) < 0)
    {
        puts("recv failed");
    }
    puts("Server Head1 :");
    puts(server_reply);
    free(server_reply);


    //keep communicating with server

    //final_sorted = (*)malloc(2048*5050*sizeof(SortArray));
    // int i;
    // i = 0;
    // for(;i<50000;i++){
    //     final_sorted[i]= malloc(sizeof (char*) * 28);
    // }

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


    // // arguments less than 3, error
    // if (c < 3){
    //     printf("Error: insufficient parameters!\n");
    //     exit(0);
    // }
    //
    // if (c == 3){
    //   sort_value_type=v[2];
    // }   // 2 parameters: -c -d | -c -o | -d -c | -o -c
    // else if (c == 5){
    //     if (strstr(v[4], cwd) != NULL){
    //         memcpy(abs_path, &v[4][cwd_len+1], strlen(v[4])-cwd_len);
    //         abs_path[strlen(v[4]) - cwd_len] = '\0';
    //         isAbs = 0;
    //     }
    //     ifPathCorrect(v[4]);
    //
    //     // -c -d
    //     if (strcmp(v[1], "-c") == 0 && strcmp(v[3], "-d") == 0){
    //         sort_value_type = v[2];
    //         if (isAbs == 0){
    //             strcpy(initial_dir, abs_path);
    //         } else {
    //             strcpy(initial_dir, v[4]);
    //         }
    //     // -c -o
    //     } else if (strcmp(v[1], "-c") == 0 && strcmp(v[3], "-o") == 0){
    //         sort_value_type = v[2];
    //         if (isAbs == 0){
    //             strcpy(output_path, abs_path);
    //         } else {
    //             strcpy(output_path, v[4]);
    //         }
    //     // -d -c
    //     } else if (strcmp(v[1], "-d") == 0 && strcmp(v[3], "-c") == 0) {
    //         sort_value_type = v[4];
    //         if (strstr(v[2], cwd) != NULL){
    //             memcpy(abs_path, &v[2][cwd_len+1], strlen(v[2])-cwd_len);
    //             abs_path[strlen(v[2]) - cwd_len] = '\0';
    //             strcpy(initial_dir, abs_path);
    //         } else {
    //             strcpy(initial_dir, v[2]);
    //         }
    //     // -o -c
    //     } else if (strcmp(v[1], "-o") == 0 && strcmp(v[3], "-c") == 0){
    //         sort_value_type = v[4];
    //         if (strstr(v[2], cwd) != NULL){
    //             memcpy(abs_path, &v[2][cwd_len+1], strlen(v[2])-cwd_len);
    //             abs_path[strlen(v[2]) - cwd_len] = '\0';
    //             strcpy(output_path, abs_path);
    //         } else {
    //             strcpy(output_path, v[2]);
    //         }
    //     } else {
    //         printf("Error: incorrect parameter!\n");
    //         exit(0);
    //     }
    // // -c -d -o | -c -o -d | -o -c -d | -o -d -c | -d -c -o | -d -o -c
    // } else if (c == 7){
    //     ifPathCorrect(v[4]);
    //     ifPathCorrect(v[6]);
    //     // -c -d -o
    //     if (strcmp(v[1], "-c") == 0 && strcmp(v[3], "-d") == 0 && strcmp(v[5], "-o") == 0){
    //         sort_value_type = v[2];
    //         if (strstr(v[4], cwd) != NULL){
    //             memcpy(abs_path, &v[4][cwd_len+1], strlen(v[4])-cwd_len);
    //             abs_path[strlen(v[4]) - cwd_len] = '\0';
    //             strcpy(initial_dir, abs_path);
    //         } else {
    //             strcpy(initial_dir, v[4]);
    //         }
    //         if (strstr(v[6], cwd) != NULL){
    //             char abs_path2[1024];
    //             memcpy(abs_path2, &v[6][cwd_len+1], strlen(v[6])-cwd_len);
    //             abs_path2[strlen(v[6]) - cwd_len] = '\0';
    //             strcpy(output_path, abs_path2);
    //         } else {
    //             strcpy(output_path, v[6]);
    //         }
    //     // -c -o -d
    //     } else if (strcmp(v[1], "-c") == 0 && strcmp(v[3], "-o") == 0 && strcmp(v[5], "-d") == 0){
    //         sort_value_type = v[2];
    //         if (strstr(v[4], cwd) != NULL){
    //             memcpy(abs_path, &v[4][cwd_len+1], strlen(v[4])-cwd_len);
    //             abs_path[strlen(v[4]) - cwd_len] = '\0';
    //             strcpy(output_path, abs_path);
    //         } else {
    //             strcpy(output_path, v[4]);
    //         }
    //         if (strstr(v[6], cwd) != NULL){
    //             char abs_path2[1024];
    //             memcpy(abs_path2, &v[6][cwd_len+1], strlen(v[6])-cwd_len);
    //             abs_path2[strlen(v[6]) - cwd_len] = '\0';
    //             strcpy(initial_dir, abs_path2);
    //         } else {
    //             strcpy(initial_dir, v[6]);
    //         }
    //     // -o -c -d
    //     } else if (strcmp(v[1], "-o") == 0 && strcmp(v[3], "-c") == 0 && strcmp(v[5], "-d") == 0){
    //         sort_value_type = v[4];
    //         if (strstr(v[2], cwd) != NULL){
    //             memcpy(abs_path, &v[2][cwd_len+1], strlen(v[2])-cwd_len);
    //             abs_path[strlen(v[2]) - cwd_len] = '\0';
    //             strcpy(output_path, abs_path);
    //         } else {
    //             strcpy(output_path, v[2]);
    //         }
    //         if (strstr(v[6], cwd) != NULL){
    //             char abs_path2[1024];
    //             memcpy(abs_path2, &v[6][cwd_len+1], strlen(v[6])-cwd_len);
    //             abs_path2[strlen(v[6]) - cwd_len] = '\0';
    //             strcpy(initial_dir, abs_path2);
    //         } else {
    //             strcpy(initial_dir, v[6]);
    //         }
    //     // -o -d -c
    //     } else if (strcmp(v[1], "-o") == 0 && strcmp(v[3], "-d") == 0 && strcmp(v[5], "-c") == 0){
    //         sort_value_type = v[6];
    //         if (strstr(v[2], cwd) != NULL){
    //             memcpy(abs_path, &v[2][cwd_len+1], strlen(v[2])-cwd_len);
    //             abs_path[strlen(v[2]) - cwd_len] = '\0';
    //             strcpy(output_path, abs_path);
    //         } else {
    //             strcpy(output_path, v[2]);
    //         }
    //         if (strstr(v[4], cwd) != NULL){
    //             char abs_path2[1024];
    //             memcpy(abs_path2, &v[4][cwd_len+1], strlen(v[4])-cwd_len);
    //             abs_path2[strlen(v[4]) - cwd_len] = '\0';
    //             strcpy(initial_dir, abs_path2);
    //         } else {
    //             strcpy(initial_dir, v[4]);
    //         }
    //     // -d -c -o
    //     } else if (strcmp(v[1], "-d") == 0 && strcmp(v[3], "-c") == 0 && strcmp(v[5], "-o") == 0){
    //         sort_value_type = v[4];
    //         if (strstr(v[6], cwd) != NULL){
    //             memcpy(abs_path, &v[6][cwd_len+1], strlen(v[6])-cwd_len);
    //             abs_path[strlen(v[6]) - cwd_len] = '\0';
    //             strcpy(output_path, abs_path);
    //         } else {
    //             strcpy(output_path, v[6]);
    //         }
    //         if (strstr(v[2], cwd) != NULL){
    //             char abs_path2[1024];
    //             memcpy(abs_path2, &v[2][cwd_len+1], strlen(v[2])-cwd_len);
    //             abs_path2[strlen(v[2]) - cwd_len] = '\0';
    //             strcpy(initial_dir, abs_path2);
    //         } else {
    //             strcpy(initial_dir, v[2]);
    //         }
    //     // -d -o -c
    //     } else if (strcmp(v[1], "-d") == 0 && strcmp(v[3], "-o") == 0 && strcmp(v[5], "-c") == 0){
    //         sort_value_type = v[6];
    //         if (strstr(v[4], cwd) != NULL){
    //             memcpy(abs_path, &v[4][cwd_len+1], strlen(v[4])-cwd_len);
    //             abs_path[strlen(v[4]) - cwd_len] = '\0';
    //             strcpy(output_path, abs_path);
    //         } else {
    //             strcpy(output_path, v[4]);
    //         }
    //         if (strstr(v[2], cwd) != NULL){
    //             char abs_path2[1024];
    //             memcpy(abs_path2, &v[2][cwd_len+1], strlen(v[2])-cwd_len);
    //             abs_path2[strlen(v[2]) - cwd_len] = '\0';
    //             strcpy(initial_dir, abs_path2);
    //         } else {
    //             strcpy(initial_dir, v[2]);
    //         }
    //     }
    // }
    //暂时comment 掉
    // else {
    //     printf("Error: insufficient parameter!\n");
    //     exit(0);
    // }


    printf("Initial TID: %ld\n",pthread_self());
    printf("INITIAL: %s\n", initial_dir);
    printf("OUTPUT: %s\n", output_path);
    char* tmpInitDir = strdup(initial_dir);
    recur((void *)tmpInitDir);


    printf("sent %d files\n",sentCounter);

    pthread_mutex_destroy(&path_lock);
    pthread_mutex_destroy(&threadlock);
    pthread_mutex_destroy(&csv_lock);
    pthread_mutex_destroy(&count_lock);
    free(tmpInitDir);
    close(sock);
    return 0;
}
