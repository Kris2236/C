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
        printf("Unable to open the file!\n");
        exit(errno);
    }
}

void format_file(char *input_file_name, int char_limit, char *output_file_name)
{
    FILE *fp_in = fopen(input_file_name, "r");
    FILE *fp_out = fopen(output_file_name, "w");
    check_file(fp_in);
    check_file(fp_out);
    char line[char_limit];
    int is_new_line;

    while(fgets(line, sizeof(line),fp_in))
    {
        is_new_line = 0;

        for(int i=0; i<char_limit; i++)
            if(line[i] == '\n')
                is_new_line = 1;

        if(is_new_line)
            fprintf(fp_out,"%s", line);
        else
            fprintf(fp_out,"%s\n", line);



        // Other method now, we can use bigger buffer size / char limit to write line, but buffer_length % char_limit = 0

//        if(strlen(line) <= char_limit)
//        {
//            fprintf(fp_out,"%s", line);
//        }
//        else
//        {
//            int l=0;
//            for(l=0; l<strlen(line)/char_limit; l++)
//            {
//                char *tmp_line = calloc(char_limit, sizeof(char));
//                for(int j=0; j<char_limit; j++)
//                    tmp_line[j] = line[j+l*char_limit];
//                fprintf(fp_out,"%s\n", tmp_line);
//            }
//
//            char *tmp_line = calloc(char_limit, sizeof(char));
//            for(int j=0; j<char_limit; j++)
//                tmp_line[j] = line[j+l*char_limit];
//            fprintf(fp_out,"%s", tmp_line);
//        }
    }

    fclose(fp_in);
    fclose(fp_out);
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
        printf("Provide:\n[input file name]\n[output file name]");
        for(int i=0; i<NUMBER_OF_ARGUMENTS; i++)
            arg[i] = read_arg();
    }
    else if (argc == 2)
    {
        printf("Provide:\n[output file name]");
        arg[0] = argv[1];
        for(int i=1; i<NUMBER_OF_ARGUMENTS; i++)
            arg[i] = read_arg();
    }
    else if (argc == 3)
    {
        for(int i=0; i<NUMBER_OF_ARGUMENTS; i++)
            arg[i] = argv[i+1];
    }
    else
    {
        printf("Wrong number of arguments (given %d expected 2).", argc);
        exit(0);
    }
}

void run(int argc, char *argv[])
{
    char **arg = calloc(1, sizeof(char*));
    parse_arg(argc, argv, arg);     // arg[0 - input file, 1 - output file

    format_file(arg[0], 50, arg[1]);

    free(arg);
}


int main(int argc, char *argv[]) {
    run(argc, argv);

    return 0;
}