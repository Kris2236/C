#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define BUFFER_SIZE 256
#define NUMBER_OF_ARGUMENTS 4


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

int match(char text[], char pattern[]) {
    int c, d, e, text_length, pattern_length, position = -1;

    text_length = strlen(text);
    pattern_length = strlen(pattern);

    if (pattern_length > text_length)
        return -1;

    for (c = 0; c <= text_length - pattern_length; c++) {
        position = e = c;

        for (d = 0; d < pattern_length; d++) {
            if (pattern[d] == text[e])
                e++;
            else
                break;
        }

        if (d == pattern_length)
            return position;
    }

    return -1;
}

void insert(char *source_str, int begin, char *str_to_insert, int len_replaced_str)
{
    char *line = calloc(BUFFER_SIZE, sizeof(char));
    int len = strlen(str_to_insert);
    strcpy(line, source_str);

    if(begin == 0)
    {
        // Making gap for string - we cant rewrite first element because we loose pointer to array
        for(int i=0; i<len; i++)
            line[i] = str_to_insert[i];

        for(int i=len; i<len+strlen(line)+1; i++)
            line[i] = source_str[i-len+len_replaced_str];
    }
    else
    {
        // Making gap for string
        for(int i=begin + len; i<strlen(line) + len; i++)
            line[i] = source_str[i-len+len_replaced_str];

        // Inserting sting
        for(int i=begin; i<begin+len; i++)
            line[i] = str_to_insert[i-begin];
    }

    for(int i=0; i<strlen(line) + len; i++)
        source_str[i] = line[i];

   free(line);
}

void parse_file(char *name, char **lines, char *n1, char *n2)
{
    FILE *fp = fopen(name, "r");
    check_file(fp);
    char line[BUFFER_SIZE];
    int i=0, position = -1;

    while(fgets(line, sizeof(line),fp))
    {
        position = match(line, n1);
        char *tmp_line = calloc(BUFFER_SIZE, sizeof(char));

        for(int j=0; j<sizeof(line); j++)
            tmp_line[j] = line[j];

        while(position != -1)
        {
            insert(tmp_line, position,n2, strlen(n1));
            position = match(tmp_line, n1);
        }

        lines[i] = tmp_line;
        i++;
    }

    fclose(fp);
}

char *read_arg()
{
    // Command buffer
    char *command, buffer[BUFFER_SIZE];
    int pos = 0, input_size = 0;

    printf("Provide arg: ");
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
        printf("Provide: [input file name]\n[output file name]\n[string 1]\n[string 2]\n");
        for(int i=0; i<NUMBER_OF_ARGUMENTS; i++)
            arg[i] = read_arg();
    }
    else if (argc == 2)
    {
        arg[0] = argv[1];
        printf("Provide:\n[output file name]\n[string 1]\n[string 2]\n");
        for(int i=1; i<4; i++)
            arg[i] = read_arg();
    }
    else if(argc == 3)
    {
        arg[0] = argv[1];
        arg[1] = argv[2];
        printf("Provide:\n[string 1]\n[string 2]\n");
        for(int i=2; i<NUMBER_OF_ARGUMENTS; i++)
            arg[i] = read_arg();
    }
    else if(argc == 4)
    {
        for(int i=0; i<3; i++)
            arg[i] = argv[i+1];

        printf("Provide:\n[string 2]\n");
        arg[3] = read_arg();
    }
    else if(argc == 5)
    {
        for(int i=0; i<NUMBER_OF_ARGUMENTS; i++)
            arg[i] = argv[i+1];
    }
    else
    {
        printf("Wrong number of arguments (given %d expected %d).", argc, NUMBER_OF_ARGUMENTS);
        exit(0);
    }

    printf("\"%s\" -> \"%s\"\n", arg[2], arg[3]);
}

void write_to_file(char ** lines, char *name, int lines_nr)
{
    FILE *fp = fopen(name, "w");
    check_file(fp);

    for(int i=0; i<lines_nr; i++)
        fprintf(fp,"%s", lines[i] );

    fclose(fp);
}

void run(int argc, char *argv[])
{
    char **arg = calloc(4, sizeof(char *));
    parse_arg(argc, argv, arg);     // arg[0 - input file, 1 - output file, 2 string_1, 3 - string_2]

    int file_lines_nr = number_of_lines(arg[0]);
    char **lines = calloc(file_lines_nr, sizeof(char *));
    parse_file(arg[0], lines, arg[2], arg[3]);
    write_to_file(lines, arg[1], file_lines_nr);

    free(lines);
    free(arg);
}


int main(int argc, char *argv[]) {
    run(argc, argv);

    return 0;
}