/*
    C ECHO client example using sockets
*/
#include<stdio.h> //printf
#include<string.h>    //strlen
#include<sys/socket.h>    //socket
#include<arpa/inet.h> //inet_addr
//new

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
//socket global
int sock;
//global
char* sort_value_type;
int tidindex = 0;
int fileindex = 0;
pthread_t tid[10000];
int pathcounter = 0;
int accessArray[10000];
pthread_mutex_t path_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t threadlock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t csv_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t count_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sort_lock = PTHREAD_MUTEX_INITIALIZER;

char* headerArray[28];   // this array hold the first row.
int check_header = 0;
char **final_sorted[50000];
int index_of_sorted = 0;
int count_row = 0;

char file_dictionary[1000][300];
char thread_path[1000][300];
#define VALID_MOVIE_HEADER_NUMBER 28

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

/* This array contains the first row of the csv file.
It stores all the value types. */
void initValueTypesArray(char** array,int arraySize,char* line){
    char *token = strtok(line, ",");
    int counter=0;
    while (token != NULL && counter<arraySize){
        if(token[strlen(token)-1] == '\n') token[strlen(token)-1]=0;
        strcpy(array[counter], token);
        token = strtok(NULL, ",");
        counter++;
    }
    return;
}

// check if a string is numeric, 0 -> false, non-zero -> true
int isNumeric(char* str){
    //printf("%s\n",str);
    if (str == NULL || *str == '\0' || isspace(*str)){
      return 0;
    }

    char * p;
    strtod (str, &p);
    return *p == '\0'?0:1;
}

