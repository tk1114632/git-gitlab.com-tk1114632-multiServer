///////////////////////////////////////////
//Server for transferring files
//Mingheng Song
///////////////////////////////////////////
#include<stdio.h>
#include<string.h>    //strlen
#include<stdlib.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write
#include<pthread.h> //for threading , link with lpthread
 
#define 	BUFFERSIZE 			2000
#define 	PORT 				8888
#define		FILENAMELENGTHMAX	1024

//the thread function
void *connection_handler(void *);
 
int main(int argc , char *argv[])
{
    int socket_desc , client_sock , c , *new_sock;
    struct sockaddr_in server , client;
     
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");
     
    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( PORT );
     
    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("Bind Failed. Error");
        return 1;
    }
    puts("Bind Done!");
     
    //Listen
    listen(socket_desc , 3);     
     
    //Accept and incoming connection
    puts("Waiting for connections...");
    c = sizeof(struct sockaddr_in);
    while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
    {
        puts("Connection accepted");
         
        pthread_t sniffer_thread;
        new_sock = malloc(1);
        *new_sock = client_sock;
        
        if( pthread_create( &sniffer_thread , NULL ,  connection_handler , (void*) new_sock) < 0)
        {
            perror("Create Thread Failed");
            return 1;
        }
         
        //Now join the thread , so that we dont terminate before the thread
        //pthread_join( sniffer_thread , NULL);
        puts("Handler assigned");
    }
     
    if (client_sock < 0)
    {
        perror("Acception Failed");
        return 1;
    }
     
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
    char *message , client_message[BUFFERSIZE];
     
    bzero(client_message, BUFFERSIZE);
    while( (read_size = recv(sock, client_message, BUFFERSIZE,0)) < 0 )
    {
    	char RetryMessage[] = "Please retry.";
    	write(sock, RetryMessage, strlen(RetryMessage));
    }

    //read the file name
    char file_name[FILENAMELENGTHMAX+1];
    bzero(file_name, strlen(file_name));
    strncpy(file_name, client_message, 
    	strlen(client_message) > FILENAMELENGTHMAX ? FILENAMELENGTHMAX : strlen(client_message));

    //read the file on server and transfer it
    FILE *fp = fopen(file_name, "r");
    if (fp == NULL)
		{
			printf("File:\t%s Not Found!\n", file_name);
		}
	else {
		char fileBuffer[BUFFERSIZE];
		bzero(fileBuffer, BUFFERSIZE);
		int file_block_length = 0;
			while( (file_block_length = fread(fileBuffer, sizeof(char), BUFFERSIZE, fp)) > 0)
			{
				printf("file_block_length = %d\n", file_block_length);

				if (send(sock, fileBuffer, file_block_length, 0) < 0)
				{
					printf("File Send Failed!\n");
					break;
				}

				bzero(fileBuffer, sizeof(fileBuffer));
			}
			fclose(fp);
			printf("File Transfer Finished!\n");
	}
	//Close socket after completing tranferring
	close(sock);
     
    if(read_size == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("recv failed");
    }
    
    //Free the socket pointer
    free(socket_desc);
    return 0;
}
