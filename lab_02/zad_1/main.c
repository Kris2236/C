#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFERSIZE 256


int number_of_lines(char *file_name)
{
    FILE *fp;
    int count_lines = 0;
    char chr;

    fp = fopen(file_name, "r");
    chr = getc(fp);
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

void read_file_line_by_line(char *name, char **lines)
{
    FILE *fp = fopen(name, "r");
    if(fp == NULL)
    {
        perror("Unable to open the file\n");
        exit(1);
    }

    char line[BUFFERSIZE];

    for(int i=0; i< number_of_lines(name); i++)
    {
        fgets(line, sizeof(line),fp);
        char *tmp_line = calloc(BUFFERSIZE, sizeof(char));

        for(int j=0; j<sizeof(line); j++)
            tmp_line[j] = line[j];

        lines[i] = tmp_line;
    }

    fclose(fp);
}

char *read_arg()
{
    // Command buffer
    char *command = calloc(1,1), buffer[BUFFERSIZE];
    int pos = 0, input_size = 0;

    printf("Provide file name: ");
    fgets(buffer, BUFFERSIZE , stdin);

    // Parse command
    while (buffer[0] == '\n' != 0 || buffer[0] == ' ')
    {
        printf("Wrong input characters!\n$ ");
        fgets(buffer, BUFFERSIZE , stdin);
    }

    for (input_size; buffer[input_size] != '\0'; ++input_size);
    while (buffer[pos] != ' ' && buffer[pos] != '\n' && pos < input_size)
        pos++;

    command = calloc(pos, sizeof(char));

    for (int j = 0; j < pos; ++j)
        command[j] = buffer[j];

    return command;
}

char parse_arg(int argc, char *argv[], char **arg)
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

    for(int i=0; i<2; i++)
        printf("%s\n", arg[i]);
    printf("\n");
}

void merge_property(char **merged, char **lines_0, char **lines_1, int len_0, int len_1)
{
    int id =0;
    for(int i=0; i<len_0; i++)
    {
        merged[id] = lines_0[i];
        id++;
        merged[id] = lines_1[i];
        id++;
    }

    for(int i=0; i<len_1-len_0; i++)
    {
        merged[id] = lines_1[i+len_0];
        id++;
    }
}

void run(int argc, char *argv[])
{
    char **file_name = calloc(2, sizeof(char *));
    parse_arg(argc, argv, file_name);

    int len_file_0 = number_of_lines(file_name[0]);
    int len_file_1 = number_of_lines(file_name[1]);
    int len_merged = len_file_0 + len_file_1;

    char **lines_0 = calloc(len_file_0, sizeof(char *));
    char **lines_1 = calloc(len_file_1, sizeof(char *));
    char **merged = calloc(len_merged, sizeof(char *));

    read_file_line_by_line(file_name[0], lines_0);
    read_file_line_by_line(file_name[1], lines_1);

    if(len_file_0 < len_file_1)
        merge_property(merged, lines_0, lines_1, len_file_0, len_file_1);
    else
        merge_property(merged, lines_1, lines_0, len_file_1, len_file_0);

    print_tab(merged, len_merged);

    free(lines_0);
    free(lines_1);
    free(merged);
    free(file_name);
}


int main(int argc, char *argv[]) {
    run(argc, argv);

    return 0;
}
