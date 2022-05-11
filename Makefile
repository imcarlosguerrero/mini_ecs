CC=gcc

%.o: %.c
	$(CC) -c -o $@ $<

all: elastic_container_service host_1 host_2

elastic_container_service: elastic_container_service.o
	gcc -o elastic_container_service elastic_container_service.o -lrt && ./elastic_container_service

host_1: host_1.o
	gcc -o host_1 host_1.o && ./host_1

host_2: host_2.o
	gcc -o host_2 host_2.o && ./host_2
	
clean:
	rm -f -r *.o *.pyc __pycache__ .vscode && clear && clear
