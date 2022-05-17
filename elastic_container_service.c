#include <stdio.h>
#include <stdlib.h>
#include <string.h>	//strlen
#include <sys/socket.h>
#include <arpa/inet.h>	//inet_addr
#include <unistd.h>
#include <time.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>

#define HOST_NUMBER 200

#define SHM_OBJ_NAME "/myMemoryObj"

#define SHM_OBJ_SIZE 200

#define MESSAGE_SIZE 2048

#define DEFAULT_SIZE 1024

#define ADMIN_CONTAINER_PORT 7070

#define SUBSCRIBE_HOST_PORT 6060

#define MAX_REQUEST_NUMBER 100

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

int getLineHost(char * hostName){

	FILE* filePointer;

	int wordExist = 0;

	int bufferLength = 255;

	char line[bufferLength];

	int lineCounter = 0;

	filePointer = fopen("hosts.txt", "r");

	while(fgets(line, bufferLength, filePointer)){

		lineCounter++;

		char *ptr = strstr(line, hostName);

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

int getLineHostSpecial(char * hostName){

	FILE* filePointer;

	int wordExist = 0;

	int bufferLength = 255;

	char line[bufferLength];

	int lineCounter = 0;

	filePointer = fopen("hosts.txt", "r");

	while(fgets(line, bufferLength, filePointer)){

		char *ptr = strstr(line, hostName);

		if(ptr != NULL){

			wordExist = 1;

			break;

		}

		lineCounter++;
	}

	fclose(filePointer);

	if(wordExist == 1){

		return lineCounter;

	}

	else {

		return lineCounter;

	}

}

int getContainerHostPort(char * containerName){

	int lineNumber = getContainerLine(containerName), lineCounter = 0;

	static const char filename[] = "containers.txt";

	FILE *file = fopen(filename, "r");

	char line[DEFAULT_SIZE];

    int count = 0;

	while(fgets(line, sizeof line, file) != NULL){   

		if(count == lineNumber){   

			printf("\n str %s ", line);

			break;

		}   

		else{   

			count++;

		}   

	}  

	fclose(file);

	char * token = strtok(line, " ");

	char hostName[DEFAULT_SIZE];

	strcpy(hostName, token);

	int lineHostNumber = getLineHostSpecial(hostName);

	static const char hFilename[] = "hosts.txt";

	FILE *hFile = fopen(hFilename, "r");

	char lineHost[DEFAULT_SIZE];

    count = 0;

	while(fgets(lineHost, sizeof lineHost, file) != NULL){   

		if(count == lineHostNumber){   

			printf("\n str %s ", lineHost);

			break;

		}   

		else{   

			count++;

		}   

	}  

	fclose(file);

	char * hToken = strtok(lineHost, " ");

	hToken = strtok(NULL, " ");

	hToken = strtok(NULL, " ");

	char hostPort[DEFAULT_SIZE];

	strcpy(hostPort, hToken);

	int realHostPort = atoi(hostPort);

	printf("%d\n", realHostPort);

	return realHostPort;

}

void sendHostRequest(char * client_message, int hostPort){

    //Creación del Socket Client del adminContainer para comunicarse con los diferentes host. 

	int sock;

	struct sockaddr_in server;

	char message[MESSAGE_SIZE], server_reply[MESSAGE_SIZE];

	sock = socket(AF_INET, SOCK_STREAM, 0);

	if(sock == -1){

		printf("\nElastic Container Service - Admin Container: Could not create Socket Server.\n");

        exit(1);

	}

	printf("\nElastic Container Service - Admin Container: Socket Client created successfully.\n");

	server.sin_family = AF_INET;

	server.sin_addr.s_addr = INADDR_ANY;

	server.sin_port = htons(hostPort);

	if(connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0){

		printf("\nElastic Container Service - Admin Container: Connect Failed.\n");

		exit(1);
	}

	printf("\nElastic Container Service - Subscribe Host: Connected.\n");

	memset(message, 0, MESSAGE_SIZE);

    strcpy(message, client_message);

    send(sock, message, strlen(message), 0);

	close(sock);

}

void listContainers(){

	char hostsArray[HOST_NUMBER][HOST_NUMBER];

    FILE *fptr = NULL; 

    int i = 0;

    int tot = 0;

	char fname[DEFAULT_SIZE] = "containers.txt";

    fptr = fopen(fname, "r");

    while(fgets(hostsArray[i], MESSAGE_SIZE, fptr)){

        hostsArray[i][strlen(hostsArray[i]) - 1] = '\0';

        i++;

    }

    tot = i;

	printf("\nElastic Container Service - Admin Container: Los Contenedores Activos Son:\n\n");

	for(int i = 0; i < tot; i++){

		printf(" %s\n", hostsArray[i]);

		printf("\n");

	}

    printf("\n\n");

}



void subscribe_host(){

	int hostNumber = 0;

	int subscribe_host, client_sock, c, read_size;

	struct sockaddr_in server, client;

	char client_message[MESSAGE_SIZE];

	subscribe_host = socket(AF_INET, SOCK_STREAM, 0);

	if(subscribe_host == -1){

		printf("\nElastic Container Service - Subscribe Host: Could not create Socket Server.\n");

        exit(1);

	}

	printf("\nElastic Container Service - Subscribe Host: Socket Server created successfully.\n");

	char *filename = "hosts.txt";

	server.sin_family = AF_INET;

	server.sin_addr.s_addr = INADDR_ANY;

	server.sin_port = htons(SUBSCRIBE_HOST_PORT);

	if(bind(subscribe_host, (struct sockaddr *) &server, sizeof(server)) < 0){

		printf("\nElastic Container Service - Subscribe Host: Socket Server Bind Failed.\n");

	}

	printf("\nElastic Container Service - Subscribe Host: Socket Server Bind Done.\n");

	while(1){

		listen(subscribe_host, 10);

		printf("\nElastic Container Service - Subscribe Host: Waiting for incoming connections...\n");

		c = sizeof(struct sockaddr_in);

		client_sock = accept(subscribe_host, (struct sockaddr *) &client, (socklen_t*) &c);

		if(client_sock < 0){

			printf("\nElastic Container Service - Subscribe Host: Accept failed.\n");

			exit(1);

		}

		printf("\n\nElastic Container Service - Subscribe Host: Connection accepted from a host succesfully.");

		memset(client_message, 0, 2000);

		if(recv(client_sock, client_message, 2000, 0) > 0){

			char to_find[] = "Elastic";

			if(strstr(client_message, to_find)){

				printf("%s", client_message);

			}

			else{

				FILE *fp = fopen(filename, "a");

				if(fp == NULL){
		
					printf("\nElastic Container Service - Subscribe Host: Error opening the file %s", filename);

					exit(1);
				}

				hostNumber++;

				

				fprintf(fp, "host%d %s\n", hostNumber, client_message);

				fclose(fp);

				//RANDOM HOST AND SEND TO SHARE MEMORY

				int fd;

				char buff[1024];

				char *ptr;

				fd = shm_open(SHM_OBJ_NAME, O_RDWR, 0);

				if(fd == -1){
 
					printf("\nShare Memory Write: Failed Opening.\n");

					exit(1);

				}

				char hostsArray[HOST_NUMBER][HOST_NUMBER];

				FILE *fptr = NULL; 

				int i = 0;

				int tot = 0;

				char fname[20] = "hosts.txt";

				fptr = fopen(fname, "r");

				while(fgets(hostsArray[i], 2000, fptr)){

					hostsArray[i][strlen(hostsArray[i]) - 1] = '\0';

					i++;

				}

				tot = i;

				char randomHost[200];

				srand(time(NULL));

				int randomHostNumber = rand() % hostNumber;

				strcpy(randomHost, hostsArray[randomHostNumber]);

				strcpy(buff, randomHost);

				ptr = mmap(0, sizeof(buff), PROT_WRITE, MAP_SHARED, fd, 0);

				if(ptr == MAP_FAILED){

					printf("Share Memory Write: Map Failed.\n");

					exit(1);

				}

				memcpy(ptr, buff, sizeof(buff));

				close(fd);

				//RANDOM HOST AND SEND TO SHARE MEMORY

				send(client_sock, client_message, strlen(client_message), 0);

			}

		}
		
		else{

			printf("\nElastic Container Service - Subscribe Host: Received Failed.\n");

		}
	
	}

}

int admin_container(){

    //Levantar el Socket Server de adminContainer

	int admin_container, ecs_client_sock, c, read_size, hostPort = 0, receivedMessage = 0;;

	struct sockaddr_in server, client;

	char ecs_client_message[MESSAGE_SIZE], selectedHost[DEFAULT_SIZE],hostName[DEFAULT_SIZE], clientRequest[DEFAULT_SIZE], containerName[DEFAULT_SIZE];

	admin_container = socket(AF_INET, SOCK_STREAM , 0);

	if(admin_container == -1){

		printf("\nElastic Container Service - Admin Container: Could not create Socket Server.\n");

	}

	printf("\nElastic Container Service - Admin Container: Socket Server created successfully.\n");

	server.sin_family = AF_INET;

	server.sin_addr.s_addr = INADDR_ANY;

	server.sin_port = htons(ADMIN_CONTAINER_PORT);

	if(bind(admin_container, (struct sockaddr *) &server, sizeof(server)) < 0){

		printf("\nElastic Container Service - Admin Container: Socket Server Bind Failed.\n");

		exit(1);

	}

	printf("\nElastic Container Service - Admin Container: Socket Server Bind Done.\n");

    //Levantar el Socket Server de adminContainer

    while(1){

        listen(admin_container, MAX_REQUEST_NUMBER);

        printf("\nElastic Container Service - Admin Container: Waiting for Incoming Connections...\n");

		c = sizeof(struct sockaddr_in);

		ecs_client_sock = accept(admin_container, (struct sockaddr *) &client, (socklen_t*) &c);

		if(ecs_client_sock < 0){

			printf("\nElastic Container Service - Admin Container: Accept Failed\n");

			exit(1);

		}

		printf("\nElastic Container Service - Admin Container: Connection Accepted.\n");

		receivedMessage = 0;

        while(!receivedMessage){

            sleep(2);

            memset(ecs_client_message, 0, MESSAGE_SIZE);

            if(recv(ecs_client_sock, ecs_client_message, MESSAGE_SIZE, 0) > 0){

                receivedMessage = 1;

                printf("\nElastic Container Service - Admin Container: Received Request: %s\n", ecs_client_message);

				char ecs_client_message_no_split[DEFAULT_SIZE];

				strcpy(ecs_client_message_no_split, ecs_client_message);

				char * client_message_split = strtok(ecs_client_message, " ");

				strcpy(clientRequest, client_message_split);

				client_message_split = strtok(NULL, " ");

				strcpy(containerName, client_message_split);

				int containerExistence = checkExistence(containerName);

                if(strcmp(clientRequest, "list") == 0){

                    listContainers();

                }

                else if(strcmp(clientRequest, "create") == 0){

					//Share Memory Read

					int fd;

					char *ptr;

					struct stat shmobj_st;

					fd = shm_open(SHM_OBJ_NAME, O_RDONLY, 0);

					if(fd == -1){

						printf("\nShare Memory Read: Error File Descriptor.\n");

						exit(1);

					}

					if(fstat(fd, &shmobj_st) == -1){

						printf("\nShare Memory Read: Error on Fstat.\n");

						exit(1);

					}

					ptr = mmap(NULL, shmobj_st.st_size, PROT_READ, MAP_SHARED, fd, 0);

					if(ptr == MAP_FAILED){

						printf("\nShare Memory Read: Map Failed.\n");

						exit(1);

					}

					//Share Memory Read

					//Uso del host tomado de la Share Memory

					strcpy(selectedHost, ptr);

					char * token = strtok(selectedHost, " ");

					int i = 0;

					while(token != NULL){

						if(i == 0){

							strcpy(hostName, token);

						}

						if(i == 2){

							//Se saca el puerto del host seleccionado

							hostPort = atoi(token);

							break;	

						}

						token = strtok(NULL, " ");

						i++;

					}

					//Ejecuciones de las peticiones del ecs_client

                    if(containerExistence == 0){

                        sendHostRequest(ecs_client_message_no_split, hostPort);

                        char *filename = "containers.txt";

                        FILE *fp = fopen(filename, "a");

                        fprintf(fp, "%s %s Running\n", hostName, containerName);

                        fclose(fp);

                        printf("\nElastic Container Service - Admin Container: El contenedor ha sido creado.\n");

                    }

                    else{

                        printf("\nElastic Container Service - Admin Container: El contenedor ya existe.\n");

                    }

                }

                else if(strcmp(clientRequest, "remove") == 0){

                    if(containerExistence == 1){

						int hostContainerPort = getContainerHostPort(containerName);

                        sendHostRequest(ecs_client_message_no_split, hostContainerPort);

                        int line = getContainerLine(containerName), lineCounter = 0;

                        FILE *containerListPointer, *temporalFilePointer;

                        char temporalString[256];

                        char containerList[] = "containers.txt", temporalFile[] = "temporal.txt";

                        containerListPointer = fopen(containerList, "r");

                        temporalFilePointer = fopen(temporalFile, "w");

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

                        printf("\nElastic Container Service - Admin Container: Contenedor Borrado con Exito.\n");

                    }

                    else{

                        printf("\nElastic Container Service - Admin Container: El contenedor no existe.\n");

                    }

                }

                else if(strcmp(clientRequest, "start") == 0){

                    if(containerExistence == 1){

						int hostContainerPort = getContainerHostPort(containerName);

                        int line = getContainerLine(containerName), lineCounter = 0;

                        FILE *containerListPointer, *temporalFilePointer;

                        char temporalString[256];

                        char containerList[] = "containers.txt", temporalFile[] = "temporal.txt";

                        containerListPointer = fopen(containerList, "r");

                        temporalFilePointer = fopen(temporalFile, "w");

                        while(!feof(containerListPointer)){

                            strcpy(temporalString, "\0");

                            fgets(temporalString, 256, containerListPointer);

                            lineCounter++;
                        
                            if(lineCounter != line){

                                fprintf(temporalFilePointer, "%s", temporalString);

                            }

                            else{

                                char containerHost[DEFAULT_SIZE], containerNameID[DEFAULT_SIZE], containerStatus[DEFAULT_SIZE];

                                char * token = strtok(temporalString, " ");

                                strcpy(containerHost, token);


                                //Ya se cual es el container Host, ahora lo busco en hosts.txt y obtengo su info

                                int lineHost = getLineHost(containerHost);

                                token = strtok(NULL, " ");

                                strcpy(containerNameID, token);

                                token = strtok(NULL, " ");

                                strcpy(containerStatus, token);

                                printf("%s", containerStatus);

                                if(strcmp(containerStatus, "Running") == 0){

                                    printf("\nElastic Container Service - Admin Container: El contenedor ya está corriendo.\n");

                                }

                                else{

                                    sendHostRequest(ecs_client_message_no_split, hostContainerPort);

                                    strcat(containerHost, " ");

                                    strcat(containerHost, containerNameID);

                                    strcat(containerHost, " Running\n");

                                    fprintf(temporalFilePointer, "%s", containerHost);

                                    printf("\nElastic Container Service - Admin Container: El contenedor ha sido activado.\n");

                                }

                            }

                        }

                        fclose(containerListPointer);

                        fclose(temporalFilePointer);

                        remove(containerList);

                        rename(temporalFile, containerList);

                    }

                }

                else if(strcmp(clientRequest, "stop") == 0){

                    if(containerExistence == 1){

						int hostContainerPort = getContainerHostPort(containerName);

                        int line = getContainerLine(containerName), lineCounter = 0;

                        FILE *containerListPointer, *temporalFilePointer;

                        char temporalString[256];

                        char containerList[] = "containers.txt", temporalFile[] = "temporal.txt";

                        containerListPointer = fopen(containerList, "r");

                        temporalFilePointer = fopen(temporalFile, "w");

                        while(!feof(containerListPointer)){

                            strcpy(temporalString, "\0");

                            fgets(temporalString, 256, containerListPointer);

                            lineCounter++;
                        
                            if(lineCounter != line){

                                fprintf(temporalFilePointer, "%s", temporalString);

                            }

                            else{

                                char containerHost[DEFAULT_SIZE], containerNameID[DEFAULT_SIZE], containerStatus[DEFAULT_SIZE];

                                char * token = strtok(temporalString, " ");

                                strcpy(containerHost, token);

                                token = strtok(NULL, " ");

                                strcpy(containerNameID, token);

                                token = strtok(NULL, " ");

                                strcpy(containerStatus, token);

                                printf("%s", containerStatus);

                                if(strcmp(containerStatus, "Stopped") == 0){

                                    printf("\nElastic Container Service - Admin Container: El contenedor ya está detenido.\n");

                                }

                                else{

                                    sendHostRequest(ecs_client_message_no_split, hostContainerPort);

                                    strcat(containerHost, " ");

                                    strcat(containerHost, containerNameID);

                                    strcat(containerHost, " Stopped\n");

                                    fprintf(temporalFilePointer, "%s", containerHost);

                                    printf("\nElastic Container Service - Admin Container: El contenedor ha sido detenido.\n");

                                }

                            }

                        }

                        fclose(containerListPointer);

                        fclose(temporalFilePointer);

                        remove(containerList);

                        rename(temporalFile, containerList);

                    }

                }

            }

        }


    }


}

int main(){

	//Share Memory Creation

	int fd;

	fd = shm_open(SHM_OBJ_NAME, O_CREAT | O_RDWR, 00600);

	if(fd == -1){

        printf("\nShare Memory Creation: ");

		perror("");

        printf("\n");

		exit(1);

	}

	if(ftruncate(fd, SHM_OBJ_SIZE) == -1){

        printf("\nShare Memory Creation: ");

		perror("");

        printf("\n");

		exit(1);

	}

	//Share Memory Creation

    //Fork for adminContainer and subscribeHost

	pid_t pid;

	pid = fork();

	if(pid == 0){

		subscribe_host();

	}

	else{

		admin_container();

	}

}