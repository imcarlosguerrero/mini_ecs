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

#define HOST_NUMBER 200

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

	printf("\nElastic Container Service - Subscribe Host: Los Hosts Disponibles son: \n\n");   

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

	printf("Elastic Container Service - Subscribe Host: Socket Server created successfully.");

	char *filename = "hosts.txt";

	server.sin_family = AF_INET;

	server.sin_addr.s_addr = INADDR_ANY;

	server.sin_port = htons(6060);

	if(bind(subscribe_host, (struct sockaddr *) &server, sizeof(server)) < 0){

		perror("\n\nElastic Container Service - Subscribe Host: Socket Server bind failed. Error");

	}

	printf("\nElastic Container Service - Subscribe Host: Socket Server bind done.");

	while(1){

		listen(subscribe_host, 10);

		puts("\n\nElastic Container Service - Subscribe Host: Waiting for incoming connections...");

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

	int admin_container, client_sock, c, read_size, host_port = 8080;

	struct sockaddr_in server , client;

	char client_message[2000];

	int received = 0;

	admin_container = socket(AF_INET, SOCK_STREAM , 0);

	if(admin_container == -1){

		printf("\n\nElastic Container Service - Admin Container: Could not create Socket Server.\n\n");

	}

	puts("\nElastic Container Service - Admin Container: Socket Server created successfully.\n");

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

		puts("\n\nElastic Container Service - Admin Container: Waiting for incoming connections...\n\n");

		c = sizeof(struct sockaddr_in);

		client_sock = accept(admin_container, (struct sockaddr *) &client, (socklen_t*) &c);

		if(client_sock < 0){

			perror("\n\nElastic Container Service - Admin Container: Accept failed");

			return 1;

		}

		puts("Elastic Container Service - Admin Container: Connection accepted.");

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

	pid = fork();

	if(pid == 0){

		subscribe_host();

	}

	else{

		admin_container();

	}

}