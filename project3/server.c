#include<stdio.h>
#include<string.h>    //strlen
#include<stdlib.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write
#include "mergesort.c"
#include<pthread.h> //for threading , link with lpthread
#define SESSION_MSG "session_msg"
pthread_mutex_t session_lock = PTHREAD_MUTEX_INITIALIZER;
int currentsessionID=0;
int sessionTotal=0;
char** sessionDict=NULL;
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
void *connection_handler(void *socket_desc)
{
    // printf("one socket come in\n" );
    //Get the socket descriptor
    int sock = *(int*)socket_desc;
    int read_size;

    //Receive a message from client
    char client_message[1024];
    char sendback_message[1024];
    while( (read_size = read(sock , client_message , 1024 )) > 0 )
    {
        //Send the message back to client
        strcpy(sendback_message,client_message);
        char *p = strchr(sendback_message, '\n');
        if (!p) /* deal with error: / not present" */;
        *(p+1) = 0;

        if(strstr(sendback_message,SORT_REQUEST)!=NULL){
          char* copy = strdup(sendback_message);
          char *breakdown = strchr(copy, '|');
          if (!breakdown) /* deal with error: / not present" */;
          *(breakdown) = 0;
          breakdown = strchr(copy, '-');
          char* sort_type = strdup(breakdown+1);
          if (!breakdown) /* deal with error: / not present" */;
          *(breakdown) = 0;
          breakdown = strchr(copy, '_');
          char* row_str = strdup(breakdown+1);
          if (!breakdown) /* deal with error: / not present" */;
          *(breakdown) = 0;
          int dataRow = atoi(row_str);
          int sessionID = atoi(copy);
          printf("\nsort_request with search value type :%s,dataRow:%d,session:%d\n",sort_type,dataRow,sessionID);
          free(copy);
          free(sort_type);
          free(row_str);
        }
        if(strstr(sendback_message,DUMP_REQUEST)!=NULL){
          printf("\ndump request\n");
        }

        write(sock , sendback_message , strlen(sendback_message));
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

    return 0;
}
