#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define BUFFER_SIZE 256
#define NUMBER_OF_ARGUMENTS 2


void check_file(FILE *fp)
{
    if(fp == NULL)
    {
        perror("Unable to open the file\n");
        exit(errno);
    }
}

int number_of_lines(char *arg)
{
    FILE *fp = fopen(arg, "r");
    check_file(fp);

    int count_lines = 0;
    char chr = getc(fp);

    while (chr != EOF)
    {
        if (chr == '\n')
            count_lines = count_lines + 1;

        chr = getc(fp);
    }

    fclose(fp);
    return count_lines;
}

void read_file_lines(char *name, char **lines)
{
    FILE *fp = fopen(name, "r");
    check_file(fp);
    char line[BUFFER_SIZE];

    for(int i=0; i< number_of_lines(name); i++)
    {
        fgets(line, sizeof(line),fp);
        char *tmp_line = calloc(BUFFER_SIZE, sizeof(char));

        for(int j=0; j<sizeof(line); j++)
            tmp_line[j] = line[j];

        lines[i] = tmp_line;
    }

    fclose(fp);
}

void parse_arg(int argc, char *argv[], char **arg)
{
    //// optionally parse: is digit
    if(argc != NUMBER_OF_ARGUMENTS)
        arg[0] = "data.txt";    // default settings
    else
        arg[0] = argv[1];   // you can type data file
}

void write_to_file(char *name, int even_nr)
{
    FILE *fp = fopen(name, "w");
    check_file(fp);
    fprintf(fp,"Liczb parzystych jest [%d]", even_nr );
    fclose(fp);
}

int count_even(char **lines, int lines_nr)
{
    int even_nr = 0;

    for(int i=0; i<lines_nr; i++)
        if(atoi(lines[i]) % 2 == 0)
            even_nr++;

    write_to_file("a.txt", even_nr);
    return even_nr;
}

int is_seven(char *line)
{
    if(line[strlen(line)-2] == '7')
        return 1;

    return 0;
}

void write_b(char **lines, int lines_nr, char *name)
{
    FILE *fp = fopen(name, "w");
    check_file(fp);

    for(int i=0; i< lines_nr; i++)
        if(atoi(lines[i]) % 10 == 0 || is_seven(lines[i]))
            fprintf(fp,"%s", lines[i] );

    fclose(fp);
}

int is_integer_root(int x)
{
    long long left = 1, right = x;

    while (left <= right)
    {
        long long mid = (left + right) / 2;

        if (mid * mid == x)
            return 1;

        if (mid * mid < x)
            left = mid + 1;
        else
            right = mid - 1;
    }

    return 0;
}

void write_c(char **lines, int lines_nr, char *name)
{
    FILE *fp = fopen(name, "w");
    check_file(fp);

    for(int i=0; i< lines_nr; i++)
        if(is_integer_root(atoi(lines[i])))
            fprintf(fp,"%s", lines[i] );

    fclose(fp);
}

void run(int argc, char *argv[])
{
    char **arg = calloc(1, sizeof(char *));
    parse_arg(argc, argv, arg);

    int len_file = number_of_lines(arg[0]);
    char **lines = calloc(len_file, sizeof(char *));

    read_file_lines(arg[0], lines);

    count_even(lines, len_file);
    write_b(lines, len_file, "b.txt");
    write_c(lines, len_file, "c.txt");

    free(lines);
    free(arg);
}


int main(int argc, char *argv[]) {
    run(argc, argv);

    return 0;
}
