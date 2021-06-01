#include <stdlib.h>
#include <sys/time.h>
#include <sys/sem.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/shm.h>

#include "sem_fun.h"


union semun {
    int val;
    struct semid_ds *buf;
    unsigned short  *array;
};

key_t create_shm(int *shmid, size_t shmsize, struct pizzeria** shmaddr) {
    /* Create key */
    key_t shmkey;
    sleep(1);
    if ((shmkey = ftok(KEY_PATH, 'S')) == -1) {
        perror("ftok(). You schould change in esm_func.h KEY_PATH to your location");
        exit(1);
    }

    /* Create shared memory segment */
    if ((*shmid = shmget(shmkey, shmsize, IPC_CREAT | S_IRWXU)) == -1) {
        perror("Could not create shm id.\n");
        exit(1);
    }

    /* attach to the segment to get a pointer to it: */
    *shmaddr = shmat(*shmid, NULL, 0);
    if (*shmaddr == (struct pizzeria*)(-1)) {
        perror("shmat:Could not attach to the segment to get a pointer to it!\n");
        exit(1);
    }

    return shmkey;
}

void set_sem_val(int sid, int id, int sem_val) {
    /* Set semaphore initial value */
    union semun arg;
    arg.val = sem_val;

    if (semctl(sid, id, SETVAL, arg) < 0) {
        perror("Could not set semaphore initial value");
        exit(1);
    }
}

int create_semaphore(key_t shmkey, int nsems, int sem_val) {
    int sid;
    if ((sid = semget(shmkey, nsems, IPC_CREAT | S_IRWXU)) < 0) {
        perror("Could not create semaphore's set id");
        exit(1);
    }

    /* set each semaphore to property value */
    for (int i=0; nsems>0 && i<nsems; i++) {
        switch ( i )
        {
            case PUTTING_IN_OVEN:
                set_sem_val(sid, i, 1);
                break;
            case BAKING:
                set_sem_val(sid, i, sem_val);
                break;
            case TABLE:
                set_sem_val(sid, i, sem_val);
                break;
            case PUTTING_ON_TABLE:
                set_sem_val(sid, i, 1);
                break;
            case DELIVERY:
                set_sem_val(sid, i, 1);
                break;
            case IS_READY:
                set_sem_val(sid, i, 1);
                break;
            default:
                perror("Wrong number of semaphores!");
                exit(2);
        }
    }

    return sid;
}

double get_timestamp_mil() {
    struct timeval  tv;
    gettimeofday(&tv, NULL);
    return ((tv.tv_sec) * 1000 + (tv.tv_usec) / 1000);
}

void dec_sem(int sid, int semnum) {
    struct sembuf sops;
    sops.sem_num = semnum;
    sops.sem_op = DEC;
    sops.sem_flg = 0;

    if (semop(sid, &sops, 1) == -1) {
        perror("semop");
        exit(1);
    }
}

void inc_sem(int sid, int semnum) {
    struct sembuf sops;
    sops.sem_num = semnum;
    sops.sem_op = INC;
    sops.sem_flg = 0;

    if (semop(sid, &sops, 1) == -1) {
        perror("semop");
        exit(1);
    }
}

int get_sem_val(int sid, int sem_num) {
    int sem_val;
    if ((sem_val = semctl(sid, sem_num, GETVAL, NULL)) < 0) {     /* Get semaphore value */
        perror("Could not set semaphore initial value");
        exit(1);
    }
    return sem_val;
}
