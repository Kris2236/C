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
    int iter = 10;

    while (iter>0) {
        for (int i=0; i<size; i++)
            if (arr[i] != EMPTY)
                return i;

        //printf("[Supplier] Table is empty. Supplier pid: %d is waiting for pizzas...\n", getpid());
        sleep(1);
        iter--;
    }

    printf("[Supplier]: No pizza to supply. Supplier pid %d is fired.\n", getpid());
    inc_sem(sid, DELIVERY);
    exit(0);
}

void supply_pizza(int sid, struct pizzeria *piz) {
    /* Choose available pizza - make sure that at the moment only you are choosing */
    dec_sem(sid, DELIVERY);

    /* Take pizza position from table - make sure that only you are choosing this position */
    int pizza_id = empty_pos((*piz).table_with_orders, MAX_TABLE_CAPACITY, sid);

    int pizza_type = (*piz).table_with_orders[pizza_id];
    (*piz).table_with_orders[pizza_id] = EMPTY;

    printf("table: ");
    print_status((*piz).table_with_orders, MAX_TABLE_CAPACITY);

    inc_sem(sid, TABLE);

    printf("[pid: %d timestamp: %f] Pobieram pizze: %d Liczba pizz na stole: %d.\n",
               getpid(), get_timestamp_mil(), pizza_type, MAX_TABLE_CAPACITY - get_sem_val(sid, TABLE));

    inc_sem(sid, DELIVERY);

    sleep(rand() % 2 + 4);      /* Trips to the customer */
    printf("[pid: %d timestamp: %f] Dostarczam pizze: %d.\n", getpid(), get_timestamp_mil(), pizza_type);
    sleep(rand() % 2 + 4);      /* Comes back */
}


int main() {
    int shmid, sid;
    struct pizzeria *piz;

    key_t shmkey = create_shm(&shmid, 0, &piz);
    sid = create_semaphore(shmkey, 0, 0);
    srand(getpid());    // randomize for each process
    printf("Supplier pid: %d started work. SemID: %d\n", getpid(), sid);

    while (1)
        supply_pizza(sid, piz);

    return EXIT_SUCCESS;
}
