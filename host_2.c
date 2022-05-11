/*

host El host es la maquina donde se crearan los contenedores, para efectos de
prueba puede ser una maquina virtual o la maquina que tenga disponible el
estudiante.

ecs-agent Este agente es el encargado de crear, listar, detener o borrar contenedores. Este proceso solo recibe instrucciones del elastic-container-service
por medio del proceso admin_container. Como la creación de un contenedor
no todo el tiempo es inmediata, el agente recibe la petición y responde con un
status de recibido y ejecuta la acción.
Cuando un agente se inicia en un host, este debe reportarse enviando una petición al proceso subscribe_host del elastic-container-service.

*/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>	//strlen
#include<sys/socket.h>
#include<arpa/inet.h>	//inet_addr
#include<unistd.h>	//write
#include<string.h>
#include<sys/wait.h>

#define HOST_NUMBER 2

#define PORT 9090

#define HOST_INADDR "127.0.0.0 9090"

#define SUBSCRIBE_HOST_PORT 6060

#define ADMIN_CONTAINER_PORT 7070

void sendSubscribeHostMessage(char * message){

	int sock;

	struct sockaddr_in server;

	char reply[200];

	sock = socket(AF_INET, SOCK_STREAM, 0);

	if(sock == -1){

		printf("\n\nElastic Container Service - Host #%d: Could not create Socket Client.", HOST_NUMBER);

	}

	printf("\n\nElastic Container Service - Host #%d: Socket Client created successfully.", HOST_NUMBER);

	server.sin_family = AF_INET;

	server.sin_addr.s_addr = INADDR_ANY;

	server.sin_port = htons(SUBSCRIBE_HOST_PORT);

	if(connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0){

		printf("\n\nElastic Container Service - Host #%d: Connect failed.", HOST_NUMBER);

	}

	printf("\n\nElastic Container Service - Host #%d: Socket Client connected successfully.", HOST_NUMBER);

	memset(reply, 0, 20);

    strcpy(reply, message);

    send(sock, reply, strlen(reply), 0);

	close(sock);

}

int checkExistence(char * containerName){

	FILE* filePointer;

	int wordExist = 0;

	int bufferLength = 255;

	char line[bufferLength];

	int lineCounter = 0;

	filePointer = fopen("containers.txt", "r");

	while(fgets(line, bufferLength, filePointer)){

		lineCounter++;

		char *ptr = strstr(line, containerName);

		if(ptr != NULL){

			wordExist = 1;

			break;
		}
	}

	printf("WORD EXISTE VALUE %d", wordExist);

	fclose(filePointer);

	if(wordExist == 0){

		return 0;

	}

	else if(wordExist == 1) {

		return 1;

	}

}

int getContainerLine(char * containerName){

	FILE* filePointer;

	int wordExist = 0;

	int bufferLength = 255;

	char line[bufferLength];

	int lineCounter = 0;

	filePointer = fopen("containers.txt", "r");

	while(fgets(line, bufferLength, filePointer)){

		lineCounter++;

		char *ptr = strstr(line, containerName);

		if(ptr != NULL){

			wordExist = 1;

			break;

		}
	}

	fclose(filePointer);

	if(wordExist == 1){

		return lineCounter;

	}

	else {

		return lineCounter;

	}

}

