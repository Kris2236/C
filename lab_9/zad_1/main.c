#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_REINDEER_HERD 9
#define MAX_ELF 10
#define MAX_GIFTS 3

bool end_routine = false;
pthread_mutex_t mutex;
pthread_cond_t cond;

int reindeers_ready = 0;
int reindeers_blocked_by_santa = 0;
pthread_mutex_t mutex_reindeer;
pthread_cond_t cond_reindeer;

int elves_problem = 0;
bool elf_problem_map[MAX_ELF];
pthread_mutex_t mutex_elf;
pthread_cond_t cond_elf;


int generate_rand(int min, int max)
{
    srand(pthread_self());
    return (rand() % (max-min + 1) + min);
}

void solve_elves_problems() {
    for(int i = 0; i < MAX_ELF; i++) {
        if(elf_problem_map[i] == 1) {
            printf(" [Santa]: helps elf %d\n", i);
            elf_problem_map[i] = 0;
        }
    }

    elves_problem -=3;
}

void santa_wait_for_notification()
{
    while( !(reindeers_ready==MAX_REINDEER_HERD || elves_problem >=3) )
    {
        printf(" [Santa]: still sleeping. There are only %d reindeers and %d elves.\n", reindeers_ready, elves_problem);
        pthread_cond_wait(&cond, &mutex);
    }
}

void give_gift()
{
    /* Lock reindeers */
    pthread_mutex_lock(&mutex_reindeer);
    reindeers_blocked_by_santa = MAX_REINDEER_HERD;

    printf(" [Santa]: giving gifts.\n");
    sleep(generate_rand(2, 4));
    pthread_mutex_unlock(&mutex);

    pthread_mutex_unlock(&mutex_reindeer);
    pthread_cond_broadcast(&cond_reindeer); // Notify cond all threads waiting

    /* Wait for all reindeers leave */
    pthread_mutex_lock(&mutex_reindeer);
    while(reindeers_blocked_by_santa > 0)
        pthread_cond_wait(&cond_reindeer, &mutex_reindeer);

    pthread_mutex_unlock(&mutex_reindeer);
}

void solve_problems()
{
    /* Solve elves problem - santa is not available */
    pthread_mutex_lock(&mutex_elf);
    solve_elves_problems();
    sleep(generate_rand(2,4));
    pthread_mutex_unlock(&mutex_elf);

    pthread_cond_broadcast(&cond_elf); // Notify elves - you can notify problems
    pthread_mutex_unlock(&mutex);
}

void *santa_routine()
{
    int iter = 1;
    while (iter <= MAX_GIFTS)
    {
        printf(" [Santa]: starts snoring.\n");

        pthread_mutex_lock(&mutex);
        santa_wait_for_notification();

        if (reindeers_ready == MAX_REINDEER_HERD)
        {
            give_gift();
            printf("\tITERATION %d\n\n", iter);
            iter++;
        }
        else if (elves_problem == 3)
        {
            solve_problems();
        }
        else
        {
            perror("To many elves or wrong reindeers number.\n");
            exit(EXIT_FAILURE);
        }
    }

    end_routine = true;
    pthread_exit(0);
}

void reindeer_wait_for_santa()
{
    pthread_mutex_lock(&mutex_reindeer);
    while(reindeers_blocked_by_santa == 0)
        pthread_cond_wait(&cond_reindeer, &mutex_reindeer);

    reindeers_blocked_by_santa--;
    pthread_mutex_unlock(&mutex_reindeer);
}

