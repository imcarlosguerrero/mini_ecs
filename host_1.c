#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<string.h>
#include<sys/wait.h>

#define HOST_NUMBER 1

#define PORT 8080

#define HOST_INADDR "127.0.0.0 8080"

#define SUBSCRIBE_HOST_PORT 6060

#define ADMIN_CONTAINER_PORT 7070

void sendSubscribeHostMessage(char * message){

	int sock;

	struct sockaddr_in server;

	char reply[200];

	sock = socket(AF_INET, SOCK_STREAM, 0);

	if(sock == -1){

		printf("\n\nElastic Container Service - Host #%d: Could not create Socket Client.\n", HOST_NUMBER);

	}

	printf("\nElastic Container Service - Host #%d: Socket Client created successfully.\n", HOST_NUMBER);

	server.sin_family = AF_INET;

	server.sin_addr.s_addr = INADDR_ANY;

	server.sin_port = htons(SUBSCRIBE_HOST_PORT);

	if(connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0){

		printf("\nElastic Container Service - Host #%d: Connect failed.\n", HOST_NUMBER);

	}

	printf("\nElastic Container Service - Host #%d: Socket Client connected successfully.\n", HOST_NUMBER);

	memset(reply, 0, 20);

    strcpy(reply, message);

    send(sock, reply, strlen(reply), 0);

	close(sock);

}

int dockerExecutions(int client_sock, char * clientRequest, char * containerName){

	char reply[200];

	int containerExistence = 0;

	char *filename = "containers.txt";

	pid_t pid = fork();

	if(pid == 0){

		if(strcmp(clientRequest, "create") == 0){

			execlp("docker", "docker", "run", "-di", "--name", containerName, "ubuntu:latest", "/bin/bash", NULL);

		}

		else if(strcmp(clientRequest, "stop") == 0){

			execlp("docker", "docker", "stop", containerName, NULL);

		}

		else if(strcmp(clientRequest, "remove") == 0){
		
			execlp("docker", "docker", "rm", "--force", containerName, NULL);

		}

		else if(strcmp(clientRequest, "start") == 0){

			execlp("docker", "docker", "start", containerName, NULL);

		}

	}

}

int ecs_agent(){

	int socket_desc, client_sock, c, read_size;

	struct sockaddr_in server, client;

	char client_message[2000], clientRequest[200], containerName[200];

	int received = 0;

	socket_desc = socket(AF_INET, SOCK_STREAM , 0);

	if(socket_desc == -1){

		printf("\nElastic Container Service - Host #%d: Could not create Socket Server.\n", HOST_NUMBER);

	}

	printf("\nElastic Container Service - Host #%d: Socket Server created successfully.\n", HOST_NUMBER);

	sendSubscribeHostMessage(HOST_INADDR);

	server.sin_family = AF_INET;

	server.sin_addr.s_addr = INADDR_ANY;

	server.sin_port = htons(PORT);

	if(bind(socket_desc, (struct sockaddr *) &server, sizeof(server)) < 0){

		printf("\nElastic Container Service - Host #%d: Bind failed.\n", HOST_NUMBER);

		return 1;

	}

	printf("\nElastic Container Service - Host #%d: Bind Done.\n", HOST_NUMBER);

	while(1){

		listen(socket_desc, 100);

		printf("\nElastic Container Service - Host #%d: Waiting for incoming connections...\n", HOST_NUMBER);

		c = sizeof(struct sockaddr_in);

		//ACCEPT CONNECTION AND SEND TO THREAD

		client_sock = accept(socket_desc, (struct sockaddr *) &client, (socklen_t*) &c);

		if(client_sock < 0){

			printf("\nElastic Container Service - Host #%d: Accept Failed.\n", HOST_NUMBER);

			return 1;

		}

		printf("\nElastic Container Service - Host #%d: Connection accepted.\n", HOST_NUMBER);

		received = 0;
		
		while(!received){

			memset(client_message, 0, 2000);

			if(recv(client_sock, client_message, 2000, 0) > 0){

				printf("\nElastic Container Service - Host #%d: received message host: %s\n", HOST_NUMBER, client_message);

				char * client_message_split = strtok(client_message, " ");

				strcpy(clientRequest, client_message_split);

				if(strcmp(clientRequest, "list") != 0){

					client_message_split = strtok(NULL, " ");

					strcpy(containerName, client_message_split);

					printf("\nElastic Container Service - Host #%d: Request: %s, Container Name: %s\n", HOST_NUMBER, clientRequest, containerName);

					dockerExecutions(client_sock, clientRequest, containerName);

				}

				received = 1;

			}
			
			else{

				printf("\nElastic Container Service - Host #%d: Receive Failed.\n", HOST_NUMBER);

				break;

			}
			
		}

	}

}

void main() {

	ecs_agent();

}