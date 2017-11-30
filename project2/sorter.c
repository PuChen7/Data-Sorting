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
#define VALID_MOVIE_HEADER_NUMBER 28
#define VALID_USAGE   "invalid argument numbers\ncorrect usage :./sample -c <column> (other args are optional after this)-d<directory to start> -o<outputdirectory>"
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

/*
struct ArgsForSorting{
    char* input_path;
    char* output_path;
};

struct ArgsForRecur{
    char path[1000][500];
    char* output_path;
};
*/
char file_dictionary[1000][300];
char thread_path[1000][300];
#define VALID_MOVIE_HEADER_NUMBER 28


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
        array[counter]=token;
        token = strtok(NULL, ",");
        counter++;
    }
}

// char*** initDataArray(char*** array,struct node * data){
//     int i,j;
//     i = 0;
//     for(;i<dataRow;i++){
//     j=0;
//         for(; j< dataCol ;j++){
//             array[i][j]=data->line_array[j];
//
//         }
//         data = data->next;
//     }
//     return array;
// }

// check if a string is numeric, 0 -> false, non-zero -> true
int isNumeric(char* str){
    printf("%s\n",str);
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
    retstr = mystr;
    lastdot = strrchr (retstr, '.');
    if (lastdot != NULL)
        *lastdot = '\0';
    return retstr;
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



    char* headerArray[value_type_number];   // this array hold the first row.
    initValueTypesArray(headerArray,value_type_number,headerLine);

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
            dataArray[i][j]=temp->line_array[j];
        }
        temp = temp->next;
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
          //numericFlag += isNumeric(sort_array[count].str);
          count++;
      }

      // check if the value is digits or string
      // return 0 for false, non-zero for true.
      int numeric = numericFlag;

      // if the string is a number, then sort based on the value of the number
      // NOTE: numeric 0:false 1:true
      int MAXROW=rowNumber-1;


      if(MAXROW>=0){
          mergeSort(sort_array, 0, MAXROW,numeric);
      }

      //print header
      count=0;
      for(;count<value_type_number;count++){
          count==(value_type_number-1)?
          printf("%s\n", headerArray[count])
          :printf("%s,", headerArray[count]);

      }
      //print content
      count=0;
      for(;count<MAXROW+1;count++){
          for(i=0;i<dataCol;i++){
              i==(dataCol-1)?
              printf("%s\n",dataArray[sort_array[count].index][i])
              :printf("%s,",dataArray[sort_array[count].index][i]);
          }//end of line
      }

      printf("this time %d\n",MAXROW);

      free(sort_array);
  }


    // free Linked List
    struct node *tmp;
    while (head != NULL){
        tmp = head;
        head = head->next;
        int i = 0;
        for(;i<value_type_number;i++){
            free(tmp-> line_array[i]);
        }
        free(tmp-> line_array);
        free(tmp);
    }

    free(temp);
    free(headerLine);
    fclose(input_file);
    free(tmp_path);

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

        //pthread_mutex_lock(&count_lock);
        // path_index = pathcounter % 1000;
        // pathcounter++;
        char* tmp_path=(char*)malloc(sizeof tmppath+ sizeof pDirent->d_name);
        //strcat(strcat(strcat(tmp_path,tmppath),"/"),pDirent->d_name);
        sprintf(tmp_path, "%s/%s", tmppath, pDirent->d_name);
        // printf("temp path : %s\n",tmp_path );
        //printf("read some file %s %s\n", tmppath,pDirent->d_name);
        //pthread_mutex_unlock(&count_lock);

        if (pDirent->d_type == DT_DIR){
            // pthread_mutex_lock(&count_lock);
            // char* tmp_path=strdup(thread_path[path_index]);
            // pthread_mutex_unlock(&count_lock);
            pthread_create(&tidgroup[tidlocalindex], NULL, (void *)&recur, (void *)tmp_path);
            tidlocalindex++;


            // pthread_mutex_lock(&threadlock);
            // tidlocalindex++;
            // pthread_mutex_unlock(&threadlock);

            continue;
        } else {
            // if it is a new csv, sort it.
            //pthread_mutex_lock(&count_lock);
            if(strlen(tmp_path) < 4 || strcmp(tmp_path + strlen(tmp_path) - 4,".csv") != 0
            || strstr(tmp_path,"-sorted")){//found csv
              //pthread_mutex_unlock(&count_lock);
                continue;
            }
            //char* tmp_path=strdup(thread_path[path_index]);
            //printf("\n\n%s  csv\n\n",tmp_path);
            //pthread_mutex_unlock(&count_lock);
            // pthread_create(&tidgroup[tidlocalindex], NULL, (void *)&sort_one_file, (void *)tmp_path);
            // tidlocalindex++;
            // pthread_mutex_lock(&count_lock);
            strcpy(file_dictionary[fileindex],tmp_path);
            fileindex++;
            // if(count_header(tmp_path)!=28){
            //   printf("invalid csv %s \n",tmp_path);
            //   pthread_mutex_unlock(&count_lock);
            //   continue;
            // }
            // pthread_mutex_unlock(&count_lock);
            //

            pthread_create(&tidgroup[tidlocalindex], NULL, (void *)&sort_one_file, (void *)tmp_path);
            tidlocalindex++;
            //pthread_join(tidgroup[500],NULL);
            //printf("come back %s\n", tmp_path);
            //tidlocalindex++;

            // pthread_mutex_lock(&threadlock);
            // pthread_mutex_unlock(&threadlock);

            //strcpy(file_dictionary[fileindex++],thread_path[path_index]);
            //printf("%s csv index %d\n",thread_path[path_index],fileindex++);
            //
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
    return NULL;
}

void ifPathCorrect(char* path){
    if (strstr(path, "//") != NULL){
        printf("Error: incorrect path!\n");
        exit(0);
    }
}

int main(int c, char *v[]){


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
            if (isAbs == 0){
                strcpy(initial_dir, abs_path);
            } else {
                strcpy(initial_dir, v[4]);
            }
        // -c -o
        } else if (strcmp(v[1], "-c") == 0 && strcmp(v[3], "-o") == 0){
            if (isAbs == 0){
                strcpy(output_path, abs_path);
            } else {
                strcpy(output_path, v[4]);
            }
        // -d -c
        } else if (strcmp(v[1], "-d") == 0 && strcmp(v[3], "-c") == 0) {
            if (strstr(v[2], cwd) != NULL){
                memcpy(abs_path, &v[2][cwd_len+1], strlen(v[2])-cwd_len);
                abs_path[strlen(v[2]) - cwd_len] = '\0';
                strcpy(initial_dir, abs_path);
            } else {
                strcpy(initial_dir, v[2]);
            }
        // -o -c
        } else if (strcmp(v[1], "-o") == 0 && strcmp(v[3], "-c") == 0){
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
    } else {
        printf("Error: insufficient parameter!\n");
        exit(0);
    }

    printf("Initial TID: %ld\n",pthread_self());
    printf("INITIAL: %s\n", initial_dir);
    printf("OUTPUT: %s\n", output_path);
    char* tmpInitDir = strdup(initial_dir);
    recur((void *)tmpInitDir);



    pthread_mutex_lock(&count_lock);
    int i=0;
    for(; i <fileindex;i++){
      printf("main : %s index %d\n",file_dictionary[i],i );
    }
    pthread_mutex_unlock(&count_lock);
    pthread_mutex_destroy(&path_lock);
    pthread_mutex_destroy(&threadlock);
    pthread_mutex_destroy(&csv_lock);
    pthread_mutex_destroy(&count_lock);
    free(tmpInitDir);
    return 0;
}
