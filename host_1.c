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

#define HOST1_INADDR "127.0.0.0 8080"

#define HOST_NUMBER 1

#define PORT 8080

int sendSubscribeHostMessage(char * message){

	int sock;

	struct sockaddr_in server;

	char reply[50];

	sock = socket(AF_INET, SOCK_STREAM, 0);

	if(sock == -1){

		printf("\n\nElastic Container Service - host #%d: Could not create Socket Client.", HOST_NUMBER);

	}

	printf("\n\nElastic Container Service - host #%d: Socket Client created successfully.", HOST_NUMBER);

	server.sin_family = AF_INET;

	server.sin_addr.s_addr = INADDR_ANY;

	server.sin_port = htons(6060);

	if(connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0){

		//printf("\n\nElastic Container Service - host #%d: Connect failed. ", HOST_NUMBER);

		perror("Error");

		return 1;
	}

	//printf("\n\nElastic Container Service - host #%d: Socket Client connected successfully.", HOST_NUMBER);

	memset(reply, 0, 20);

    strcpy(reply, message);

    send(sock, reply, strlen(reply), 0);

	printf("\n\nElastic Container Service - Host #%d: Message sent to Subscribe Host.", HOST_NUMBER);

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

	fclose(filePointer);

	if(wordExist == 1){

		return 1;

	}

	else {

		return 0;

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

		if(strcmp(clientRequest, "create") == 0){

			containerExistence = checkExistence(containerName);

			if(containerExistence == 0){

				execlp("docker", "docker", "run", "-di", "--name", containerName, "ubuntu:latest", "/bin/bash", NULL);

			}

		}

		else if(strcmp(clientRequest, "stop") == 0){

			containerExistence = checkExistence(containerName);

			if(containerExistence == 1){

				execlp("docker", "docker", "stop", containerName, NULL);

			}

		}

		else if(strcmp(clientRequest, "remove") == 0){

			containerExistence = checkExistence(containerName);

			if(containerExistence == 1){

				execlp("docker", "docker", "rm", "--force", containerName, NULL);

			}

		}

		else if(strcmp(clientRequest, "list") == 0){

			execlp("docker", "docker", "ps", NULL);

		}

	}

	else{

		if(strcmp(clientRequest, "create") == 0){

			containerExistence = checkExistence(containerName);

			if(containerExistence == 0){

				FILE *fp = fopen(filename, "a");

				fprintf(fp, "host1 %s\n", containerName);

				fclose(fp);

				sendSubscribeHostMessage("\n\nElastic Container Service - Host #1: El contenedor ha sido creado.");

			}

			else{

				sendSubscribeHostMessage("\n\nElastic Container Service - Host #1: El contenedor ya existe.");

			}

		}

		else if(strcmp(clientRequest, "stop") == 0){

			//REVISAR SI CONTAINER CORRIENDO O DETENIDO (FUNCION)

			containerExistence = checkExistence(containerName);

			if(containerExistence == 1){

				sendSubscribeHostMessage("\n\nElastic Container Service - Host #1: El contenedor ha sido detenido");

			}

			else{

				sendSubscribeHostMessage("\n\nElastic Container Service - Host #1: El contenedor no existe.");

			}

		}

		else if(strcmp(clientRequest, "remove") == 0){

			containerExistence = checkExistence(containerName);

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

				sendSubscribeHostMessage("\n\nElastic Container Service - Host #1: Contenedor Borrado con Exito.");

			}

			else{

				sendSubscribeHostMessage("\n\nElastic Container Service - Host #1: El contenedor no existe.");

			}

		}

		else if(strcmp(clientRequest, "list") == 0){

			printf("\n\nLista de Contenedores\n\n");

		}

		wait(NULL);

	}

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

	sendSubscribeHostMessage(HOST1_INADDR);

	server.sin_family = AF_INET;

	server.sin_addr.s_addr = INADDR_ANY;

	server.sin_port = htons(8080);

	if(bind(socket_desc, (struct sockaddr *) &server, sizeof(server)) < 0){

		perror("Bind failed. Error");

		return 1;

	}

	printf("\n\nElastic Container Service - Host #%d: Socket Server bind done.", HOST_NUMBER);

	while(1){

		listen(socket_desc, 3);

		printf("\n\nElastic Container Service - Host #%d: Waiting for incoming connections...", HOST_NUMBER);

		c = sizeof(struct sockaddr_in);

		client_sock = accept(socket_desc, (struct sockaddr *) &client, (socklen_t*) &c);

		if(client_sock < 0){

			puts("\n\nElastic Container Service - Host #%d: Accept failed");

			return 1;

		}

		printf("\n\nElastic Container Service - Host #%d: Connection accepted", HOST_NUMBER);

		received = 0;
		
		while(!received){

			memset(client_message, 0, 2000);

			if(recv(client_sock, client_message, 2000, 0) > 0){

				printf("received message host: %s\n", client_message);

				char * client_message_split = strtok(client_message, " ");

				strcpy(clientRequest, client_message_split);

				if(strcmp(clientRequest, "list") != 0){

					client_message_split = strtok(NULL, " ");

					strcpy(containerName, client_message_split);

					printf("Request: %s, Container Name: %s\n\n", clientRequest, containerName);

				}

				dockerExecutions(client_sock, clientRequest, containerName);

				received = 1;

			}
			
			else{

				perror("Receive Failed");

				break;

			}
			
		}

	}

}

int main(int argc, char *argv[]) {

	ecs_agent();

	return 0;

}