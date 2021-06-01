#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "sem_fun.h"

void print_status(const int *arr, int size) {
    /* Print shared memory status */
    for (int i=0; i<size; i++)
        printf(" %d", arr[i]);
    printf("\n");
}

int empty_pos(const int *arr, int size, int sid) {
    for (int i=0; i<size; i++)
        if (arr[i] == EMPTY)
            return i;

    printf("Table status empty: %d, Oven status empty: %d",
           get_sem_val(sid, TABLE), get_sem_val(sid, BAKING));
    perror("[Cook]:Semaphores works wrong. Can't find empty space in shared array.");
    exit(2);
}

void bake_pizza(int sid, int pizza_type, struct pizzeria *piz) {
    dec_sem(sid, PUTTING_IN_OVEN);
    dec_sem(sid, BAKING);

    /* Get pizza position in oven - make sure that only you are choosing this position */
    int pizza_id = empty_pos((*piz).bake, MAX_TABLE_CAPACITY, sid);

    /* Put pizza to empty place in oven */
    (*piz).bake[pizza_id] = pizza_type;

    printf("[pid: %d timestamp: %f] Dodałem pizze: %d. Liczba pizz w piecu: %d.\n",
           getpid(), get_timestamp_mil(), pizza_type, MAX_OVEN_CAPACITY - get_sem_val(sid, BAKING));

    printf("oven: ");
    print_status((*piz).bake, MAX_OVEN_CAPACITY);

    inc_sem(sid, PUTTING_IN_OVEN);



    /* Baking time */
    sleep(rand() % 2 + 4);

    /* Pull out of oven */
    (*piz).bake[pizza_id] = EMPTY;
}

void put_on_table(int sid, int pizza_type, struct pizzeria *piz) {
    dec_sem(sid, PUTTING_ON_TABLE);
    dec_sem(sid, TABLE);

    /* Put on table */
    int table_pos = empty_pos((*piz).table_with_orders, MAX_TABLE_CAPACITY, sid);
    (*piz).table_with_orders[table_pos] = pizza_type;

    printf("[pid: %d timestamp: %f] Wyjmuję pizze: %d. Liczba pizz w piecu: %d. Liczba pizz na stole: %d.\n",
           getpid(), get_timestamp_mil(), pizza_type, MAX_OVEN_CAPACITY - get_sem_val(sid, BAKING)-1,
           MAX_TABLE_CAPACITY - get_sem_val(sid, TABLE));

    printf("table: ");
    print_status((*piz).table_with_orders, MAX_TABLE_CAPACITY);

    inc_sem(sid, PUTTING_ON_TABLE);
    inc_sem(sid, BAKING);
}

int prepare_pizza() {
    int pizza_type = rand() % 10;
    printf("[pid: %d timestamp: %f] Przygotowuje pizze: %d.\n", getpid(), get_timestamp_mil(), pizza_type);
    sleep(rand() % 2 + 1);
    return pizza_type;
}

int main() {
    int shmid;
    struct pizzeria *piz;
    key_t shmkey = create_shm(&shmid, 0, &piz);
    int sid = create_semaphore(shmkey, 0, 0);
    srand(getpid());    // randomize for each process
    printf("Cook pid: %d started work. SemID: %d\n", getpid(), sid);

    for(int i=0; i<PIZZA_PER_COOK; i++) {
        int pizza_type = prepare_pizza();
        bake_pizza(sid, pizza_type, piz);
        put_on_table(sid, pizza_type, piz);
    }

    return EXIT_SUCCESS;
}
