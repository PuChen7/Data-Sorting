/*
    C ECHO client example using sockets
*/
#include<stdio.h> //printf
#include<string.h>    //strlen
#include<sys/socket.h>    //socket
#include<arpa/inet.h> //inet_addr

int main(int argc , char *argv[])
{
    int sock;
    struct sockaddr_in server;
    char message[1000] ;
    char *server_reply;
    //Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");

    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons( atoi(argv[2]) );

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

    server_reply = malloc(2000);
    if( recv(sock , server_reply , 2000 , 0) < 0)
    {
        puts("recv failed");
    }
    puts("Server Head2 :");
    puts(server_reply);
    free(server_reply);

    //keep communicating with server


    scanf("%s" , message);
    //Send some data
    if(send(sock , message , strlen(message) , 0) < 0)
    {
        puts("Send failed");
        return 1;
    }

    server_reply = malloc(2000);

    if(recv(sock , server_reply , 2000 , 0) < 0)
    {
        puts("recv failed");
        return 2;
    }
    puts("Server replies :");
    puts(server_reply);
    free(server_reply);


    close(sock);
    return 0;
}