int dockerExecutions(int client_sock, char * clientRequest, char * containerName){

	char reply[200];

	int containerExistence = 0;

	char *filename = "containers.txt";

	pid_t pid = fork();

	if(pid == 0){

		containerExistence = checkExistence(containerName);

		if(strcmp(clientRequest, "create") == 0){

			printf("CHECK EXISTENCE VALUE %d", containerExistence);

			if(containerExistence == 0){

				execlp("docker", "docker", "run", "-di", "--name", containerName, "ubuntu:latest", "/bin/bash", NULL);

			}

		}

		else if(strcmp(clientRequest, "stop") == 0){

			if(containerExistence == 1){

				execlp("docker", "docker", "stop", containerName, NULL);

			}

		}

		else if(strcmp(clientRequest, "remove") == 0){

			if(containerExistence == 1){

				execlp("docker", "docker", "rm", "--force", containerName, NULL);

			}

		}

		else if(strcmp(clientRequest, "list") == 0){

			execlp("docker", "docker", "ps", NULL);

		}

	}

	else{

		containerExistence = checkExistence(containerName);

		if(strcmp(clientRequest, "create") == 0){

			if(containerExistence == 0){

				FILE *fp = fopen(filename, "a");

				fprintf(fp, "host2 %s\n", containerName);

				wait(10);

				fclose(fp);

				sendSubscribeHostMessage("\n\nElastic Container Service - Host #2: El contenedor ha sido creado.");

			}

			else{

				sendSubscribeHostMessage("\n\nElastic Container Service - Host #2: El contenedor ya existe.");

			}

		}

		else if(strcmp(clientRequest, "stop") == 0){

			//REVISAR SI CONTAINER CORRIENDO O DETENIDO (FUNCION)

			if(containerExistence == 1){

				sendSubscribeHostMessage("\n\nElastic Container Service - Host #2: El contenedor ha sido detenido");

			}

			else{

				sendSubscribeHostMessage("\n\nElastic Container Service - Host #2: El contenedor no existe.");

			}

		}

		else if(strcmp(clientRequest, "remove") == 0){

			if(containerExistence == 1){

				int line, lineCounter = 0;

				FILE *containerListPointer, *temporalFilePointer;

				char temporalString[256];

				char containerList[] = "containers.txt", temporalFile[] = "temporal.txt";

				containerListPointer = fopen(containerList, "r");

				temporalFilePointer = fopen(temporalFile, "w");
				
				line = getContainerLine(containerName);
				
				printf("%d", line);

				while(!feof(containerListPointer)){

					strcpy(temporalString, "\0");

					fgets(temporalString, 256, containerListPointer);

					if(!feof(containerListPointer)){

						lineCounter++;
					
						if(lineCounter != line){

							fprintf(temporalFilePointer, "%s", temporalString);

						}

					}

				}

				fclose(containerListPointer);

				fclose(temporalFilePointer);

				remove(containerList);

				rename(temporalFile, containerList);

				wait(10);

				sendSubscribeHostMessage("\n\nElastic Container Service - Host #2: Contenedor Borrado con Exito.");

			}

			else{

				sendSubscribeHostMessage("\n\nElastic Container Service - Host #2: El contenedor no existe.");

			}

		}

		else if(strcmp(clientRequest, "list") == 0){

			printf("\n\nLista de Contenedores\n\n");

		}

	}

	wait(NULL);

}

int ecs_agent(){

	int socket_desc, client_sock, c, read_size;

	struct sockaddr_in server, client;

	char client_message[2000], clientRequest[200], containerName[200];

	int received = 0;

	socket_desc = socket(AF_INET, SOCK_STREAM , 0);

	if(socket_desc == -1){

		printf("\n\nElastic Container Service - Host #%d: Could not create Socket Server.", HOST_NUMBER);

	}

	printf("\n\nElastic Container Service - Host #%d: Socket Server created successfully.", HOST_NUMBER);

	sendSubscribeHostMessage(HOST_INADDR);

	server.sin_family = AF_INET;

	server.sin_addr.s_addr = INADDR_ANY;

	server.sin_port = htons(PORT);

	if(bind(socket_desc, (struct sockaddr *) &server, sizeof(server)) < 0){

		printf("\n\nElastic Container Service - Host #%d: Bind failed. Error", HOST_NUMBER);

		return 1;

	}

	printf("\n\nElastic Container Service - Host #%d: Bind done", HOST_NUMBER);

	while(1){

		listen(socket_desc, 3);

		printf("\n\nElastic Container Service - Host #%d: Waiting for incoming connections...", HOST_NUMBER);

		c = sizeof(struct sockaddr_in);

		//ACCEPT CONNECTION AND SEND TO THREAD

		client_sock = accept(socket_desc, (struct sockaddr *) &client, (socklen_t*) &c);

		if(client_sock < 0){

			printf("\n\nElastic Container Service - Host #%d: Accept failed", HOST_NUMBER);

			return 1;

		}

		printf("\n\nElastic Container Service - Host #%d: Connection accepted", HOST_NUMBER);

		received = 0;
		
		while(!received){

			memset(client_message, 0, 2000);

			if(recv(client_sock, client_message, 2000, 0) > 0){

				printf("\n\nElastic Container Service - Host #%d: received message host: %s\n", HOST_NUMBER, client_message);

				char * client_message_split = strtok(client_message, " ");

				strcpy(clientRequest, client_message_split);

				if(strcmp(clientRequest, "list") != 0){

					client_message_split = strtok(NULL, " ");

					strcpy(containerName, client_message_split);

					printf("\n\nElastic Container Service - Host #%d: Request: %s, Container Name: %s\n\n", HOST_NUMBER, clientRequest, containerName);

					dockerExecutions(client_sock, clientRequest, containerName);

				}

				else{
					
					dockerExecutions(client_sock, "list", containerName);

				}

				received = 1;

			}
			
			else{

				printf("\n\nElastic Container Service - Host #%d: Receive Failed", HOST_NUMBER);

				break;

			}
			
		}

	}

}

int main(int argc, char *argv[]) {

	ecs_agent();

	return 0;

}