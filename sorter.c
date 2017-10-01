#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#define sorter_header "sorter.h"
#include sorter_header
#include <ctype.h>
#define EMPTY_STRING ""


struct node{
    struct node *next;  // pointer points to the next row
    char **line_array;  // hold the entire row
};

typedef struct{
    int index;  // hold the original index number
    char* str;  // hold the string value
} SortArray;

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

//global value to for column and row
int static dataRow,dataCol;
void initDataArray(char* array[dataRow][dataCol],struct node * data){
    for(int i = 0;i<dataRow;i++){
        for(int j = 0; j< dataCol ;j++){
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

// helper function for sorting numbers
void merge(SortArray* sort_array, int left, int middle, int right,int numeric){
    int i, j, k;
    int n1 = middle - left + 1;
    int n2 =  right - middle;
    
    // temporary arrays
    SortArray *L;
    L = (SortArray*) malloc(n1 * sizeof(SortArray));
    SortArray *R;
    R = (SortArray*) malloc(n2 * sizeof(SortArray));

    // copy data into temporary arrays
    for (i = 0; i < n1; i++){
        L[i].str = sort_array[left + i].str;
        L[i].index = sort_array[left + i].index;
    }
    for (j = 0; j < n2; j++){
        R[j].str = sort_array[middle + 1+ j].str;
        R[j].index = sort_array[middle + 1 + j].index;
    }

    i = 0; 
    j = 0; 
    k = left;

    while (i < n1 && j < n2) {
        if(numeric==0){//if str
            int cmpResult = strcmp(L[i].str,R[j].str);
            if (cmpResult<=0) {
                sort_array[k].str = L[i].str;
                sort_array[k].index = L[i].index;
                i++;
            }
            else {
                sort_array[k].str = R[j].str;
                sort_array[k].index = R[j].index;
                j++;
            }
        }
        else{//if numeric
            if (atoi(L[i].str) <= atoi(R[j].str)) {
                sort_array[k].str = L[i].str;
                sort_array[k].index = L[i].index;
                i++;
            }
            else {
                sort_array[k].str = R[j].str;
                sort_array[k].index = R[j].index;
                j++;
            }
        }
        k++;
    }

    while (i < n1) {
        sort_array[k].str = L[i].str;
        sort_array[k].index = L[i].index;
        i++;
        k++;
    }

    while (j < n2) {
        sort_array[k].str = R[j].str;
        sort_array[k].index = R[j].index;
        j++;
        k++;
    }

}

// function for sorting numbers
void mergeSort(SortArray* sort_array, int left, int right,int numeric){
    if (left < right){
        int middle = left + (right - left) / 2;
        mergeSort(sort_array, left, middle,numeric);
        mergeSort(sort_array, middle+1, right,numeric);
        merge(sort_array, left, middle, right,numeric);
    }
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
    // get the sort value type
    char* sort_value_type = argv[2];

    //extra credit, ./a.exe -c [col] -a [condition] is the entire format to run
    //[condition] : if we do ./a.exe -c actor -a ="Charlize Theron", this program is going to return all rows that contains "Charlize Theron" in
    //"actor" field. It is like a reduct version of Mysql written by C.
    char * analyzeCmd =NULL;
    char * analyzeCond=NULL;
    if(argc == 5){
    analyzeCmd = argv[3];
    analyzeCond = argv[4];
    }
    int analyzeFlag = 0;
    if(analyzeCmd!= NULL && strcmp(analyzeCmd,"-a")==0)analyzeFlag=1;

    

    // head of the Linked List
    struct node *head = NULL;
    struct node *prev = NULL;

    char line[1024];    // temp array for holding csv file lines.
    int rowNumber=-1;    // hold the number of lines of the csv file.
    int value_type_number;    // hold the number of value types(column numbers).
    char* headerLine;   // hold the first line of the csv file, which is the value types.
    int isFirstElement = 0; // mark the first element in LL

    // loop for reading the csv file line by line.
    while (fgets(line, 1024, stdin)){
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
                    token[strlen(token)-1]=0;//make it end of string         
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
                token[strlen(token)-1]=0;//make it end of string         
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
        if (strcmp(headerArray[i], sort_value_type) == 0){
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
    //---analyze part----//
    int analyzeOperator = 10;//0 means =. 1 means >, -1 means <
    int analyzeCondNumeric = 10;

    while (count < rowNumber){
        //printf("index %d and str %s \n",count,dataArray[count][i]);   
        if(analyzeFlag==1 && analyzeOperator==10)//it is an analyze command
        {
            analyzeOperator = analyzeCond[0]=='='?0:analyzeCond[0]=='+'?1:analyzeCond[0]=='-'?-1:10;
            //move pointer ahead to get the rest condition
            analyzeCond++;   
            if(isNumeric(analyzeCond)==0)   analyzeCondNumeric=0;
            else      analyzeCondNumeric=1; 
        }     

        int analyzeSortFlag = 0;
        if(analyzeFlag==1){
            switch(analyzeOperator){
                case -1://operator <
                        if(analyzeCondNumeric==1){//if numeric
                            if(atoi(dataArray[count][i])<atoi(analyzeCond))analyzeSortFlag=1;
                        }
                        else{//if string
                            if(strcmp(dataArray[count][i],analyzeCond)<0)analyzeSortFlag=1;                            
                        }
                        break;
                case 0://operator =
                        if(analyzeCondNumeric==1){//if numeric
                            if(atoi(dataArray[count][i])==atoi(analyzeCond))analyzeSortFlag=1;
                        }
                        else{//if string
                            if(strcmp(dataArray[count][i],analyzeCond)==0)analyzeSortFlag=1;                            
                        }
                        break;
                case 1://operator >
                        if(analyzeCondNumeric==1){//if numeric
                            if(atoi(dataArray[count][i])>atoi(analyzeCond))analyzeSortFlag=1;
                        }
                        else{//if string
                            if(strcmp(dataArray[count][i],analyzeCond)>0)analyzeSortFlag=1;                            
                        }
                        break;
                default:
                        printf("something wrong with analyze oper");
                        break;
            }
            if(analyzeSortFlag==1){
                sort_array[sortArraycount].index = count;
                sort_array[sortArraycount].str = dataArray[count][i];
                numericFlag += isNumeric(sort_array[sortArraycount].str);
                
                sortArraycount++;

            }
        }else{
            sort_array[count].index = count;
            sort_array[count].str = dataArray[count][i];
            numericFlag += isNumeric(sort_array[count].str);
        }
        //printf("%s count:%d\n",sort_array[sortArraycount].str,sortArraycount);        
        count++; 
    }

    //printf("%d %s how many rows :%d\n",analyzeOperator,analyzeCond,sortArraycount);
    // check if the value is digits or string
    // return 0 for false, non-zero for true.
    int numeric = numericFlag;

    //printf("before %d %d %d\n",sort_array[0].index,sort_array[1].index,sort_array[2].index);
    
    // if the string is a number, then sort based on the value of the number
    // NOTE: numeric 0:false 1:true
    int MAXROW=rowNumber-1;
    if(analyzeFlag==1)MAXROW=sortArraycount;

    printf("col1:%s col2:%s\n",sort_array[0].str,sort_array[1].str);
    
    if(MAXROW>0)
        mergeSort(sort_array, 0, MAXROW,numeric); 
    
    printf("col1:%s col2:%s\n",sort_array[0].str,sort_array[1].str);
         
    if(MAXROW==0)printf("no data satisfying this condition");
    
    count=0;
    for(;count<MAXROW;count++){
        for(i=0;i<dataCol;i++){
            i==(dataCol-1)?printf("%s\n",dataArray[sort_array[count].index][i]):printf("%s,",dataArray[sort_array[count].index][i]);
        }//end of line
    }
    

    return 0;
}
