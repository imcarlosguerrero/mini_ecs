/*

elastic-container-service Este servicio es el encargado de recibir las peticiones del cliente y ejecutarlas. Este servicio constara de dos procesos, el admin_container y el subscribe_host, los cuales se crearan usando fork.
El admin_container es el proceso encargado de recibir las peticiones para manejar los contenedores (crear, listar, detener y borrar contenedores). El subscribe_host, es un proceso encargado de recibir peticiones desde los hosts y lleva
un control de los hosts disponibles para crear contenedores. Solo los agentes que
corren en el host interactuan con el proceso subscribe_host.

*/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>	//strlen
#include<sys/socket.h>
#include<arpa/inet.h>	//inet_addr
#include<unistd.h>	//write
#include <time.h>
#include <sys/mman.h>

#define HOST_NUMBER 200

void* create_shared_memory(size_t size) {
  // Our memory buffer will be readable and writable:
  int protection = PROT_READ | PROT_WRITE;

  // The buffer will be shared (meaning other processes can access it), but
  // anonymous (meaning third-party processes cannot obtain an address for it),
  // so only this process and its children will be able to use it:
  int visibility = MAP_SHARED | MAP_ANONYMOUS;

  // The remaining parameters to `mmap()` are not important for this use case,
  // but the manpage for `mmap` explains their purpose.
  return mmap(NULL, size, protection, visibility, -1, 0);
}

int readHosts(){

	char hostsArray[HOST_NUMBER][HOST_NUMBER];

    FILE *fptr = NULL; 

    int i = 0;

    int tot = 0;

	char fname[20] = "hosts.txt";

    fptr = fopen(fname, "r");
	
    while(fgets(hostsArray[i], 200, fptr)){

        hostsArray[i][strlen(hostsArray[i]) - 1] = '\0';

        i++;

    }

    tot = i;

	printf("\n\nElastic Container Service - Subscribe Host: Los Hosts Disponibles son: \n\n");   

    for(int i = 0; i < tot; ++i){

        printf(" %s\n", hostsArray[i]);

    }

    printf("\n");

    return 0;

}

int sendHostMessage(char * client_message, int port){

	int sock;

	struct sockaddr_in server;

	char message[200], server_reply[50];

	sock = socket(AF_INET, SOCK_STREAM, 0);

	if(sock == -1){

		printf("\n\nElastic Container Service - Subscribe Host: Could not create Socket Server.");

	}

	printf("\n\nElastic Container Service - Subscribe Host: Socket Client created successfully.");

	server.sin_family = AF_INET;

	server.sin_addr.s_addr = INADDR_ANY;

	server.sin_port = htons(port);

	if(connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0){

		perror("\n\nElastic Container Service - Subscribe Host: Connect failed. Error");

		return 1;
	}

	printf("\n\nElastic Container Service - Subscribe Host: Connected.");

	memset(message, 0, 200);

    strcpy(message, client_message);

    send(sock, message, strlen(message), 0);

	close(sock);

}

