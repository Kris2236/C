CFLAGS = -Wall
CC = gcc

main: main.o
	gcc -o main main.c -lpthread

clean:
	rm -f main.o main

test:  main
	@echo -e "\nTESTING\n"
	./main 2 0 input.pgm negative.pgm
	./main 2 1 input.pgm negative.pgm

	./main 2 0 input_big.pgm negative_big.pgm
	./main 2 1 input_big.pgm negative_big.pgm

	./main 7 0 input_big.pgm negative_big.pgm
	./main 7 1 input_big.pgm negative_big.pgm

all: main test
