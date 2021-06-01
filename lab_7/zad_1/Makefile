CFLAGS = -Wall
CC = gcc

main: main.c
	gcc -o main main.c sem_fun.c

cook: cook.c
	gcc -o cook cook.c sem_fun.c

supplier: supplier.c
	gcc -o supplier supplier.c sem_fun.c

clean:
	rm -f main.o main
	rm -f cook.o cook
	rm -f supplier.o supplier
	rm -f sem_fun.o sem_fun

test:  main
	@echo -e "\nTESTING\n"
	./main 2 4
	@echo -e "\nTESTING\n"
	./main 7 6

all: clean cook supplier main test
