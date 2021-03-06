CC=gcc

%.o: %.c
	$(CC) -c -o $@ $<

all: elastic_container_service host_1 host_2

elastic_container_service: elastic_container_service.o
	gcc -o elastic_container_service elastic_container_service.o -lrt && ./elastic_container_service

main: main.o

	gcc -o main main.o && ./main

host_1: host_1.o
	gcc -o host_1 host_1.o && ./host_1

host_2: host_2.o
	gcc -o host_2 host_2.o && ./host_2

host_3: host_3.o
	gcc -o host_3 host_3.o && ./host_3

host_4: host_4.o
	gcc -o host_4 host_4.o && ./host_4

host_5: host_5.o
	gcc -o host_5 host_5.o && ./host_5

host_6: host_6.o
	gcc -o host_6 host_6.o && ./host_6

clean:
	rm -f -r *.o *.pyc __pycache__ .vscode && clear && clear
