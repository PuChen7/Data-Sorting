#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "mergesort.c"
#include <ctype.h>
#include <unistd.h>
#include <dirent.h>
#include <locale.h>
#include <string.h>
#include <ftw.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/types.h>

#define EMPTY_STRING ""

static int *stdoutFlag;
static int *pCounter;
pid_t init_pid;

static int argCounter;
static char** argVariables;
static char* sort_value_type;


void sort_one_file(char* input_path,char* output_path){
  FILE    *input_file;
  FILE    *output_file;

  output_file = fopen(output_path, "w");
   if (output_file == NULL)
   {
        fprintf(stderr, "Error : Failed to open output_file - %s\n", strerror(errno));
        fclose(output_file);
        exit(0);
   }
  input_file = fopen(input_path, "r");
  if (input_file == NULL)
  {
      fprintf(stderr, "Error : Failed to open entry file - %s\n", strerror(errno));
      fclose(output_file);
      fclose(input_file);
      exit(0);
  }

  // head of the Linked List
  struct node *head = NULL;
  struct node *prev = NULL;

  char line[1024];    // temp array for holding csv file lines.
  int rowNumber=-1;    // hold the number of lines of the csv file.
  int value_type_number;    // hold the number of value types(column numbers).
  char* headerLine;   // hold the first line of the csv file, which is the value types.
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
      while (token != NULL){
          if(token[strlen(token)-1] == '\n'){
              int len = strlen(token);
              token[len-1]='\0';//make it end of string
          }

          char *tempStr = trimwhitespace(token);
          char *dummy = NULL;
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

      // create a new node
      // rowNumber starts from 1
      struct node *temp = (struct node*) malloc(sizeof(struct node));
      temp-> line_array = new_array;
      temp-> next = NULL;

      if (isFirstElement == 0){
          temp-> next = head;
          head = temp;
          isFirstElement = 1;
          prev = head;
          continue;
      }
      prev-> next = temp;
      prev = temp;
  }

  char* headerArray[value_type_number];   // this array hold the first row.
  initValueTypesArray(headerArray,value_type_number,headerLine);
  //printf("%s\n",headerArray[2]);

  //resuage of temp to 'copy' a head, then pass it to initDataArray to store 2d data array
  struct node *temp = (struct node*) malloc(sizeof(struct node));
  temp-> line_array = head->line_array;
  temp-> next = head -> next;
  //[row][column]
  dataCol= value_type_number;
  dataRow = rowNumber;
  char* dataArray[dataRow][dataCol];
  initDataArray(dataArray,temp);

  // free Linked List
  struct node *tmp;
  while (head != NULL){
      tmp = head;
      head = head->next;
      free(tmp);
  }

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
      printf("Error: The value type was not found in csv file!\n");
      exit(0);
  }

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

  //printf("%d %s how many rows :%d\n",analyzeOperator,analyzeCond,sortArraycount);
  // check if the value is digits or string
  // return 0 for false, non-zero for true.
  int numeric = numericFlag;


  // if the string is a number, then sort based on the value of the number
  // NOTE: numeric 0:false 1:true
  int MAXROW=rowNumber-1;

  if(MAXROW>=0)
      mergeSort(sort_array, 0, MAXROW,numeric);

  //NOTE write to file
  //NOTE -o

  //print header
  count=0;
  for(;count<value_type_number;count++){
      count==(value_type_number-1)?
      fprintf(output_file, "%s\n", headerArray[count])
      :fprintf(output_file, "%s", headerArray[count]);

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

  /* When you finish with the file, close it */
  fclose(input_file);
  fclose(output_file);
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

    while ((pDirent = readdir(pDir)) != NULL) {
        if (pDirent->d_name[0] == '.'){continue;}
        //printf("scanning file %s  with pid [%d] +[%d]|", pDirent->d_name,getpid(),*pidArrayCursor);
        if(strchr(pDirent->d_name, '.') != NULL ){
          if(strcmp("csv",get_filename_ext(pDirent->d_name))==0 && strstr(pDirent->d_name,"-out")==NULL){//found csv
            fpid = fork();
            if(fpid<0){}
            else if(fpid==0){//child

              /*NOTE create path for input and output file, and perform Mergesort on each csv file
              */
              char  outP[strlen(path)];
              char  inP[strlen(path)];
              strcpy(outP,path);
              strcpy(inP,path);
              char * outputPath = strcat(strcat(strcat(outP,"/"),strcat(remove_ext(pDirent->d_name),"-out")),".csv");
              char* inputPath = strcat(strcat(inP,"/"),pDirent->d_name);

              sort_one_file(inputPath,outputPath);

              //printf("found file %s with pid [%d] [%d] parent[%d] \n", pDirent->d_name,getpid(),pid,getppid());
              // !important printf
              //printf("[%d],%s\n",getpid(),pDirent->d_name);
              sleep(1);
              exit(0);
            }
            else{//parent

              continue;
            }

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
      return 0;
    char * p;
    strtod (str, &p);
    return *p == '\0';
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

int main(int argc, char** argv){
    // check for command line input
    if (argc < 3){
        printf("error");
        exit(0);
    }

    //NOTE:copy and setup global parameters in case latter we need it
    sort_value_type = argv[2];
    argCounter = argc;
    argVariables = malloc(argc * sizeof(char*));

    int tempC = 0;
    for(;tempC<argc;tempC++){
      argVariables[tempC]=argv[tempC];
    }

    struct dirent *pDirent;
    DIR *pDir;
    char cwd[1024];
    pCounter = mmap(NULL, sizeof *pCounter, PROT_READ | PROT_WRITE,
                        MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *pCounter = 0;

    stdoutFlag = mmap(NULL, sizeof *stdoutFlag, PROT_READ | PROT_WRITE,
                        MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *stdoutFlag = 0;

    //NOTE we should handle some default behaviour like no -d or -o was input
    //NOTE getcwd get current dir. we can chdir("..") and then perform traverse dir
    //NOTE -d , up to you

    if(argc==3){
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

    // ./a.o -c <column> -d dir -otherpara
    pDir = opendir (argv[4]);
    char path[200];
    strncpy(path, argv[4], strlen(argv[4]));
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

    //Here we reap children and print out their PID's
    int status = 0;
    pid_t wpid;
    while ((wpid = wait(&status)) > 0)
    {
        printf("%d,",wpid);
        *pCounter+=1;
    }

    //Last line to print total number PID and unmap shared memory
    if(getpid()==init_pid){
      wait(NULL);
      printf("\nTotal Child Processes Number: %d \n",*pCounter);
      int counter=0;

      munmap(pCounter, sizeof *pCounter);
      munmap(stdoutFlag, sizeof *stdoutFlag);
    }

    return 0;
}
