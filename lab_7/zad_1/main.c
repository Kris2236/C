#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>
#include <wait.h>

#include "sem_fun.h"


void delete_shm(int shmid, struct pizzeria **shmaddr) {
    /* Detach from the segment: */
    if (shmdt(*shmaddr) == -1) {
        perror("shmdt");
        exit(1);
    }

    /* Delete the memory segment */
    shmctl(shmid, IPC_RMID, NULL);
}

void init_struct(struct pizzeria *piz) {
    /* Set initial value */
    piz->cur_bake = 0;
    piz->cur_order = 0;

    for(int i=0; i<MAX_OVEN_CAPACITY; i++)
        piz->bake[i] = EMPTY;

    for(int i=0; i<MAX_TABLE_CAPACITY; i++)
        piz->table_with_orders[i] = EMPTY;
}


int main(int argc, char *argv[]) {
    int shmid;
    struct pizzeria *piz;

    /* Parse args */
    if (argc != 3) {
        perror("Wrong number of argument.\n");
        exit(1);
    }
    int cookers = atoi(argv[1]);
    int suppliers = atoi(argv[2]);

    /* Create shared memory and semaphores */
    key_t shmkey = create_shm(&shmid, sizeof(struct pizzeria), &piz);
    create_semaphore(shmkey, SEM_NUM, 5);
    init_struct(piz);
    pid_t pid;
    fflush(stdout);

    /* Run cookers */
    for(int i=0; i<cookers; i++)
    {
        pid = fork();
        if (pid < 0) {
            perror("fotk()");
            exit(1);
        }

        if(pid==0) {
            char* argument_list[] = {"./cook", NULL};
            if (execvp("./cook", argument_list) == -1) {
                perror("Could not run cook program.\n");
                exit(1);
            }
        }
    }

    /* Run suppliers */
    for(int i=0; i<suppliers; i++)
    {
        pid = fork();
        if (pid < 0) {
            perror("fotk()");
            exit(1);
        }

        if(pid==0) {
            char* argument_list[] = {"./supplier", NULL};
            if (execvp("./supplier", argument_list) == -1) {
                perror("Could not run supplier program.\n");
                exit(1);
            }
        }
    }

    /* wait for execution */
    for(int i=0; i<cookers+suppliers; i++)
        wait(NULL);

    delete_shm(shmid, &piz);
    return 0;
}
