C=gcc -Wall

all: build

build: client.c server.c
	$(C) -o client client.c -lrt
	$(C) -o server server.c -lrt

test: server
	./server

clean:
	rm -f client server