#ifndef mylib_h
#define mylib_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <time.h>


int64_t get_line(char **line, size_t *len, FILE *fp);
int number_of_lines(char *name);
int min(int a, int b);
struct File read_file(char *f_name);
struct File merge_pair(char *f1_name, char *f2_name);
void merge(char **arg, int nr_arg);
int line_number(int block_number);
void delete_line(int block_number, int line_nr);
void delete_block(int block_number);
void save_merged(int block_number);
void print_block(int block_number);
void create_table(int size);
void clear_memory();
void print_help();
char *parse_arg(char *buffer, int begin, int end);
void measure_execution_time(char **arg, int nr_arg);
int execute(char *command, char **arg, int nr_arg);
void run_commander();

#endif
