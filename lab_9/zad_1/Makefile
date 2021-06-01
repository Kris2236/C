CFLAGS = -Wall
CC = gcc

main: main.o
	gcc -o main main.c -lpthread

clean:
	rm -f main.o main

test:  main
	@echo -e "\nTESTING\n"
	./main

all: main test
