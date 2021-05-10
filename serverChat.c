#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>

#define BUFF_MAX 80
#define PORT 8080
#define IP INADDR_ANY
#define MAX_CLIENTS_COUNT 2


struct client
{
   int index;
   int socketFd;
   char name[BUFF_MAX];
   struct sockaddr_in clientSocket;
};


int clientsCount = 0;
struct client clients[MAX_CLIENTS_COUNT];
pthread_t thread[MAX_CLIENTS_COUNT];


void* startChat(void*);
void sendMessageToAll(int, char*);


int main()
{
    int socketFd;
    int connectionFd;
    struct sockaddr_in serverSocket;
    struct sockaddr_in clientSocket;

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
    serverSocket.sin_addr.s_addr = htonl(IP);
    serverSocket.sin_port = htons(PORT);

    if ((bind(socketFd, (struct sockaddr *)&serverSocket, sizeof(serverSocket))) != 0)
    {
        perror("socket bind failed...\n");
        exit(1);
    }
    else
    {
        printf("Socket successfully binded..\n");
    }

    if ((listen(socketFd, MAX_CLIENTS_COUNT)) != 0)
    {
        perror("Listen failed...\n");
        exit(1);
    }
    else
    {
        printf("Server listening..\n");
    }
    
    int i;
    for (i = 0; i < MAX_CLIENTS_COUNT; i++)
    {
        int clientSocketLength = sizeof(clientSocket);
        connectionFd = accept(socketFd, (struct sockaddr *)&clientSocket, &clientSocketLength);
        if (connectionFd < 0)
        {
            perror("server acccept failed...\n");
            exit(1);
        }
        else
        {
            printf("server acccepted the %dth client...\n", i + 1);
    	}
	
	clients[i].index = i;
	clients[i].socketFd = connectionFd;

	pthread_create(&thread[i], NULL, startChat, (void *) &clients[i]);

	clientsCount++;
    }

    for (i = 0; i < MAX_CLIENTS_COUNT; i++)
    {
	pthread_join(thread[i], NULL);
    }

    printf("server has closed\n");
}

void* startChat(void* clientAsVoid)
{
    struct client* curClient = (struct client*) clientAsVoid;
    char buffer[BUFF_MAX];
    int socketFd = curClient->socketFd;
    char name[BUFF_MAX];
    int index = curClient->index;

    read(socketFd, name, BUFF_MAX);
    //replace \n with \0
    name[strlen(name) - 1] = '\0';

    bzero(buffer, BUFF_MAX);
    strcat(buffer, name);
    strcat(buffer, " has joined the chat\n");
    sendMessageToAll(index, buffer);

    for (;;)
    {
        bzero(buffer, BUFF_MAX);
        int readCode = read(socketFd, buffer, sizeof(buffer));

	if (strncmp("exit", buffer, 4) == 0 || readCode == 0)
        {
	    bzero(buffer, BUFF_MAX);
	    strcat(buffer, name);
	    strcat(buffer, " has left the chat\n");

    	    sendMessageToAll(index, buffer);
	    break;
        }

	char message[BUFF_MAX];
	bzero(message, BUFF_MAX);
	strcat(message, name);
	strcat(message, " - ");
	strcat(message, buffer);
	sendMessageToAll(index, message);
    }

    close(socketFd);
    return NULL;
}

void sendMessageToAll(int exceptSocketIndex, char* buffer)
{
	int i;
	for (i = 0; i < clientsCount; i++)
	{
	    if (i != exceptSocketIndex)
	    {
                write(clients[i].socketFd, buffer, BUFF_MAX);
	    }
	}
}