char *trimwhitespace(char *str) {
  char *end;

  // Trim leading space
  while(isspace((unsigned char)*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) end--;

  // Write new null terminator
  *(end+1) = 0;

  return str;
}

void *send_request(char* send_file_path)
{
    char* message=strdup(send_file_path);

    //printf("sent %s \n\n",message);

    pthread_mutex_lock(&sort_lock);
    sentCounter++;

    if(send(sock , message , strlen(message) , 0) < 0)
    {
        puts("Send failed");
        return 1;
    }

    char server_reply[100];
    if(recv(sock , server_reply , 100 , 0) < 0)
    {
        puts("recv failed");
        return 2;
    }

    puts("Server replies :");
    puts(server_reply);
    pthread_mutex_unlock(&sort_lock);
    free(message);
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

void *sort_one_file(void* arg_path){
    printf("\n\npassed file : %s\n\n",arg_path );
    pthread_mutex_lock(&sort_lock);

    char* tmp_path = (char*)arg_path;

    //output_path = tmp_path;
    FILE    *input_file = fopen(tmp_path, "r");
    //input_file

    if (input_file == NULL){
        fprintf(stderr, "Error : Failed to open entry file - %s\n", strerror(errno));
        fclose(input_file);
        return NULL;
    }

    // head of the Linked List
    struct node *head = NULL;
    struct node *prev = NULL;
    char line[1024];    // temp array for holding csv file lines.
    int rowNumber=-1;    // hold the number of lines of the csv file.
    int value_type_number;    // hold the number of value types(column numbers).
    char* headerLine=NULL;   // hold the first line of the csv file, which is the value types.
    int isFirstElement = 0; // mark the first element in LL

    // loop for reading the csv file line by line.

    while (fgets(line, 1024, input_file)){
        rowNumber++;
        char* tmp = strdup(line);
        //printf("#### %s", tmp);
        // first row
        // Returns first token
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
            free(tmp);

            continue;
        }

        /* malloc array for holding tokens.*/
        char** new_array = malloc(value_type_number * sizeof(char*));

        // using token to split each line.
        // store each token into corresponding array cell.
        int counter = 0;

        //variables in order to keep tracking of cells that contains , value
        int headerDoubleQuotes = 0;
        int tailerDoubleQuotes =0;

        char * tempStr;
        char  tempCell[100000];
        char *dummy = NULL;
        char * KillerQueen;

        while (token != NULL){
            // printf("\nTOKENTOKEN: %s\n", token);

            if(token[strlen(token)-1] == '\n'){
                int len = strlen(token);
                token[len-1]='\0';//make it end of string
            }

            tempStr = trimwhitespace(token);

            if(tempStr[0] == '"'){
                headerDoubleQuotes=1;
                strcpy(tempCell,"");
            }

            if(tempStr[strlen(tempStr)-1] == '"'){
                headerDoubleQuotes=0;
                tailerDoubleQuotes=1;
            }
            if(headerDoubleQuotes== 1 && tailerDoubleQuotes == 0){
                dummy=strdup(tempStr);
                int len =strlen(dummy);
                dummy[len]=',';
                dummy[len+1]='\0';
                strcat(tempCell, dummy);
            }else if(tailerDoubleQuotes == 1){

                dummy=strdup(tempStr);
                strcat(tempCell, dummy);
                headerDoubleQuotes=0;
            }

            if(tailerDoubleQuotes == 1){
                tailerDoubleQuotes=0;
                new_array[counter] = tempCell;
                counter++;
            }
            else if(headerDoubleQuotes!= 1 && tailerDoubleQuotes!=1){
                new_array[counter] = *token ? trimwhitespace(token) : EMPTY_STRING; // store token into array
                counter++;
            }
            token = strtok_single(NULL, ",");
        }
        free(dummy);



        // create a new node
        // rowNumber starts from 1
        struct node *temp = (struct node*) malloc(sizeof(struct node));
        temp-> line_array = (char**)malloc(value_type_number*sizeof (char*));

        int i = 0;
        for(;i<value_type_number;i++){
            temp-> line_array[i]=strdup(new_array[i]);
        }
        temp-> next = NULL;

        if (isFirstElement == 0){
            temp-> next = head;
            head = temp;
            isFirstElement = 1;
            prev = head;
            free(tmp);
            free(new_array);

            continue;
        }
        prev-> next = temp;
        prev = temp;
        free(new_array);
        free(tmp);

    }

    if (check_header == 0){
        initValueTypesArray(headerArray,value_type_number,headerLine);
        check_header = 1;
    }


    //resuage of temp to 'copy' a head, then pass it to initDataArray to store 2d data array
    struct node *temp = (struct node*) malloc(sizeof(struct node));
    temp-> line_array = head->line_array;
    temp-> next = head -> next;


    //[row][column]
    dataCol= value_type_number;
    dataRow = rowNumber;


    char** dataArray[dataRow];
    int i,j;
    i = 0;
    for(;i<dataRow;i++){
        j=0;
        dataArray[i]= malloc(sizeof (char*) * dataCol);
            for(; j< dataCol ;j++){
                                    //printf("%s ",temp->line_array[j]);

                dataArray[i][j]=temp->line_array[j];
                //printf("%s ", dataArray[i][j]);
            }
            temp = temp->next;
                                    //printf("\n");

    }


    //free(dataArray);


    /* sorintg */
    // first need to decide which column to sort. Do the search on headerArray.
     i = 0;
    int isFound = 1;
    for (; i < value_type_number; i++){
        if (strcmp(trimwhitespace(headerArray[i]), sort_value_type) == 0){
            isFound = 0;    // need to check if the csv file has this type.
            break;  // i is the index of the column
        }
    }



    // the type is not found in the file. ERROR.
    if (isFound == 1){
        printf("Error: The value type was not found in csv file!\n");

    }else{
        //store the column as an array
        SortArray *sort_array;
        sort_array = (SortArray*) malloc(rowNumber * sizeof(SortArray));
        int count = 0;
        int sortArraycount=0;
        //a safer way to check if numeric
        int numericFlag = 0;

        while (count < rowNumber){
            sort_array[count].index = count;
            sort_array[count].str = dataArray[count][i];
            numericFlag += isNumeric(sort_array[count].str);
            count++;
        }

        // check if the value is digits or string
        // return 0 for false, non-zero for true.
        int numeric = numericFlag;

        // if the string is a number, then sort based on the value of the number
        // NOTE: numeric 0:false 1:true
        int MAXROW=rowNumber-1;

        // int test = 0;
        // for (; test < MAXROW+1; test++){
        //     printf("%d\n", sort_array[test].index);
        // }
        if(MAXROW>=0){
            mergeSort(sort_array, 0, MAXROW,numeric);
        }

        // int test = 0;
        // for (; test < MAXROW+1; test++){
        //     printf("%d\n", sort_array[test].index);
        // }

        // i,j;
        // i = 0;
        // for(;i<dataRow;i++){
        //     j=0;
        //         for(; j< dataCol ;j++){
        //             printf("%s,", dataArray[i][j]);
        //         }
        //         printf("\n");
        // }


        count_row += MAXROW+1;
        //printf("%d current row\n",count_row);
        int row_counter = 0;
        int col_counter = 0;
        for (; row_counter < MAXROW+1; row_counter++){
            for (; col_counter < 28; col_counter++){
                // printf("%d ----",sort_array[row_counter].index);
                //printf(" %s ",dataArray[sort_array[row_counter].index][col_counter]);
                if (col_counter == 27){
                    int len = strlen(dataArray[sort_array[row_counter].index][col_counter]);
                    dataArray[sort_array[row_counter].index][col_counter][len]='\n';
                    dataArray[sort_array[row_counter].index][col_counter][len+1]='\0';
                    final_sorted[row_counter+index_of_sorted][col_counter] = dataArray[sort_array[row_counter].index][col_counter];
                    //printf("%s ",final_sorted[row_counter+index_of_sorted][col_counter]);
                    //strcat(final_sorted[row_counter+index_of_sorted][col_counter], "\n");
                    break;
                } else {

                    int len = strlen(dataArray[sort_array[row_counter].index][col_counter]);
                    if (strstr(dataArray[sort_array[row_counter].index][col_counter], ",") == NULL){
                        //      if (isNumeric(dataArray[sort_array[row_counter].index][col_counter]) != 1){
                        // printf("%s*****%d***\n", dataArray[sort_array[row_counter].index][col_counter], strlen(dataArray[sort_array[row_counter].index][col_counter]));
                        // }
                        dataArray[sort_array[row_counter].index][col_counter][len]=',';
                        dataArray[sort_array[row_counter].index][col_counter][len+1]='\0';
                    //printf("%s***\n", dataArray[sort_array[row_counter].index][col_counter]);

                    }
                    final_sorted[row_counter+index_of_sorted][col_counter] = dataArray[sort_array[row_counter].index][col_counter];
                }


                //printf("%s ",final_sorted[row_counter+index_of_sorted][col_counter]);
            }
            //printf("\n");
            col_counter = 0;
        }
        index_of_sorted = index_of_sorted + row_counter;

    // for (; row_counter < dataRow; row_counter++){
    //     final_sorted[row_counter+index_of_sorted].index = row_counter;
    //     final_sorted[row_counter+index_of_sorted].str = dataArray[sort_array[];
    // }


     //free(sort_array);
  }


    // // free Linked List
    // struct node *tmp;
    // while (head != NULL){
    //     tmp = head;
    //     head = head->next;
    //     int i = 0;
    //     for(;i<value_type_number;i++){
    //         free(tmp-> line_array[i]);
    //     }
    //     free(tmp-> line_array);
    //     free(tmp);
    // }

    // free(temp);
    // free(headerLine);
    // fclose(input_file);
    // free(tmp_path);

    pthread_mutex_unlock(&sort_lock);

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
            // char* headerfile = strdup(tmp_path);
            // int flag=0;
            // if(count_header(tmp_path)==28){
            //   flag =1 ;
            // }
            //if(flag){
              strcpy(file_dictionary[fileindex++],tmp_path);
              pthread_create(&tidgroup[tidlocalindex++], NULL, (void *)&send_request, (void *)tmp_path);
            //}
            //free(headerfile);
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

int main(int c, char *v[]){
    struct sockaddr_in server;
    char *server_reply;
    //Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");

    server.sin_addr.s_addr = inet_addr("127.0.0.1");//default to localhost
    server.sin_family = AF_INET;
    server.sin_port = htons( atoi(v[8]) );//-c -d -o -p portnumber

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
    int i;
    i = 0;
    for(;i<50000;i++){
        final_sorted[i]= malloc(sizeof (char*) * 28);
    }

    struct dirent *pDirent;
    DIR *pDir;

    //headerArray = (char**)malloc(28 * sizeof(char*));

    int count_m1 = 0;
    for(; count_m1 < 28; count_m1++){
        headerArray[count_m1] = malloc(90*sizeof(char));
    }

    // path for output sorted csv
    char output_path[500] = ".";
    // path for starting dir
    char initial_dir[500] = ".";

    char abs_path[1024];

    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    int cwd_len = strlen(cwd);

    int isAbs = 1;


    // arguments less than 3, error
    if (c < 3){
        printf("Error: insufficient parameters!\n");
        exit(0);
    }

    if (c == 3){
      sort_value_type=v[2];
    }   // 2 parameters: -c -d | -c -o | -d -c | -o -c
    else if (c == 5){
        if (strstr(v[4], cwd) != NULL){
            memcpy(abs_path, &v[4][cwd_len+1], strlen(v[4])-cwd_len);
            abs_path[strlen(v[4]) - cwd_len] = '\0';
            isAbs = 0;
        }
        ifPathCorrect(v[4]);

        // -c -d
        if (strcmp(v[1], "-c") == 0 && strcmp(v[3], "-d") == 0){
            sort_value_type = v[2];
            if (isAbs == 0){
                strcpy(initial_dir, abs_path);
            } else {
                strcpy(initial_dir, v[4]);
            }
        // -c -o
        } else if (strcmp(v[1], "-c") == 0 && strcmp(v[3], "-o") == 0){
            sort_value_type = v[2];
            if (isAbs == 0){
                strcpy(output_path, abs_path);
            } else {
                strcpy(output_path, v[4]);
            }
        // -d -c
        } else if (strcmp(v[1], "-d") == 0 && strcmp(v[3], "-c") == 0) {
            sort_value_type = v[4];
            if (strstr(v[2], cwd) != NULL){
                memcpy(abs_path, &v[2][cwd_len+1], strlen(v[2])-cwd_len);
                abs_path[strlen(v[2]) - cwd_len] = '\0';
                strcpy(initial_dir, abs_path);
            } else {
                strcpy(initial_dir, v[2]);
            }
        // -o -c
        } else if (strcmp(v[1], "-o") == 0 && strcmp(v[3], "-c") == 0){
            sort_value_type = v[4];
            if (strstr(v[2], cwd) != NULL){
                memcpy(abs_path, &v[2][cwd_len+1], strlen(v[2])-cwd_len);
                abs_path[strlen(v[2]) - cwd_len] = '\0';
                strcpy(output_path, abs_path);
            } else {
                strcpy(output_path, v[2]);
            }
        } else {
            printf("Error: incorrect parameter!\n");
            exit(0);
        }
    // -c -d -o | -c -o -d | -o -c -d | -o -d -c | -d -c -o | -d -o -c
    } else if (c == 7){
        ifPathCorrect(v[4]);
        ifPathCorrect(v[6]);
        // -c -d -o
        if (strcmp(v[1], "-c") == 0 && strcmp(v[3], "-d") == 0 && strcmp(v[5], "-o") == 0){
            sort_value_type = v[2];
            if (strstr(v[4], cwd) != NULL){
                memcpy(abs_path, &v[4][cwd_len+1], strlen(v[4])-cwd_len);
                abs_path[strlen(v[4]) - cwd_len] = '\0';
                strcpy(initial_dir, abs_path);
            } else {
                strcpy(initial_dir, v[4]);
            }
            if (strstr(v[6], cwd) != NULL){
                char abs_path2[1024];
                memcpy(abs_path2, &v[6][cwd_len+1], strlen(v[6])-cwd_len);
                abs_path2[strlen(v[6]) - cwd_len] = '\0';
                strcpy(output_path, abs_path2);
            } else {
                strcpy(output_path, v[6]);
            }
        // -c -o -d
        } else if (strcmp(v[1], "-c") == 0 && strcmp(v[3], "-o") == 0 && strcmp(v[5], "-d") == 0){
            sort_value_type = v[2];
            if (strstr(v[4], cwd) != NULL){
                memcpy(abs_path, &v[4][cwd_len+1], strlen(v[4])-cwd_len);
                abs_path[strlen(v[4]) - cwd_len] = '\0';
                strcpy(output_path, abs_path);
            } else {
                strcpy(output_path, v[4]);
            }
            if (strstr(v[6], cwd) != NULL){
                char abs_path2[1024];
                memcpy(abs_path2, &v[6][cwd_len+1], strlen(v[6])-cwd_len);
                abs_path2[strlen(v[6]) - cwd_len] = '\0';
                strcpy(initial_dir, abs_path2);
            } else {
                strcpy(initial_dir, v[6]);
            }
        // -o -c -d
        } else if (strcmp(v[1], "-o") == 0 && strcmp(v[3], "-c") == 0 && strcmp(v[5], "-d") == 0){
            sort_value_type = v[4];
            if (strstr(v[2], cwd) != NULL){
                memcpy(abs_path, &v[2][cwd_len+1], strlen(v[2])-cwd_len);
                abs_path[strlen(v[2]) - cwd_len] = '\0';
                strcpy(output_path, abs_path);
            } else {
                strcpy(output_path, v[2]);
            }
            if (strstr(v[6], cwd) != NULL){
                char abs_path2[1024];
                memcpy(abs_path2, &v[6][cwd_len+1], strlen(v[6])-cwd_len);
                abs_path2[strlen(v[6]) - cwd_len] = '\0';
                strcpy(initial_dir, abs_path2);
            } else {
                strcpy(initial_dir, v[6]);
            }
        // -o -d -c
        } else if (strcmp(v[1], "-o") == 0 && strcmp(v[3], "-d") == 0 && strcmp(v[5], "-c") == 0){
            sort_value_type = v[6];
            if (strstr(v[2], cwd) != NULL){
                memcpy(abs_path, &v[2][cwd_len+1], strlen(v[2])-cwd_len);
                abs_path[strlen(v[2]) - cwd_len] = '\0';
                strcpy(output_path, abs_path);
            } else {
                strcpy(output_path, v[2]);
            }
            if (strstr(v[4], cwd) != NULL){
                char abs_path2[1024];
                memcpy(abs_path2, &v[4][cwd_len+1], strlen(v[4])-cwd_len);
                abs_path2[strlen(v[4]) - cwd_len] = '\0';
                strcpy(initial_dir, abs_path2);
            } else {
                strcpy(initial_dir, v[4]);
            }
        // -d -c -o
        } else if (strcmp(v[1], "-d") == 0 && strcmp(v[3], "-c") == 0 && strcmp(v[5], "-o") == 0){
            sort_value_type = v[4];
            if (strstr(v[6], cwd) != NULL){
                memcpy(abs_path, &v[6][cwd_len+1], strlen(v[6])-cwd_len);
                abs_path[strlen(v[6]) - cwd_len] = '\0';
                strcpy(output_path, abs_path);
            } else {
                strcpy(output_path, v[6]);
            }
            if (strstr(v[2], cwd) != NULL){
                char abs_path2[1024];
                memcpy(abs_path2, &v[2][cwd_len+1], strlen(v[2])-cwd_len);
                abs_path2[strlen(v[2]) - cwd_len] = '\0';
                strcpy(initial_dir, abs_path2);
            } else {
                strcpy(initial_dir, v[2]);
            }
        // -d -o -c
        } else if (strcmp(v[1], "-d") == 0 && strcmp(v[3], "-o") == 0 && strcmp(v[5], "-c") == 0){
            sort_value_type = v[6];
            if (strstr(v[4], cwd) != NULL){
                memcpy(abs_path, &v[4][cwd_len+1], strlen(v[4])-cwd_len);
                abs_path[strlen(v[4]) - cwd_len] = '\0';
                strcpy(output_path, abs_path);
            } else {
                strcpy(output_path, v[4]);
            }
            if (strstr(v[2], cwd) != NULL){
                char abs_path2[1024];
                memcpy(abs_path2, &v[2][cwd_len+1], strlen(v[2])-cwd_len);
                abs_path2[strlen(v[2]) - cwd_len] = '\0';
                strcpy(initial_dir, abs_path2);
            } else {
                strcpy(initial_dir, v[2]);
            }
        }
    }
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
    // FILE *fp;
    // char *cat_tmp = malloc(sizeof(char)*200);
    // strcpy(cat_tmp, "/AllFiles-sorted-<");
    // strcat(strcat(cat_tmp, sort_value_type),">.csv");
    // strcat(output_path, cat_tmp);
    // //printf("OUTPUT: %s\n", output_path);
    //
    // fp=fopen(output_path,"w+");
    //
    // // print header
    // int count1 = 0;
    //
    // int output_count = 0;
    //
    // for (; count1 < dataCol; count1++){
    //     if (count1 == dataCol-1){
    //         fprintf(fp, "%s\n", headerArray[count1]);
    //         break;
    //     }
    //     fprintf(fp, "%s,", headerArray[count1]);
    // }
    //
    // count1 = 0;
    // int count2 = 0;
    // for(; count2 < count_row; count2++){
    //     for (; count1 < dataCol; count1++){
    //         // printf("%s",final_sorted[count2][count1]);
    //         fprintf(fp,"%s",final_sorted[count2][count1]);
    //     }
    //     count1 = 0;
    // }

    // i=0;
    // for(; i <fileindex;i++){
    //   char wut[1024];
    //   printf("main : %s index %d\n",file_dictionary[i],i );
    //   strcpy(wut,"/");
    //   strcat(wut,file_dictionary[i]);
    //
    //
    //         //pthread_create(&tid_sort[thread_count], NULL, (void*)&sort_one_file, (void*)file_dictionary[thread_count]);
    //         sort_one_file(file_dictionary[i]);
    //
    // }

    //pthread_mutex_unlock(&count_lock);

    // pthread_mutex_lock(&sort_lock);
    // int join_count = 0;
    // for (; join_count < fileindex; join_count++){
    //     printf("joined: %u", tid_sort);
    //     pthread_join(tid_sort[join_count], NULL);

    // }
    // pthread_mutex_unlock(&sort_lock);

    pthread_mutex_destroy(&path_lock);
    pthread_mutex_destroy(&threadlock);
    pthread_mutex_destroy(&csv_lock);
    pthread_mutex_destroy(&count_lock);
    free(tmpInitDir);
    //fclose(fp);
    close(sock);
    return 0;
}
