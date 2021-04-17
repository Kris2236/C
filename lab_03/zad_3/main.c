#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <dirent.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define NUMBER_OF_ARGUMENTS 3
#define BUFFER_SIZE 3
#define MAX_PATH_LEN 1000

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
        printf("Provide:\n[start location]\n[string to find]\n[max search depth]\n");
        for(int i=0; i<NUMBER_OF_ARGUMENTS; i++)
            arg[i] = read_arg();
    }
    else if (argc == 2)
    {
        printf("Provide:\\n[string to find]\n[max search depth]\n");
        arg[0] = argv[1];
        for(int i=1; i<NUMBER_OF_ARGUMENTS; i++)
            arg[i] = read_arg();
    }
    else if (argc == 3)
    {
        arg[0] = argv[1];
        arg[1] = argv[2];

        for(int i=2; i<NUMBER_OF_ARGUMENTS; i++)
            arg[i] = argv[i+1];
    }
    else if (argc == 4)
    {
        for(int i=0; i<NUMBER_OF_ARGUMENTS; i++)
            arg[i] = argv[i+1];
    }
    else
    {
        printf("Wrong number of arguments (given %d expected 3).", argc);
        exit(0);
    }
}

void search(char *start_path, char *file_name, int max_depth)
{
    DIR *d;
    struct dirent *dir;
    char full_path[MAX_PATH_LEN];
    d = opendir(start_path);

    if (d && max_depth > 0)
    {
        int child_pid = fork();

        if(child_pid == -1)
        {
            perror("Error occurred with process!\n");
            exit(2);
        }
        else if(child_pid == 0)
        {
            // child process
            while ((dir = readdir(d)) != NULL)
            {
                if(dir->d_type==DT_REG) // check only regular file
                {
                    full_path[0]='\0';
                    strcat(full_path,start_path);
                    strcat(full_path,"/");
                    strcat(full_path,dir->d_name);

                    if(strcmp(file_name, dir->d_name) == 0)
                        printf("%s found in %s/ in process %d\n",dir->d_name, start_path, getpid());

                    search(full_path, file_name, max_depth-1);
                }
            }
            closedir(d);
            exit(0);
        }
        else
        {
            // main process
            int res = wait(NULL);
            if(res == -1)
                printf("Ther is no process to wait for.\n");
            else
                printf("%d proces was finished.\n", res);
        }
    }
}

void run(int argc, char *argv[])
{
    char **arg = calloc(1, sizeof(char*));
    parse_arg(argc, argv, arg);     // 3 parameters [start location] [string to find] [max search depth]

    search(arg[0], arg[1], atoi(arg[2]));

    free(arg);
}


int main(int argc, char *argv[]) {
    run(argc, argv);

    return 0;
}
