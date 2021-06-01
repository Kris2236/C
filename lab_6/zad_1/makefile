Compile=gcc -Wall

all: build

build: client.c server.c
	$(Compile) -o client client.c
	$(Compile) -o server server.c

clean:
	rm -f client server