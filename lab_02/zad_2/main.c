#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define BUFFER_SIZE 256


void check_file(FILE *fp)
{
    if(fp == NULL)
    {
        printf("Unable to open the file!\n");
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

void print_tab(char **arr, int len)
{
    for(int i=0; i<len; i++)
        printf("%s", arr[i]);
}

int filter_file_lines(char *name, char **lines, char c)
{
    FILE *fp = fopen(name, "r");
    check_file(fp);
    
    char line[BUFFER_SIZE];
    int id = 0;
    int is_char_in_line;

    for(int i=0; i< number_of_lines(name); i++)
    {
        is_char_in_line = 0;
        fgets(line, sizeof(line),fp);
        char *tmp_line = calloc(BUFFER_SIZE, sizeof(char));

        for(int j=0; j<sizeof(line); j++)
        {
            if(line[j] == c)
                is_char_in_line = 1;

            tmp_line[j] = line[j];
        }

        if(is_char_in_line)
        {
            lines[id] = tmp_line;
            id ++;
        }
    }

    fclose(fp);
    return id;
}

char *read_arg()
{
    // Command buffer
    char *command, buffer[BUFFER_SIZE];
    int pos = 0, input_size = 0;

    printf("Provide arg: [character] [file_name] ");
    fgets(buffer, BUFFER_SIZE , stdin);

    // Parse command
    while (buffer[0] == '\n' != 0 || buffer[0] == ' ')
    {
        printf("Wrong input characters! errno = %i\n", errno);
        fgets(buffer, BUFFER_SIZE , stdin);
    }

    for (; buffer[input_size] != '\0'; ++input_size);
    while (buffer[pos] != ' ' && buffer[pos] != '\n' && pos < input_size)
        pos++;

    command = calloc(pos, sizeof(char));

    for (int j = 0; j < pos; ++j)
        command[j] = buffer[j];

    return command;
}

void parse_arg(int argc, char *argv[], char **arg)
{
    if(argc == 1)
    {
        for(int i=0; i<2; i++)
            arg[i] = read_arg();
    }
    else if (argc == 2)
    {
        arg[0] = argv[1];
        arg[1] = read_arg();
    }
    else if(argc == 3)
    {
        for(int i=1; i<3; i++)
            arg[i-1] = argv[i];
    }

    // character parser
    if(strlen(arg[0]) != 1)
    {
        printf("Wrong first argument (have to be single character)! errno = %i\n", errno);
        exit(errno);
    }

    for(int i=0; i<2; i++)
        printf("%s\n", arg[i]);
    printf("\n");
}

void run(int argc, char *argv[])
{
    char **arg = calloc(2, sizeof(char *));
    parse_arg(argc, argv, arg);

    char c = *arg[0];
    printf("filtering character: '%c'\n\n", c);

    int len_file = number_of_lines(arg[1]);
    char **lines = calloc(len_file, sizeof(char *));
    int new_file_len = filter_file_lines(arg[1], lines, c);
    print_tab(lines, new_file_len);

    free(lines);
    free(arg);
}


int main(int argc, char *argv[]) {
    run(argc, argv);

    return 0;
}
