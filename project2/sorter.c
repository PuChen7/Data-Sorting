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
static int *pCounter;
char* sort_value_type;
int tidindex = 0;
pthread_t tid[10000];
int pathcounter = 0;

pthread_mutex_t path_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t threadlock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t path2_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t csv_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t count_lock = PTHREAD_MUTEX_INITIALIZER;

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
char* output_path;
char thread_path[1000][500];
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

void initDataArray(char* array[dataRow][dataCol],struct node * data){
    int i,j;
    i = 0;
    for(;i<dataRow;i++){
    j=0;
        for(; j< dataCol ;j++){
            array[i][j]=data->line_array[j];
        }
        data = data->next;
    }
}

// check if a string is numeric, 0 -> false, non-zero -> true
int isNumeric(char* str){
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
      char* tmp = line;
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
          break;
      }
    }

  fclose(input_file);
  return value_type_number;
}

void *sort_one_file(void* arg_path){
    //struct ArgsForSorting* sortArgs = (struct ArgsForSorting*) argument;
    char* tmp_path = arg_path;
    //output_path = tmp_path;
    printf("sort one file: %s\n", tmp_path);
    FILE    *input_file;
       
    input_file = fopen(tmp_path, "r");
    
    if (input_file == NULL){
        printf("hahahaha\n");
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
    //fgets(line, 1024, input_file);
    // loop for reading the csv file line by line.
    while (fgets(line, 1024, input_file)){
        rowNumber++;
        
        char* tmp = strdup(line);
        
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
        char  tempCell[1024];
        char *dummy = NULL;
        char * KillerQueen;
        while (token != NULL){
            if(token[strlen(token)-1] == '\n'){
                int len = strlen(token);
                token[len-1]='\0';//make it end of string
            }

            char *tempStr = trimwhitespace(token);
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
    char* dataArray[dataRow][dataCol];
    initDataArray(dataArray,temp);

    /* sorintg */
    // first need to decide which column to sort. Do the search on headerArray.
    int i = 0;
    int isFound = 1;
    for (; i < value_type_number; i++){
        if (strcmp(trimwhitespace(headerArray[i]), sort_value_type) == 0){
            isFound = 0;    // need to check if the csv file has this type.
            break;  // i is the index of the column
        }
    }
    // the type is not found in the file. ERROR.
    if (isFound == 1){
        //printf("Error: The value type was not found in csv file!\n");

    }else{

        // store the column as an array
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

        //printf("col1:%s col2:%s\n",sort_array[0].str,sort_array[1].str);
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
    return NULL;
}


// Recursively call thread to sort or traverse
void *recur(void *arg_path){
    char* tmp_path = arg_path;
    printf("path: %s\n", tmp_path);

    pthread_t waittid[10000];
    int countthread = 0, i, localcounter = 0, chunk = 512, joined = 0;

    //struct ArgsForRecur* recurArgs = (struct ArgsForRecur*) argument;

    DIR *dir = opendir(tmp_path);
    //printf("tmp_path: %s\n", tmp_path);
    pthread_t current_tid;

    if (dir == NULL){
        printf("No such directory\n");
        exit(0);
    }

    struct dirent *pDirent;
   
    while (pDirent =readdir(dir)){
        if (strcmp(pDirent->d_name, ".") == 0 || strcmp(pDirent->d_name, "..") == 0 || pDirent->d_name[0] == '.')
            continue;

        pthread_mutex_lock(&count_lock);
        localcounter = pathcounter % 1000;
        pathcounter++;
        pthread_mutex_unlock(&count_lock);
        // update path
        
        pthread_mutex_lock(&path_lock);
        //thread_path[localcounter] = strcat(strcat(tmp_path,"/"),pDirent->d_name);
        sprintf(thread_path[localcounter], "%s/%s", tmp_path, pDirent->d_name);
        pthread_mutex_unlock(&path_lock);
        // if it is a directory, continue traversing
        if (pDirent->d_type == DT_DIR){
             
            /*
            pthread_mutex_lock(&path2_lock);
            strcat(recurArgs->path, "/");
            printf("name: %s\n", pDirent->d_name);
            printf("recurArgs->path: %s\n", recurArgs->path);
            strcat(recurArgs->path,pDirent->d_name);
            pthread_mutex_unlock(&path2_lock);
            */
            pthread_create(&current_tid, NULL, (void *)&recur, (void *)&thread_path[localcounter]);
        } else {
            // if it is a new csv, sort it. 
            if(thread_path[localcounter]&&strcmp("csv",get_filename_ext(pDirent->d_name))==0 && strstr(pDirent->d_name,"-sorted")==NULL){//found csv

                pthread_mutex_lock(&csv_lock);
                
                /*
                char  BiteTheDust[1024];
                char  outP[outputLength];
                char  inP[strlen(thread_path[localcounter])];
                
                
                strcpy(outP,output_path);
                strcpy(inP,thread_path[localcounter]);
                
                char* fileNoExtension = strdup(remove_ext(pDirent->d_name));
                strcpy(BiteTheDust,sort_value_type);
                char outputPath[1024];
                strcpy(outputPath,strcat(strcat(strcat(strcat(outP,"/"),strcat(fileNoExtension,"-sorted<")),BiteTheDust),">.csv"));
                char inputPath[1024];
                strcpy(inputPath,strcat(strcat(inP,"/"),strcat(pDirent->d_name,".csv")));
                int headerNumber = count_header(inputPath);
                output_path = outputPath;
            
                if(headerNumber!=VALID_MOVIE_HEADER_NUMBER){
                    free(fileNoExtension);
                    continue;
                }
                */
                //struct ArgsForSorting *sortArgs = malloc(sizeof(struct ArgsForSorting));
                //sortArgs->input_path = inputPath;
                //sortArgs->output_path = outputPath;
                
                pthread_mutex_unlock(&csv_lock);
                
                pthread_create(&current_tid, NULL, (void *)&sort_one_file, (void *)&thread_path[localcounter]);
            }
            
        }
        waittid[countthread++] = current_tid;
        pthread_mutex_lock(&threadlock);
        tid[tidindex++] = current_tid;
        pthread_mutex_unlock(&threadlock);
        // pthread_join(currtid, NULL);
        if (countthread == chunk) {
            for (i = joined; i < countthread; i++) {
                pthread_join(waittid[i], NULL);
            }
            joined = countthread;
            chunk += 512;
        }
        
        for (i = joined; i < countthread; i++) {
            pthread_join(waittid[i], NULL);
        }
    }
    
    closedir(dir);
}


int main(int c, char *v[]){

    output_path = malloc(200*sizeof(char));
    

    if(c<3){
      printf("%s",VALID_USAGE);
    }

    struct dirent *pDirent;
    DIR *pDir;
    char cwd[1024];
    pCounter = mmap(NULL, sizeof *pCounter, PROT_READ | PROT_WRITE,
                        MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *pCounter = 0;

    // hierArrayCursor = mmap(NULL, sizeof *hierArrayCursor, PROT_READ | PROT_WRITE,
    //                     MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    // *hierArrayCursor = 0;
    //
    // hierArray =(char**) malloc(256 * sizeof (char*));
    //

    getcwd(cwd, sizeof(cwd));
    // if(c==1){
    //   if (getcwd(cwd, sizeof(cwd)) != NULL){
    //   //fprintf(stdout, "Current working dir: %s\n", cwd);
    //   }
    //   else
    //   perror("getcwd() error");
    // }
    /*
    here should code scanning current directory that is
    when -d is optional
    */

    char path_tmp[300];
    char* path;
    char path_modified[300];
    char output_tmp[300];
    char output_final[300];
    //char* output_path_tmp = NULL;
    char newStr[300];
    char newStr2[300];

    int cIndex=1;
    int secIndex=0;
    int trdIndex=0;
    if (c == 3){
        // current dir
        path_tmp[0] = '.';
        path = path_tmp;
    } else if (c == 5){
        
        if(strcmp(v[1],"-c")==0){
          cIndex=1;
          secIndex=3;
        }
        else if(strcmp(v[3],"-c")==0){
          cIndex=3;
          secIndex=1;
        }
        else{
        printf("%s",VALID_USAGE);
        return 0;
        }
        
        if(strcmp(v[secIndex],"-d")==0){
            // -d
            char pat[300];
            int matchingIndex;
            if(strstr(v[secIndex+1],cwd)!=NULL){
                matchingIndex = strlen(cwd);
                }
            else matchingIndex = 0 ;

            if(strlen(v[secIndex+1])==matchingIndex)
                    pat[0]='.';
            else{

                strcpy(pat,v[secIndex+1]+matchingIndex);

                if(strlen(pat)<=1) {
                    if(strlen(pat)==0)
                        pat[0]='.';
                    if(pat[0]==' ')pat[0]='.';
                    //pat[2]='\0';
                }
                else {
                    int loopindex = 1;
                    if(pat[0]=='.');
                    else if(pat[0]=='/'){
                        newStr[0] = '.';
                        for (; loopindex < strlen(pat)+1; loopindex++){
                            newStr[loopindex] = pat[loopindex-1];
                        }

                        loopindex = 0;
                        for (; loopindex < strlen(newStr); loopindex++){
                            pat[loopindex] = newStr[loopindex];
                        }
                    }
                }
            }

            path = pat;
            
        }else if(strcmp(v[secIndex],"-o")==0){
            
            // -o
            path_tmp[0] = '.';
            path = path_tmp;
            char pat[300];
            int matchingIndex;
            if(strstr(v[secIndex+1],cwd)!=NULL){
                matchingIndex = strlen(cwd);
                }
            else matchingIndex = 0 ;

            if(strlen(v[secIndex+1])==matchingIndex)
                    pat[0]='.';
            else{

                strcpy(pat,v[secIndex+1]+matchingIndex);

                if(strlen(pat)<=1) {
                    if(strlen(pat)==0)
                        pat[0]='.';
                    if(pat[0]==' ')pat[0]='.';
                    //pat[2]='\0';
                }
                else {
                    int loopindex = 1;
                    if(pat[0]=='.');
                    else if(pat[0]=='/'){
                        newStr2[0] = '.';
                        for (; loopindex < strlen(pat)+1; loopindex++){
                            newStr2[loopindex] = pat[loopindex-1];
                        }

                        loopindex = 0;
                        for (; loopindex < strlen(newStr2); loopindex++){
                            pat[loopindex] = newStr2[loopindex];
                        }
                    }
                }
            }
            output_path = pat;
            
        }

    } else if (c == 7){
            int argsGroup[3] ={1,3,5};
            for(int init=0;init<3;init++){
              if(strcmp(v[argsGroup[init]],"-c")==0){
                cIndex=argsGroup[init];
              }
              else if(strcmp(v[argsGroup[init]],"-d")==0){
                secIndex=argsGroup[init];
              }
              else if(strcmp(v[argsGroup[init]],"-o")==0){
                trdIndex=argsGroup[init];
              }
            }

            //-d
            char pat[300];
            int matchingIndex;
            if(strstr(v[secIndex+1],cwd)!=NULL){
                matchingIndex = strlen(cwd);
            }
            else matchingIndex = 0 ;

            if(strlen(v[secIndex+1])==matchingIndex)
                    pat[0]='.';
            else{

                strcpy(pat,v[secIndex+1]+matchingIndex);

                if(strlen(pat)<=1) {
                    if(strlen(pat)==0)
                        pat[0]='.';
                    if(pat[0]==' ')pat[0]='.';
                }
                else {
                    int loopindex = 1;
                    if(pat[0]=='.');
                    else if(pat[0]=='/'){
                        newStr[0] = '.';
                        for (; loopindex < strlen(pat)+1; loopindex++){
                            newStr[loopindex] = pat[loopindex-1];
                        }

                        loopindex = 0;
                        for (; loopindex < strlen(newStr); loopindex++){
                            pat[loopindex] = newStr[loopindex];
                        }
                    }
                }
            }

            path = pat;
        //======================================

        // -o
            char pato[300];
            matchingIndex=0;
            if(strstr(v[trdIndex+1],cwd)!=NULL){
                matchingIndex = strlen(cwd);
                }
            else matchingIndex = 0 ;

            if(strlen(v[trdIndex+1])==matchingIndex)
                    pato[0]='.';
            else{

                strcpy(pato,v[trdIndex+1]+matchingIndex);

                if(strlen(pato)<=1) {
                    if(strlen(pato)==0)
                        pato[0]='.';
                    if(pato[0]==' ')pato[0]='.';
                    //pato[2]='\0';
                }
                else {
                    int loopindex = 1;
                    if(pato[0]=='.');
                    else if(pato[0]=='/'){
                        newStr2[0] = '.';
                        for (; loopindex < strlen(pato)+1; loopindex++){
                            newStr2[loopindex] = pato[loopindex-1];
                        }

                        loopindex = 0;
                        for (; loopindex < strlen(newStr2); loopindex++){
                            pato[loopindex] = newStr2[loopindex];
                        }
                    }
                }
            }
            output_path = pato;
            
    }

    // get the sort value type
    sort_value_type = v[cIndex+1];
/*
    pDir = opendir (path);
    if (output_path == NULL){
        output_path = path;
    }
*/

    //struct ArgsForRecur *recurArgs = malloc(1000*sizeof(char));
    //recurArgs->path = strdup(path);
    
    //recurArgs->output_path = strdup(output_path);
    char tmp_path_argIn[500] = ".";
    if (output_path == NULL){
        output_path = path;
    }
    recur((void *)tmp_path_argIn);
    
    int status = 0;
    pid_t wpid;
    while ((wpid = wait(&status)) > 0)
    {
        printf("%d,",wpid);
        *pCounter+=1;
    }

    return 0;
}