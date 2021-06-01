#ifndef ZAD_1_SEM_FUN_H
#define ZAD_1_SEM_FUN_H

#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <tgmath.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <inttypes.h>

#define DEC -1
#define INC 1
#define EMPTY -1
#define PIZZA_PER_COOK 2
#define MAX_OVEN_CAPACITY 5
#define MAX_TABLE_CAPACITY 5
#define KEY_PATH "../zad_1/shared_mem"
#define SEM_NUM 6

typedef enum operations {
    PUTTING_IN_OVEN = 0,
    BAKING = 1,
    TABLE = 2,
    PUTTING_ON_TABLE = 3,
    DELIVERY = 4,
    IS_READY = 5
} operation;

struct pizzeria{
    int cur_bake;
    int bake[MAX_OVEN_CAPACITY];
    int cur_order;
    int table_with_orders[MAX_OVEN_CAPACITY];
};

double get_timestamp_mil();
void dec_sem(int sid, int semnum);
void inc_sem(int sid, int semnum);
int get_sem_val(int sid, int sem_num);
int create_semaphore(key_t shmkey, int nsems, int sem_val);
key_t create_shm(int *shmid, size_t shmsize, struct pizzeria** shmaddr);

#endif //ZAD_1_SEM_FUN_H
