#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>
#include <inttypes.h>
#include <errno.h>


struct PGM_Map {
    int height;
    int width;
    int max_val;
    int **map;
} pgm_map;

struct Metadata{
    int threads_num;
    int method;
    char *input_file;
    char *output_file;
} data;


int str_to_int(char *str)
{
    uintmax_t num = strtoumax(str, NULL, 10);
    if (num == UINTMAX_MAX && errno == ERANGE)
    {
        printf("Can not cast to int \'%s\'", str);
        exit(EXIT_FAILURE);
    }

    return (int) num;
}

double get_timestamp_mil() {
    struct timeval  tv;
    gettimeofday(&tv, NULL);
    return ((double)(tv.tv_sec) * 1000 + (double)(tv.tv_usec) / 1000);
}

int negative(int p) { return pgm_map.max_val-p; }

void read_pgm( const char *path, struct PGM_Map *PGM_map)
{
    char *line = NULL;
    int line_nr = 0;
    size_t len = 0;
    FILE *fp = fopen(path, "r");

    if (fp == NULL)
        exit(EXIT_FAILURE);

    /* Read initial PGM params */
    while(line_nr < 3)
    {
        if (getline(&line, &len, fp) == -1)
        {
            perror("Cant read pgm params.\n");
            exit(EXIT_FAILURE);
        }

        bool skip_line = false;
        char *token = strtok(line, " ");

        while( token != NULL ) {
            if (token[0] == '#')
            {
                skip_line = true;
                break;
            }

            if (line_nr == 1)
            {
                (*PGM_map).width = str_to_int(token);
                token = strtok(NULL, " ");
                (*PGM_map).height = str_to_int(token);
            }
            else if (line_nr == 2)
            {
                (*PGM_map).max_val = str_to_int(token);
            }

            token = strtok(NULL, " ");
        }

        if (!skip_line)
            line_nr++;
    }

    /* Allocate memory */
    (*PGM_map).map = calloc((*PGM_map).width, sizeof(int *));
    for(int i=0; i<(*PGM_map).width; i++)
        (*PGM_map).map[i] = calloc((*PGM_map).height, sizeof(int));

    /* Write PGM map to struct */
    line_nr = 0;    // Now we are in first "array" row in file

    while (getline(&line, &len, fp) != -1)
    {
        char *token = strtok(line, " ");
        int id = 0;

        while( token != NULL ) {
            if (token[0] == '#')
                break;
            //printf( "[%d, %d]:%s", line_nr, id, token );
            (*PGM_map).map[id][line_nr] = str_to_int(token);
            token = strtok(NULL, " ");
            id++;
        }
        line_nr++;
    }

    fclose(fp);
    if (line)
        free(line);
}

void *thread_calc(void *arg)
{
    int *val_p = (int *) arg;
    printf("**** ThreadId: %lu started [%d, %d] ****\n", pthread_self(), val_p[0], val_p[1]);

    /* Measure time */
    double star_time = get_timestamp_mil();

    /* Calculate */
    for(int i=val_p[0]; i<=val_p[1]; i++)
        pgm_map.map[(int) i/pgm_map.height][i%pgm_map.height] = negative(pgm_map.map[(int) i/pgm_map.height][i%pgm_map.height]);

    double delta = (get_timestamp_mil() - star_time);
    void *time = (void *) &delta;

    free(arg);  // free memory after interceptions

    pthread_exit((void *)time);
}

void *thread_row_calc(void *arg)
{
    int *val_p = (int *) arg;
    printf("*** ThreadId: %lu started calculating row [%d, %d] ***\n", pthread_self(), val_p[0], val_p[1]);

    /* Measure time */
    double star_time = get_timestamp_mil();

    /* Calculate */
    for(int i=val_p[0]; i<=val_p[1]; i++)
        for (int j=0; j<pgm_map.height; j++)
            pgm_map.map[i][j] = negative(pgm_map.map[i][j]);
            //pgm_map.map[k][w]);
    double delta = (get_timestamp_mil() - star_time);
    void *time = (void *) &delta;

    free(arg);  // free memory after interceptions

    pthread_exit((void *)time);
}