int checkExistence(char * containerName){

	FILE* filePointer;

	int wordExist = 0;

	int bufferLength = 255;

	char line[bufferLength];

	int lineCounter = 0;

	filePointer = fopen("hosts.txt", "r");

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

int subscribe_host(){

	readHosts();

	int hostNumber = 0;

	int subscribe_host, client_sock, c, read_size;

	struct sockaddr_in server, client;

	char client_message[2000];

	int received = 0;

	subscribe_host = socket(AF_INET, SOCK_STREAM, 0);

	if(subscribe_host == -1){

		printf("\n\nElastic Container Service - Subscribe Host: Could not create Socket Server.");

	}

	printf("\n\nElastic Container Service - Subscribe Host: Socket Server created successfully.");

	char *filename = "hosts.txt";

	server.sin_family = AF_INET;

	server.sin_addr.s_addr = INADDR_ANY;

	server.sin_port = htons(6060);

	if(bind(subscribe_host, (struct sockaddr *) &server, sizeof(server)) < 0){

		perror("\n\nElastic Container Service - Subscribe Host: Socket Server bind failed. Error");

	}

	printf("\n\nElastic Container Service - Subscribe Host: Socket Server bind done.");

	while(1){

		listen(subscribe_host, 10);

		printf("\n\nElastic Container Service - Subscribe Host: Waiting for incoming connections...");

		c = sizeof(struct sockaddr_in);

		client_sock = accept(subscribe_host, (struct sockaddr *) &client, (socklen_t*) &c);

		if(client_sock < 0){

			perror("\n\nElastic Container Service - Subscribe Host: Accept failed.");

			return 1;

		}

		printf("\n\nElastic Container Service - Subscribe Host: Connection accepted from a host succesfully.");

		memset(client_message, 0, 2000);

		if(recv(client_sock, client_message, 2000, 0) > 0){

			char to_find[] = "Elastic";

			if(strstr(client_message, to_find)){

				printf("%s", client_message);

			}

			else{

				printf("\n\n%s", client_message);

				FILE *fp = fopen(filename, "a");

				if(fp == NULL){
		
					printf("\n\nElastic Container Service - Subscribe Host: Error opening the file %s", filename);

					return -1;
				}

				hostNumber++;

				fprintf(fp, "host%d %s\n", hostNumber, client_message);

				fclose(fp);

				received = 1;

				send(client_sock, client_message, strlen(client_message), 0);


			}

		}
		
		else{

			printf("\n\nElastic Container Service - Subscribe Host: Received failed.");

		}
	
	}

}

int admin_container(){

	int admin_container, client_sock, c, read_size, host_port = 9090;

	struct sockaddr_in server , client;

	char client_message[2000];

	int received = 0;

	admin_container = socket(AF_INET, SOCK_STREAM , 0);

	if(admin_container == -1){

		printf("\n\nElastic Container Service - Admin Container: Could not create Socket Server.\n\n");

	}

	printf("\n\nElastic Container Service - Admin Container: Socket Server created successfully.");

	server.sin_family = AF_INET;

	server.sin_addr.s_addr = INADDR_ANY;

	server.sin_port = htons(7070);

	if(bind(admin_container, (struct sockaddr *) &server, sizeof(server)) < 0){

		perror("\n\nElastic Container Service - Admin Container: Socket Server bind failed. Error");

		return 1;

	}

	printf("\n\nElastic Container Service - Admin Container: Socket Server bind done");
	
    while(1){

		listen(admin_container, 10);

		printf("\n\nElastic Container Service - Admin Container: Waiting for incoming connections...");

		c = sizeof(struct sockaddr_in);

		client_sock = accept(admin_container, (struct sockaddr *) &client, (socklen_t*) &c);

		if(client_sock < 0){

			perror("\n\nElastic Container Service - Admin Container: Accept failed");

			return 1;

		}

		printf("\n\nElastic Container Service - Admin Container: Connection accepted.");

		received = 0;

		while(!received){

			memset(client_message, 0, 2000);

			if(recv(client_sock, client_message, 2000, 0) > 0){

				printf("\n\nElastic Container Service - Admin Container: Received message: %s\n", client_message);

				received = 1;

				send(client_sock, client_message, strlen(client_message), 0);

				received = 1;

				//AQUI NECESITO SHARE_MEMORY DESDE SUBSCRIBE_HOST PARA PODER RECIBIR EL PUERTO QUE DEBERIA UTILIZAR A PARTIR DE LA LISTA QUE YA TENGO

				//LEIDA, DEBERIA SELECCIONAR UNA DE LAS LINEAS AL AZAR Y ENVIAR LA ULTIMA PARTE DE ELLA (EL HOST) AQUI

					

				sendHostMessage(client_message, host_port);

			}
			
			else{

				printf("\n\nElastic Container Service - Admin Container: Received failed.");

			}

		}

    }

}

int main() {

	pid_t pid;

	char parent_message[] = "hello";  // parent process will write this message

	char child_message[] = "goodbye"; // child process will then write this one

	void* shmem = create_shared_memory(128);

	memcpy(shmem, parent_message, sizeof(parent_message));

	pid = fork();

	if(pid == 0){

		printf("Child read: %s\n", shmem);
		memcpy(shmem, child_message, sizeof(child_message));
		printf("Child wrote: %s\n", shmem);

		subscribe_host();

	}

	else{

		admin_container();

		printf("Parent read: %s\n", shmem);
		sleep(1);
		printf("After 1s, parent read: %s\n", shmem);

	}

}