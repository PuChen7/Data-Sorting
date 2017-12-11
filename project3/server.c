#include<stdio.h>
#include<string.h>    //strlen
#include<stdlib.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write
#include "mergesort.c"
#include<pthread.h> //for threading , link with lpthread
#define SESSION_MSG "session_msg"

SortArray *entire;
int index_entire = 0;
int num_of_rows = 0;
char* sort_value_type;
char* header[28];

pthread_mutex_t session_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sort_lock = PTHREAD_MUTEX_INITIALIZER;
int currentsessionID=0;
int sessionTotal=0;
char** sessionDict=NULL;
char* sort_value_type;

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


int isConnecting(char* ip){//1 for true, 0 for false
    int i=0;
    for(;i<sessionTotal;i++){
      if(sessionDict[i]!=NULL){
        if(strstr(sessionDict[i],ip)!=NULL){
          return 1;
        }
      }
    }
    return 0;
}
int main(int argc , char *argv[])
{
    entire = malloc(80000 * sizeof(SortArray));
    int i = 0;
    for(;i<80000;i++){
        entire[i].str = malloc(sizeof(char*)*28);
    }

    sessionDict = (char**)malloc(sizeof (char*) * 1000);
    int socket_desc , new_socket , c , *new_sock;
    struct sockaddr_in server , client;

    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);

    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( atoi(argv[2]) );

    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        puts("bind failed");
        return 1;
    }
    puts("bind done");

    //Listen
    listen(socket_desc , 128);

    //Accept and incoming connection
    printf("Received connections from: ");
    c = sizeof(struct sockaddr_in);
    while( (new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
    {
        //puts("Connection accepted");
        char clntName[INET_ADDRSTRLEN];

        if(inet_ntop(AF_INET,&client.sin_addr.s_addr,clntName,sizeof(clntName))!=NULL){
           if(!isConnecting(clntName)){
              printf("%s,",clntName);
              char* sessionMSG = malloc(sizeof (int)+sizeof (SESSION_MSG));
              sprintf(sessionMSG,"%d-%s",currentsessionID,SESSION_MSG);
              write(new_socket,sessionMSG,strlen(sessionMSG));
              sessionDict[currentsessionID++]=clntName;
              sessionTotal = currentsessionID;
              free(sessionMSG);
            }
        } else {
           printf("Unable to get address\n"); // i just fixed this to printf .. i had it as print before
        }        //Reply to the client

        //sleep(1);
        pthread_t sniffer_thread;
        new_sock = malloc(1);
        *new_sock = new_socket;

        if( pthread_create( &sniffer_thread , NULL ,  connection_handler , (void*) new_sock) < 0)
        {
            perror("could not create thread");
            return 1;
        }

        //Now join the thread , so that we dont terminate before the thread
        pthread_join( sniffer_thread , NULL);

        pthread_mutex_lock(&session_lock);
        sessionDict[currentsessionID--]=NULL;
        sessionTotal--;
        pthread_mutex_unlock(&session_lock);


    }

    if (new_socket<0)
    {
        perror("accept failed");
        return 1;
    }
    pthread_mutex_destroy(&session_lock);
    return 0;
}

/*
 * This will handle connection for each client
 * */
void *connection_handler(void *socket_desc){
    pthread_mutex_lock(&sort_lock);
    // printf("one socket come in\n" );
    //Get the socket descriptor
    int num_of_files = 0;
    int file_row[1000];

    int sock = *(int*)socket_desc;
    int read_size;
    //Receive a message from client
    char client_message[1024];
    char sendback_message[1024];
    int head_flag = 0;

    while( (read_size = read(sock , client_message , 1024 )) > 0 ){


        //Send the message back to client
        strcpy(sendback_message,client_message);
        char *p = strchr(sendback_message, '\n');


        if (!p) /* deal with error: / not present" */;
        *(p+1) = 0;

        // need to store header
        if (strstr(sendback_message, "director_name") && head_flag == 0){
          char* headtmp = strdup(sendback_message);
          char* head_tok = strtok_single(headtmp, ",");
          int head_count = 0;
          while (head_tok != NULL){
            header[head_count] = strdup(head_tok);

            head_count++;
            head_tok = strtok_single(NULL, ",");
          }
          head_flag = 1;
          write(sock , sendback_message , strlen(sendback_message));
          continue;
        } else if (strstr(sendback_message, "director_name") && head_flag == 1){
          write(sock , sendback_message , strlen(sendback_message));
          continue;
        }

        if(strstr(sendback_message,SORT_REQUEST)!=NULL){

          char* copy = strdup(sendback_message);
          char *breakdown = strchr(copy, '|');
          if (!breakdown) /* deal with error: / not present" */;
          *(breakdown) = 0;
          breakdown = strchr(copy, '-');
          char* sort_type = strdup(breakdown+1);
          sort_value_type = strdup(sort_type);
          if (!breakdown) /* deal with error: / not present" */;
          *(breakdown) = 0;
          breakdown = strchr(copy, '_');
          char* row_str = strdup(breakdown+1);
          if (!breakdown) /* deal with error: / not present" */;
          *(breakdown) = 0;
          int dataRow = atoi(row_str);
          int sessionID = atoi(copy);
          printf("\nsort_request with search value type :%s,dataRow:%d,session:%d\n",sort_type,dataRow,sessionID);
          // store the number of rows of the current file into array

          // update the total line numbers
          file_row[num_of_files] = dataRow;
          num_of_rows = num_of_rows + dataRow;


          num_of_files++;

          if (strstr(sendback_message, "sort_request")){
            continue;
          }
          //printf("%s\n",sendback_message);


          free(copy);
          free(sort_type);
          free(row_str);
        }
        if(strstr(sendback_message,DUMP_REQUEST)!=NULL){

          printf("\ndump request\n");
        } else {

            char* tmpstr = strdup(sendback_message);

            char *token = strtok_single(tmpstr, ",");

            char * tempStr;
            char  tempCell[100000];
            char *dummy = NULL;
            char * KillerQueen;
            int counter = 0;
            int headerDoubleQuotes = 0;
            //int value_type_number = 28;
            int tailerDoubleQuotes =0;
            //char** new_array = malloc(value_type_number * sizeof(char*));

            int token_count = 0;
            while (token != NULL){


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
                  //new_array[counter] = tempCell;
                  counter++;
              }
              else if(headerDoubleQuotes!= 1 && tailerDoubleQuotes!=1){
                  //new_array[counter] = *token ? trimwhitespace(token) : EMPTY_STRING; // store token into array
                  counter++;
              }

              entire[index_entire].str[token_count] = strdup(token);

              //printf("%s\n", entire[index_entire].str[token_count]);
              //printf("%s\n", token);
              token_count++;
              token = strtok_single(NULL, ",");
            }
            free(dummy);
            entire[index_entire].index = index_entire;
            //printf("%d\n", entire[index_entire].index = index_entire);
            index_entire++;
        }
        write(sock , sendback_message , strlen(sendback_message));
    }

    int print = 0;
    int print2 = 0;
    int flag = 0;

    // int tmpp = 0;
    // for (; tmpp < file_row[0]; tmpp++){
    //   printf("%s  ----------  %d\n", entire[tmpp].str[0], entire[tmpp].index);
    // }
    //printf("%s  ----------  %d\n", entire[0].str[0], entire[0].index);
    // for (; print2 < 28; print2++){
    //   printf("%s", entire[0].str[print2]);
    // }


    // for (; print < num_of_rows; print++){
    //   if (print == file_row[flag]){
    //     for (; print2 < 28; print2++){
    //         printf("%s", entire[print].str[print2]);
    //     }
    //     flag = 1;
    //   }
    //   print2 = 0;
    // }

    // decide which column to sort
    int i = 0;
    for(; i < 28; i++){
      if (strcmp(header[i], sort_value_type) == 0){
        break;
      }
    }

    // int filep = 0;
    // for (; filep < num_of_files; filep++){
    //   printf("%d\n", file_row[filep]);
    // }

    int sort_column = i;
    int file_count = 0;
    // first file should start at 1, second file start at 5046, third: 10091...
    int start_point = 0;

    int total_row = 0;
    while (file_count < num_of_files){
      int rowNumbers = 0;
      int index_for_sorting = 0;
      // store the column as an array
      SortArray *sort_array;
      sort_array = (SortArray*) malloc(file_row[file_count] * sizeof(SortArray));

      int sortArraycount=0;
      //a safer way to check if numeric
      int numericFlag = 0;
      int count = 0;

      while (rowNumbers < file_row[file_count]){

          sort_array[rowNumbers].index = index_for_sorting;
          sort_array[rowNumbers].str = entire[total_row].str[sort_column];
          //printf("%s\n", sort_array[rowNumbers].str);
          numericFlag += isNumeric(sort_array[rowNumbers].str);
          index_for_sorting++;  // update the index for sorting
          rowNumbers++;
          total_row++;

      }





      int numeric = numericFlag;

      // if the string is a number, then sort based on the value of the number
      // NOTE: numeric 0:false 1:true
      int MAXROW=file_row[file_count]-1;

      // int test = 0;
      // for (; test < MAXROW+1; test++){
      //     printf("%d\n", sort_array[test].index);
      // }
      if(MAXROW>=0){
          mergeSort(sort_array, 0, MAXROW,numeric);
      }

      int u = 0;
      for (; u < file_row[file_count]; u++){
        printf("%s  --------------  %d\n", sort_array[u].str, sort_array[u].index);
      }

      file_count++;
    }


    if(read_size == 0)
    {
        //puts("Client disconnected");
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        // perror("recv failed");
    }

    //Free the socket pointer
    free(socket_desc);


    // i = 0;
    // int j = 0;
    // for(;i<80000;i++){
    //   for (; j < 28; j++){
    //     free(entire[i].str[j]);
    //   }
    //   j = 0;
    // }
    //
    // free(entire);
    // pthread_mutex_unlock(&sort_lock);
    return 0;
}
