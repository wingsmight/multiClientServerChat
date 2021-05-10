#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <pthread.h>

#define BUFF_MAX 80
#define SERVER_PORT 8080
#define SERVER_IP "127.0.0.1"


int socketFd;
char* name;
pthread_t readThread;

void startChat();
void* printChat(void*);


int main()
{
    struct sockaddr_in serverSocket;

    if ((socketFd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket creation failed...\n");
        exit(1);
    }
    else
    {
        printf("Socket successfully created..\n");
    }
    bzero(&serverSocket, sizeof(serverSocket));

    serverSocket.sin_family = AF_INET;
    serverSocket.sin_addr.s_addr = inet_addr(SERVER_IP);
    serverSocket.sin_port = htons(SERVER_PORT);

    if ((connect(socketFd, (struct sockaddr *)&serverSocket, sizeof(serverSocket))) != 0)
    {
        perror("connection with the server failed...\n");
        exit(1);
    }
    else
    {
        printf("connected to the server..\n");
    }

    printf("Enter your name:\n");
    scanf("%s", name);

    pthread_create(&readThread, NULL, printChat, NULL);

    startChat();

    close(socketFd);
    exit(0);
}

void startChat()
{
    char buffer[BUFF_MAX];

    write(socketFd, name, sizeof(buffer));

    for (;;)
    {
        bzero(buffer, sizeof(buffer));
	fgets(buffer, BUFF_MAX, stdin);
	write(socketFd, buffer, sizeof(buffer));

	if (strcmp(buffer, "exit\n") == 0)
	{
	    break;
	} 
   }
}

void* printChat(void* args)
{
	char buffer[BUFF_MAX];

	for (;;)
	{
		bzero(buffer, sizeof(buffer));
        	read(socketFd, buffer, BUFF_MAX);

        	printf("%s", buffer);
	}

	return NULL;
}
