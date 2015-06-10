//////////////////////////////////////////////////////
// clients for tranferring files
// Mingheng Song
// ///////////////////////////////////////////////////
#include<netinet/in.h>                         // for sockaddr_in
#include<sys/types.h>                          // for socket
#include<sys/socket.h>                         // for socket
#include<stdio.h>                              // for printf
#include<stdlib.h>                             // for exit
#include<string.h>                             // for bzero

#define PORT       8888
#define BUFFERSIZE                   2000
#define FILE_NAME_MAX_SIZE            1024

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		printf("Usage: ./%s ServerIPAddress\n", argv[0]);
		exit(1);
	}

	// Set up socket
	struct sockaddr_in client_addr;
	bzero(&client_addr, sizeof(client_addr));
	client_addr.sin_family = AF_INET; 
	client_addr.sin_addr.s_addr = htons(INADDR_ANY); 
	client_addr.sin_port = htons(0); 

	// build a socket
	int client_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (client_socket < 0)
	{
		printf("Create Socket Failed!\n");
		exit(1);
	}

	// bind socket
	if (bind(client_socket, (struct sockaddr*)&client_addr, sizeof(client_addr)))
	{
		printf("Client Bind Port Failed!\n");
		exit(1);
	}

	// set up server address
	struct sockaddr_in  server_addr;
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;

	// Input IP address
	if (inet_aton(argv[1], &server_addr.sin_addr) == 0)
	{
		printf("Server IP Address Error!\n");
		exit(1);
	}

	server_addr.sin_port = htons(PORT);
	socklen_t server_addr_length = sizeof(server_addr);

	// connect to the Server
	if (connect(client_socket, (struct sockaddr*)&server_addr, server_addr_length) < 0)
	{
		printf("Can Not Connect To %s!\n", argv[1]);
		exit(1);
	}

	char file_name[FILE_NAME_MAX_SIZE + 1];
	bzero(file_name, sizeof(file_name));
	printf("Please Input File Name On Server.\t");
	scanf("%s", file_name);

	char buffer[BUFFERSIZE];
	bzero(buffer, sizeof(buffer));
	strncpy(buffer, file_name, strlen(file_name) > BUFFERSIZE ? BUFFERSIZE : strlen(file_name));
	send(client_socket, buffer, BUFFERSIZE, 0);

	FILE *fp = fopen(file_name, "w");
	if (fp == NULL)
	{
		printf("File:\t%s Can Not Open To Write!\n", file_name);
		exit(1);
	}
	//receive file
	bzero(buffer, sizeof(buffer));
	int length = 0;
	while(length = recv(client_socket, buffer, BUFFERSIZE, 0))
	{
		if (length < 0)
		{
			printf("Recieve Data From Server %s Failed!\n", argv[1]);
			break;
		}

		int write_length = fwrite(buffer, sizeof(char), length, fp);
		if (write_length < length)
		{
			printf("File:\t%s Write Failed!\n", file_name);
			break;
		}
		bzero(buffer, BUFFERSIZE);
	}

	printf("Recieve File:\t %s From Server[%s] Finished!\n", file_name, argv[1]);

	fclose(fp);
	close(client_socket);
	return 0;

}