void divide_task(pthread_t *thread) {
    int N = data.threads_num;
    int array_size = pgm_map.height * pgm_map.width;
    int chunk_size = array_size / N;
    int bonus = array_size - chunk_size * N;
    int start, end;
    int id = 0;
    if (N > array_size)
    {
        perror("Cant divide tasks. To many threads and to small array.\n");
        exit(EXIT_FAILURE);
    }

    /* Divide the array into equal intervals */
    for (start = 0, end = chunk_size; start < array_size; start = end, end = start + chunk_size, id++) {
        if (bonus) {
            end++;
            bonus--;
        }

        /* Run threads with args [start, end-1] */
        int *tmp = (int*)malloc((2)*sizeof(int));
        tmp[0] = start;
        tmp[1] = end-1;

        if (pthread_create(&thread[id], NULL, &thread_calc, (void *)tmp) != 0)
        {
            printf("Could not create thread %d/%d.\n", id+1, N);
            exit(EXIT_FAILURE);
        }

        printf("ThreadId: %lu %d/%d, arg: [%d,%d)\n", thread[id], id+1, N, start, end);
    }
}

void divide_row_task(pthread_t *thread)
{
    int N = data.threads_num;
    int array_size = pgm_map.width;
    int chunk_size = array_size / N;
    int bonus = array_size - chunk_size * N;
    int start, end;
    int id = 0;
    if (N > array_size)
    {
        perror("Cant divide tasks. To many threads and to small array.\n");
        exit(EXIT_FAILURE);
    }

    /* Divide the array into equal intervals */
    for (start = 0, end = chunk_size; start < array_size; start = end, end = start + chunk_size, id++) {
        if (bonus) {
            end++;
            bonus--;
        }

        /* Run threads with args [start, end-1] */
        int *tmp = (int*)malloc((2)*sizeof(int));
        tmp[0] = start;
        tmp[1] = end-1;

        if (pthread_create(&thread[id], NULL, &thread_row_calc, (void *)tmp) != 0)
        {
            printf("Could not create thread %d/%d.\n", id+1, N);
            exit(EXIT_FAILURE);
        }

        printf("ThreadId: %lu %d/%d, arg: [%d,%d)\n", thread[id], id+1, N, start, end);
    }
}

void print_pgm()
{
    printf("Width: %d, Height: %d, Max value: %d\n", pgm_map.width, pgm_map.height, pgm_map.max_val);
    for(int w=0; w<pgm_map.height; w++) {
        for (int k = 0; k < pgm_map.width; k++)
            printf("%d\t", pgm_map.map[k][w]);
        printf("\n");
    }
}

void free_data()
{
    for(int i=0; i<pgm_map.width; i++)
        free(pgm_map.map[i]);
    free(pgm_map.map);
}

void parse_args(int argc, char *argv[])
{
    if (argc != 5)
        perror("Wrong number of args. Expected [thread number] [dividing method]"
               " [input file name] [output file name]\n");

    data.threads_num = str_to_int(argv[1]);
    data.method = str_to_int(argv[2]);
    data.input_file = argv[3];
    data.output_file = argv[4];
}

void save_negative(char *path)
{
    FILE *fp = fopen(path, "w");

    fprintf(fp, "P2\n");
    fprintf(fp, "%d %d\n", pgm_map.width, pgm_map.height);
    fprintf(fp, "%d\n", pgm_map.max_val);

    for(int w=0; w<pgm_map.height; w++) {
        for (int k = 0; k < pgm_map.width; k++)
            fprintf(fp, "%d ", pgm_map.map[k][w]);
        fprintf(fp, "\n");
    }

    fclose(fp);
}

void choose_method(pthread_t *thread)
{
    if (data.method == 0)
    {
        printf("Automatic divider method:\n");
        divide_task(thread);
    }
    else if (data.method == 1)
    {
        printf("Blocks method:\n");
        divide_row_task(thread);
    }
    else
    {
        perror("Wrong method was given.\n");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[])
{
    parse_args(argc, argv);
    read_pgm(data.input_file, &pgm_map);    // read image to struct
    pthread_t thread[data.threads_num];     // thread id

    double star_time = get_timestamp_mil();

    choose_method(thread);

    /* Wait and get results */
    double *res;
    for(int i=0; i<data.threads_num; i++)
    {
        if (pthread_join(thread[i], (void **) &res) != 0)
        {
            printf("Can not receive from thread %lu", thread[i]);
            return 2;
        }

        printf("Thread %lu time: %f3\n", thread[i], *res);
    }

    double total_time = get_timestamp_mil() - star_time;
    printf("Main thread time: %f3\n\n", total_time);

    save_negative(data.output_file);
    //print_pgm();
    free_data();

    return 0;
}
