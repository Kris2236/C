CFLAGS = -Wall
CC = gcc

server: server.c
	gcc -o server server.c -lpthread

client: client.c
	gcc -o client client.c -lpthread

clean:
	rm -f server.o server
	rm -f client.o client

test:  server
	@echo -e "\nTESTING INET\n"
	./server INET 9001
	#./client client_name_1 INET 127.0.0.1 9001
	#./client client_name_2 INET 127.0.0.1 9001

all: clean server client test