void *reindeer_routine()
{
    pthread_mutex_lock(&mutex);
    reindeers_ready++;
    pthread_mutex_unlock(&mutex);

    while(!end_routine)
    {
        /* Reinder is gone */
        pthread_mutex_lock(&mutex);
        reindeers_ready--;
        printf("[Reindeer] bought a ticket to warm countries.\n");

        /** Note, if we change reindeer holiday time, or reindeer will be not release same time
         * we should add one more mutex for notify santa
         * because this broadcast can allow another reinder continue (multiple iteration) without santa permission.
         * */
        pthread_cond_broadcast(&cond_reindeer); // Notify santa - reindeer is gone
        pthread_mutex_unlock(&mutex);

        sleep(generate_rand(5,10));

        /* Reinder come back */
        pthread_mutex_lock(&mutex);
        reindeers_ready++;
        printf("[Reindeer] come back. %d/%d\n", reindeers_ready, MAX_REINDEER_HERD);

        pthread_mutex_unlock(&mutex);

        pthread_cond_broadcast(&cond);          // Notify santa - reindeer come back

        /* Wait for santa */
        reindeer_wait_for_santa();
    }

    pthread_exit(0);
}

void wait_for_santa(int id)
{
    pthread_mutex_lock(&mutex_elf);
    while (elves_problem >= 3)
    {
        printf("[Elf]: %d is waiting for come back elves.\n", id);
        pthread_cond_wait(&cond_elf, &mutex_elf);
    }
    pthread_mutex_unlock(&mutex_elf);
}

void notify_issue(int id)
{
    pthread_mutex_lock(&mutex);
    elves_problem++;

    if (elves_problem < 3)
    {
        elf_problem_map[id] = true;
        printf("[Elf]: %d is waiting %d/3 elves for Santa\n", id, elves_problem);
    }
    else if (elves_problem == 3)
    {
        elf_problem_map[id] = true;
        printf("[Elf]: %d waking up santa! 3/3 problems.\n", id);
        pthread_cond_broadcast(&cond);
    }
    else
    {
        elves_problem = 3;
    }
    pthread_mutex_unlock(&mutex);
}

void *elf_routine(void * arg)
{
    int *tmp = (int *) arg;
    int id = *tmp;
    sleep(generate_rand(2,5));

    while(!end_routine)
    {
        wait_for_santa(id);
        notify_issue(id);

        sleep(generate_rand(2,5));
    }

    free(arg);
    pthread_exit(0);
}

void init_cond_mutex()
{
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    pthread_mutex_init(&mutex_reindeer, NULL);
    pthread_cond_init(&cond_reindeer, NULL);

    pthread_mutex_init(&mutex_elf, NULL);
    pthread_cond_init(&cond_elf, NULL);
}

void destroy_cond_mutex()
{
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    pthread_mutex_destroy(&mutex_reindeer);
    pthread_cond_destroy(&cond_reindeer);

    pthread_mutex_destroy(&mutex_elf);
    pthread_cond_destroy(&cond_elf);
}

void create_thread(int threads_num, pthread_t *thread, void *routine, bool is_arg)
{
    for (int i=0; i<threads_num; i++)
    {
        int *id = malloc(sizeof(int*));
        if (is_arg)
            *id = i;
        else
            id = NULL;

        if (pthread_create(&thread[i], NULL, routine, (void *) id) != 0)
        {
            perror("Could not create thread.\n");
            exit(EXIT_FAILURE);
        }
    }
}

int main()
{
    pthread_t santa_claus;
    pthread_t reindeer[MAX_REINDEER_HERD];
    pthread_t elf[MAX_ELF];

    init_cond_mutex();

    // create Santa thread
    if (pthread_create(&santa_claus, NULL, &santa_routine, NULL) != 0)
    {
        perror("Could not create thread.\n");
        return 1;
    }

    create_thread(MAX_REINDEER_HERD, reindeer, &reindeer_routine, false);
    create_thread(MAX_ELF, elf, &elf_routine, true);

    // wait just for santa's thread
    if (pthread_join(santa_claus, NULL) != 0)
    {
        perror("Can not join santa thread.\n");
        exit(EXIT_FAILURE);
    }

    /* terminate threads */
    for (int i=0; i<MAX_REINDEER_HERD; i++)
        pthread_cancel(reindeer[i]);

    for (int i=0; i<MAX_ELF; i++)
        pthread_cancel(elf[i]);
    
    printf("Terminated all threads.\n");

    destroy_cond_mutex();

    return 0;
}
