#include<stdio.h>
#include<string.h>    //strlen
#include<stdlib.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write

#include<pthread.h> //for threading , link with lpthread

void *connection_handler(void *);
pthread_mutex_t csv_lock = PTHREAD_MUTEX_INITIALIZER;

int main(int argc , char *argv[])
{
    int socket_desc , new_socket , c , *new_sock;
    struct sockaddr_in server , client;
    char *message;

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
    listen(socket_desc , 3);

    //Accept and incoming connection
    printf("Received connections from: ");
    c = sizeof(struct sockaddr_in);
    while( (new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
    {
        //puts("Connection accepted");
        char clntName[INET_ADDRSTRLEN];

        if(inet_ntop(AF_INET,&client.sin_addr.s_addr,clntName,sizeof(clntName))!=NULL){
           printf("%s,",clntName);
        } else {
           printf("Unable to get address\n"); // i just fixed this to printf .. i had it as print before
        }        //Reply to the client

        message = "Hello Client , I have received your connection. And now I will assign a handler for you\n";
        write(new_socket , message , strlen(message));

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
        //puts("Handler assigned");

    }

    if (new_socket<0)
    {
        perror("accept failed");
        return 1;
    }
    pthread_mutex_destroy(&csv_lock);
    return 0;
}

/*
 * This will handle connection for each client
 * */
void *connection_handler(void *socket_desc)
{
    //Get the socket descriptor
    int sock = *(int*)socket_desc;
    int read_size;

    //Send some messages to the client
    //message = "Greetings! I am your connection handler\n";

    // //message = "Now type something and i shall repeat what you type \n";
    // strcpy(message,"");
    // write(sock , message , strlen(message));

    //Receive a message from client
    char client_message[1024];
    char sendback_message[1024];
    // pthread_mutex_lock(&csv_lock);
    while( (read_size = read(sock , client_message , 1024 )) > 0 )
    {
        //Send the message back to client
        //printf("%d\n",strlen(client_message));
        //printf("%s",client_message);
        strcpy(sendback_message,client_message);
        char *p = strchr(sendback_message, '\n');
        if (!p) /* deal with error: / not present" */;
        *(p+1) = 0;
        printf("%s",sendback_message);

        write(sock , sendback_message , strlen(sendback_message));
    }

    if(read_size == 0)
    {
        //puts("Client disconnected");
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("recv failed");
    }

    //Free the socket pointer
    free(socket_desc);
    // pthread_mutex_unlock(&csv_lock);

    return 0;
}
