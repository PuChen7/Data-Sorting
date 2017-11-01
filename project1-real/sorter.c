#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "mergesort.c"
#include <ctype.h>
#include <unistd.h>
#include <dirent.h>
#include <locale.h>
#include <ftw.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <errno.h>

#define EMPTY_STRING ""
#define VALID_MOVIE_HEADER_NUMBER 28

//global
static int *pCounter;

pid_t init_pid;
char* sort_value_type;

char** hierArray;
static int* hierArrayCursor;
static pid_t *pidArray;
static int *pidArrayCursor;

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
    if (str == NULL || *str == '\0' || isspace(*str))
      return 1;
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

void sort_one_file(char* input_path,char* output_path){
  FILE    *input_file;
  FILE    *output_file;
  output_file = fopen(output_path, "w");
   if (output_file == NULL)
   {
        fprintf(stderr, "Error : Failed to open output_file - %s\n", strerror(errno));
        fclose(output_file);
        return;
   }
  input_file = fopen(input_path, "r");
  if (input_file == NULL)
  {
      fprintf(stderr, "Error : Failed to open entry file - %s\n", strerror(errno));
      fclose(output_file);
      fclose(input_file);
      return;
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
      for(int i = 0;i<value_type_number;i++){
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
  if(MAXROW>=0)
      mergeSort(sort_array, 0, MAXROW,numeric);

  //printf("col1:%s col2:%s\n",sort_array[0].str,sort_array[1].str);

  //print header
  count=0;
  for(;count<value_type_number;count++){
      count==(value_type_number-1)?
      fprintf(output_file, "%s\n", headerArray[count])
      :fprintf(output_file, "%s,", headerArray[count]);

  }
  //print content
  count=0;
  for(;count<MAXROW+1;count++){
      for(i=0;i<dataCol;i++){
          i==(dataCol-1)?
          fprintf(output_file, "%s\n",dataArray[sort_array[count].index][i])
          :fprintf(output_file, "%s,",dataArray[sort_array[count].index][i]);
      }//end of line
  }

  free(sort_array);

}

  // free Linked List
  struct node *tmp;
  while (head != NULL){
      tmp = head;
      head = head->next;
      for(int i = 0;i<value_type_number;i++){
        free(tmp-> line_array[i]);
      }
      free(tmp-> line_array);
      free(tmp);
  }

  free(temp);
  free(headerLine);
  fclose(output_file);
  fclose(input_file);
}
void recur(DIR *pDir, struct dirent *pDirent, char* path, char* output_path){
    
    pid_t pid,fpid=getppid();
    while ((pDirent = readdir(pDir)) != NULL) {
        if (pDirent->d_name[0] == '.'){continue;}
        //printf("scanning file %s  with pid [%d] +[%d]|", pDirent->d_name,getpid(),*pidArrayCursor);
        if(strchr(pDirent->d_name, '.') != NULL ){
          if(path&&strcmp("csv",get_filename_ext(pDirent->d_name))==0 && strstr(pDirent->d_name,"-sorted")==NULL){//found csv
            /*NOTE create path for input and output file, and perform Mergesort on each csv file
            */
            printf("********** %s\n", output_path);
            int outputLength=0;
            if (strlen(output_path) == 0){
                outputLength = strlen(path);
            } else {
                outputLength = strlen(output_path);
            }
            char  outP[outputLength];
            char  inP[strlen(path)];
            strcpy(outP,output_path);
            strcpy(inP,path);
            char* fileNoExtension = strdup(remove_ext(pDirent->d_name));
            char * outputPath = strcat(strcat(strcat(outP,"/"),strcat(fileNoExtension,"-sorted")),".csv");
            char* inputPath = strcat(strcat(inP,"/"),strcat(pDirent->d_name,".csv"));
            int headerNumber = count_header(inputPath);

            // if(headerNumber!=VALID_MOVIE_HEADER_NUMBER){
            //   //printf("%d headers for [%s], we need 28\n",headerNumber,inputPath);
            //   continue;
            // }

            fpid = fork();
            if(fpid<0){}
            else if(fpid==0){//child
              //printf("found file %s with pid [%d] [%d] parent[%d] with outputPath %s\n",
              //pDirent->d_name,getpid(),pid,getppid(),outputPath);

              sort_one_file(inputPath,outputPath);

              free(fileNoExtension);

              break;
            }
            else{//parent
              int counter=0;
              int sum =0;
              for(;counter<*pidArrayCursor;counter++){
                sum+=pidArray[counter];
                  if(getpid()<=init_pid+pidArray[counter]){
                    pidArray[*pidArrayCursor+1]=pidArray[*pidArrayCursor+1]+1;
                    //printf("%d\n",pidArray[*pidArrayCursor+1] );

                    //*pidArrayCursor = *pidArrayCursor+1;
                    break;
                  }
              }
              if(getpid()>init_pid+pidArray[*pidArrayCursor]){
                printf("before %d \n",*pidArrayCursor);
                *pidArrayCursor = *pidArrayCursor+1;
                printf("after %d \n",*pidArrayCursor);
              }

              printf("parent : %d | current : %d | file :%s\n",getpid(),fpid,pDirent->d_name);

              free(fileNoExtension);
              continue;
            }


          }
      }

        if (strchr(pDirent->d_name, '.') == NULL){//search for folder
          pid = fork();
            if (pid < 0){
                printf("error\n");
                return;
            } else if (pid == 0){   // child,aka entered directory

                  //

                //printf("enter directory %s with pid[%d]\n", pDirent->d_name,getpid());


                strcat(path, "/");
                DIR *newdir = opendir(strcat(path,pDirent->d_name));
                recur(newdir, pDirent, path, output_path);

                break;
            } else if (pid > 0){    // parent ,aka found the directory
                //printf("found directory %s with pid[%d]\n", pDirent->d_name,getpid());
                // pidArray[*pidArrayCursor]=getpid();
                // pidArray[*pidArrayCursor+1]=pid;
                // *pidArrayCursor=*pidArrayCursor+2;
                // hierArray[*hierArrayCursor]=pDirent->d_name;
                // *hierArrayCursor=*hierArrayCursor+1;
                int counter=0;
                for(;counter<*pidArrayCursor;counter++){
                    if(getpid()<=init_pid+pidArray[counter]){
                      pidArray[*pidArrayCursor+1]+=1;
                      printf("%d\n",pidArray[*pidArrayCursor+1] );
                      //*pidArrayCursor = *pidArrayCursor+1;
                      break;
                    }
                }
                printf("parent : %d | current : %d | file :%s\n",getpid(),pid, pDirent->d_name);
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


    // hierArrayCursor = mmap(NULL, sizeof *hierArrayCursor, PROT_READ | PROT_WRITE,
    //                     MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    // *hierArrayCursor = 0;
    //
    // hierArray =(char**) malloc(256 * sizeof (char*));
    //
    pidArray = mmap(NULL, sizeof *pidArray * 256, PROT_READ | PROT_WRITE,
                            MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    pidArray[0]=0;
    pidArrayCursor= mmap(NULL, sizeof *pidArrayCursor, PROT_READ | PROT_WRITE,
                            MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *pidArrayCursor=0;

    getcwd(cwd, sizeof(cwd));
    fprintf(stdout, "Current working dir: %s\n", cwd);
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
    // get the sort value type
    sort_value_type = v[2];

    char path_tmp[300];
    char* path;
    char path_modified[300];
    char output_tmp[300];
    char output_final[300];
    char* output_path = NULL;
    char newStr[300];
    char newStr2[300];
    
    if (c == 3){
        // current dir
        path_tmp[0] = '.';
        path = path_tmp;
    } else if (c == 5){
        if(strcmp(v[3],"-d")==0){
            // -d
            char pat[300];
            int matchingIndex;
            if(strstr(v[4],cwd)!=NULL){
                matchingIndex = strlen(cwd);
                }
            else matchingIndex = 0 ;

            if(strlen(v[4])==matchingIndex)
                    pat[0]='.';
            else{

                strcpy(pat,v[4]+matchingIndex);
                
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
        }else if(strcmp(v[3],"-o")==0){
            // -o
            path_tmp[0] = '.';
            path = path_tmp;
            char pat[300];
            int matchingIndex;
            if(strstr(v[4],cwd)!=NULL){
                matchingIndex = strlen(cwd);
                }
            else matchingIndex = 0 ;

            if(strlen(v[4])==matchingIndex)
                    pat[0]='.';
            else{

                strcpy(pat,v[4]+matchingIndex);
                
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
          //-d
            char pat[300];
            int matchingIndex;
            if(strstr(v[4],cwd)!=NULL){
                matchingIndex = strlen(cwd);
                }
            else matchingIndex = 0 ;

            if(strlen(v[4])==matchingIndex)
                    pat[0]='.';
            else{

                strcpy(pat,v[4]+matchingIndex);
                
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
            if(strstr(v[6],cwd)!=NULL){
                matchingIndex = strlen(cwd);
                }
            else matchingIndex = 0 ;

            if(strlen(v[6])==matchingIndex)
                    pato[0]='.';
            else{

                strcpy(pato,v[6]+matchingIndex);
                
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

    pDir = opendir (path);
    printf("Initial PID : %d\n", (init_pid=getpid()));
    if (output_path == NULL){
        output_path = path;
    }
    recur(pDir, pDirent, path, output_path);
    int status = 0;
    pid_t wpid;
    while ((wpid = wait(&status)) > 0)
    {
        printf("%d,",wpid);
        *pCounter+=1;
    }


    if(getpid()==init_pid){
      wait(NULL);
      printf("\nTotal Child Processes Number: %d  \n",*pCounter);

      //
      int counter=0;
      for(;counter<*pidArrayCursor;counter++){
        printf("p : %d \n",pidArray[counter]);
      }
      //
      // free(hierArray);
      munmap(pidArray, sizeof *pidArray);
      munmap(pidArrayCursor, sizeof *pidArrayCursor);
      // munmap(hierArrayCursor, sizeof *hierArrayCursor);
      munmap(pCounter, sizeof *pCounter);
    }

    return 0;
}
