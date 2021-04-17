#include <ctype.h>
#include <time.h>
#include "../include/my_lib.h"

#define BUFFERSIZE 128


struct File{
    char **line;
    int size;
    int actual_size;
};

struct Main_data{
    int nr_of_merged_files;
    struct File *merged;
}main_data = {0};


int64_t get_line(char **line, size_t *len, FILE *fp)
{
    // Check if either line, len or fp are NULL pointers
    if (line == NULL || len == NULL || fp == NULL)
    {
        errno = EINVAL;
        return -1;
    }

    // Use a chunk array of 128 bytes as parameter for fgets
    char chunk[128];

    // Allocate a block of memory for *line if it is NULL or smaller than the chunk array
    if (*line == NULL || *len < sizeof(chunk))
    {
        *len = sizeof(chunk);
        if ((*line = (calloc(1, *len))) == NULL)
        {
            errno = ENOMEM;
            return -1;
        }
    }

    (*line)[0] = '\0';

    while (fgets(chunk, sizeof(chunk), fp) != NULL)
    {
        size_t len_used = strlen(*line);
        size_t chunk_used = strlen(chunk);

        if (*len - len_used < chunk_used)
        {
            // Check for overflow
            if (*len > SIZE_MAX / 2)
            {
                errno = EOVERFLOW;
                return -1;
            }
            else
            {
                *len *= 2;
            }

            if ((*line = (realloc(*line, *len))) == NULL)
            {
                errno = ENOMEM;
                return -1;
            }
        }

        memcpy(*line + len_used, chunk, chunk_used);
        len_used += chunk_used;
        (*line)[len_used] = '\0';

        if ((*line)[len_used - 1] == '\n')
            return len_used;
    }

    return -1;
}

int number_of_lines(char *name)
{
    FILE *file_ptr;
    int count_lines = 0;
    char chr;

    file_ptr = fopen(name, "r");
    chr = getc(file_ptr);
    while (chr != EOF)
    {
        if (chr == '\n')
            count_lines = count_lines + 1;

        chr = getc(file_ptr);
    }
    fclose(file_ptr);

    return count_lines;
}

int min(int a, int b)
{
    if (a < b)
        return a;

    return b;
}

struct File read_file(char *f_name)
{
    FILE *fp = fopen(f_name, "r");
    if (fp == NULL)
    {
        printf("Could not open file %s", f_name);
        exit(1);
    }

    struct File file;
    file.size = number_of_lines(f_name);
    file.actual_size = file.size;
    file.line = calloc(file.size, sizeof(char *));

    char *line = NULL;
    size_t len = 0;
    int id = 0;

    while (get_line(&line, &len, fp) != -1)
    {
        char *tmp_line = (calloc(strlen(line), sizeof(char)));
        for (size_t j = 0; j < strlen(line); j++)
            tmp_line[j] = line[j];

        file.line[id] = tmp_line;
        id++;
    }
    fclose(fp);
    free(line);

    return file;
}

struct File merge_pair(char *f1_name, char *f2_name)
{
    struct File files[2];
    files[0] = read_file(f1_name);
    files[1] = read_file(f2_name);

    // Allocate memory
    struct File merged;
    merged.size = files[0].size + files[1].size;
    merged.line = calloc(merged.size, sizeof(char *));

    // Protection against different file lengths
    int merged_size = min(files[0].size, files[1].size);
    files[0].actual_size = merged_size;
    files[1].actual_size = merged_size;
    merged.actual_size = 2*merged_size;

    // Merge
    int line_id = 0;
    int file_line = 0;
    while (line_id < merged.actual_size)
    {
        merged.line[line_id++] = files[0].line[file_line];
        merged.line[line_id++] = files[1].line[file_line++];
    }

    return merged;
}

void merge(char **arg, int nr_arg)
{
    if(main_data.nr_of_merged_files == 0)
    {
        printf("You have to first create_table _.\n");
        return;
    }

    int arg_id = 0;
    int block_id = 0;

    while(arg_id+1 < nr_arg)
    {
        main_data.merged[block_id] = merge_pair(arg[arg_id], arg[arg_id+1]);
        arg_id += 2;
        block_id++;
    }
}

int line_number(int block_number)
{
    if(block_number > main_data.nr_of_merged_files || block_number < 0 || main_data.merged[block_number].line == NULL)
    {
        printf("Wrong block id: %d.", block_number);
        return 0;
    }

    return main_data.merged[block_number].actual_size;
}

void delete_line(int block_number, int line_nr)
{
    if(block_number > main_data.nr_of_merged_files || block_number < 0 || main_data.merged[block_number].line == NULL)
    {
        printf("Cant delete line %d from block nr: %d. Wrong block id.", line_nr, block_number);
        return;
    }

    if(line_nr > main_data.merged[block_number].actual_size || line_nr <0)
    {
        printf("Cant delete line %d from block nr: %d. Wrong line id.", line_nr, block_number);
        return;
    }

    // Finding real line id including NULL line (deleted)
    int real_id=0;

    for(int i=0; i<line_nr; i++, real_id++)
        while(main_data.merged[block_number].line[real_id] == NULL)
            real_id++;

    // Removed line is replaced by NULL
    free(main_data.merged[block_number].line[real_id]);

    main_data.merged[block_number].line[real_id] = NULL;
    main_data.merged[block_number].actual_size -= 1;
}

void delete_block(int block_number)
{
    if(block_number > main_data.nr_of_merged_files || block_number < 0 || main_data.merged[block_number].line == NULL)
    {
        printf("Cant print block nr: %d. Wrong block id.", block_number);
        return;
    }

    // Finding real block id including number line = 0 (deleted)
    int real_id=0;

    for(int i=0; i<block_number; i++, real_id++)
        while(main_data.merged[real_id].actual_size == 0)
            real_id++;

    // Removed block is represented by actual_size line = 0
    for(int i=0; i<main_data.merged[real_id].size; i++)
        delete_line(real_id, 0);

    free(main_data.merged[real_id].line);

    main_data.merged[real_id].actual_size = 0;
    main_data.merged[real_id].size = 0;
    main_data.nr_of_merged_files -= 1;
}

void save_merged(int block_number)
{
    if(block_number > main_data.nr_of_merged_files || block_number < 0 || main_data.merged[block_number].line == NULL)
    {
        printf("Wrong block id: %d.", block_number);
        return;
    }

    FILE *file;
    if((file = fopen("/tmp/merged_tmp.txt", "w")) != NULL)
    {
        for(int i=0; i<main_data.merged[0].size; i++)
        {
            // Skip deleted lines
            while(main_data.merged[0].line[i] == NULL)
                i++;

            // Prevents going beyond the boards
            if(i >= main_data.merged[0].size)
                break;

            fprintf(file, "%s", main_data.merged[0].line[i]);
        }

        fclose(file);
    }
    else
    {
        printf("Can not save data in \"/tmp/merged_tmp.txt\"!\n");
    }
}

void print_block(int block_number)
{
    if(main_data.nr_of_merged_files == 0)
    {
        printf("Wrong block id");
        return;
    }

    if(block_number > main_data.nr_of_merged_files || block_number < 0 || main_data.merged[block_number].line == NULL)
    {
        printf("Cant print block nr: %d. Wrong block id.", block_number);
        return;
    }

    printf("\n");
    for(int i=0; i<main_data.merged[block_number].size; i++)
        if(main_data.merged[block_number].line[i] != NULL)
            printf("%s", main_data.merged[block_number].line[i]);
}

void create_table(int size)
{
    if(main_data.nr_of_merged_files != 0)
    {
        printf("Table is already created!");
        return;
    }

    main_data.merged = calloc(size, sizeof(struct File*) + 2*sizeof(int));
    main_data.nr_of_merged_files = size;
}

void clear_memory()
{
    if(main_data.nr_of_merged_files != 0)
        free(main_data.merged);
}

void print_help()
{
    printf("[Help] options:\n"
           "\tcreate_table [number of block]\n"
           "\tdelete_line [block id] [line id]\n"
           "\tdelete_block [block id]\n"
           "\texit\n"
           "\thelp / h\n"
           "\tprint_block\n"
           "\tline_number [block id]\n"
           "\tmeasure_execution_time [file 1] [file 2] ...\n"
           "\tmerge [file 1] [file 2] ...\n"
           "\tsave_merged [block id]\n"
    );
}

char *parse_arg(char *buffer, int begin, int end)
{
    char *tmp = calloc(end-begin, sizeof(char ));

    for(int i=begin; i<end; i++)
        tmp[i-begin] = buffer[i];

    return tmp;
}

void measure_execution_time(char **arg, int nr_arg)
{
    FILE *file;
    if((file = fopen("time_log.txt", "w")) == NULL)
    {
        printf("Can not save time logs to 'time_log.txt'\n");
        exit(1);
    }

    clock_t start, total, end;

    create_table(4);

    // 1. Merge time
    start = clock();

    merge(arg, nr_arg);

    end = clock();
    total = end - start;

    for(int i=0; i<main_data.nr_of_merged_files; i++)
        printf("Number of lines in block %d is %d \n", i, line_number(i));

    fprintf(file, "merge() \t\t: %f s\n", ((double) (end - start)) / CLOCKS_PER_SEC);



    // 2 . Save file
    start = clock();

    save_merged(0);
    save_merged(3);

    end = clock();
    total += end - start;
    fprintf(file, "save_merged() \t: %f s\n", ((double) (end - start)) / CLOCKS_PER_SEC);


    // 3. Delete block
    start = clock();

    delete_block(0);

    end = clock();
    total += end - start;

    fprintf(file, "delete_block() \t: %f s\n", ((double) (end - start)) / CLOCKS_PER_SEC);
    fprintf(file, "Total_time \t\t: %f s\n\n", ((double) (total)) / CLOCKS_PER_SEC);

    fclose(file);

    clear_memory();
    exit(0);
}

int execute(char *command, char **arg, int nr_arg)
{
    if(strcmp(command, "exit") == 0)
    {
        clear_memory();
        return -1;
    }
    else if(strcmp(command, "create_table") == 0) {
        if (nr_arg != 1)
            return 0;

        create_table(atoi(arg[0]));
    }
    else if(strcmp(command, "delete_line") == 0)
    {
        if(nr_arg != 2)
            return 0;

        delete_line(atoi(arg[0]), atoi(arg[1]));
    }
    else if(strcmp(command, "delete_block") == 0)
    {
        if(nr_arg != 1)
            return 0;

        delete_block(atoi(arg[0]));
    }
    else if(strcmp(command, "print_block") == 0)
    {
        if(nr_arg != 1)
            return 0;

        print_block(atoi(arg[0]));
    }
    else if(strcmp(command, "line_number") == 0)
    {
        if(nr_arg != 1)
            return 0;

        printf("%d\n", line_number(atoi(arg[0])));
    }
    else if(strcmp(command, "measure_execution_time") == 0)
    {
        if(nr_arg % 2 != 0)
            return 0;

        measure_execution_time(arg, nr_arg);
    }
    else if(strcmp(command, "merge") == 0)
    {
        if(nr_arg % 2 != 0)
            return 0;

        merge(arg, nr_arg);
    }
    else if(strcmp(command, "save_merged") == 0)
    {
        if(nr_arg != 1)
            return 0;

        save_merged(atoi(arg[0]));
    }
    else if(strcmp(command, "help") == 0 || strcmp(command, "h") == 0)
        print_help();
    else
    {
        printf("Can not find given command!");
        return 99;
    }

    return 1;
}

void run_commander()
{
    // Command buffer
    char **arg;
    char *command, buffer[BUFFERSIZE];
    int pos, input_size, begin_pos, nr_arg;
    printf("*** Enter a command:\t(eg. 'help') ***\n ");

    while( fgets(buffer, BUFFERSIZE , stdin) )
    {
        pos = 0, nr_arg = 0;

        if(buffer[0] == '\n' || buffer[0] == ' ')
        {
            printf("\n$ ");
            continue;
        }


        // Parsing command
        for (input_size=0; buffer[input_size] != '\0'; ++input_size);
        while (buffer[pos] != ' ' && buffer[pos] != '\n' && pos < input_size)
            pos++;

        command = calloc(pos, sizeof(char));
        begin_pos = pos;

        for (int i = 0; i < pos; ++i)
            command[i] = buffer[i];


        // Parsing arg
        while (pos < input_size)
        {
            if(buffer[pos] == '\n')
                break;
            else if(buffer[pos] == ' ' )
                nr_arg++;

            pos++;
        }

        if(nr_arg)
        {
            arg = realloc(arg, pos * sizeof(char*));
            begin_pos++;
            pos = begin_pos;
            nr_arg = 0;

            while (pos <= input_size)
            {
                if(buffer[pos] == '\n' || pos == input_size)
                {
                    arg[nr_arg] = parse_arg(buffer, begin_pos, pos);
                    nr_arg++;
                    break;
                }
                else if(buffer[pos] == ' ')
                {
                    arg[nr_arg] = parse_arg(buffer, begin_pos, pos);
                    nr_arg++;
                    begin_pos = pos+1;
                }

                pos++;
            }
        }


        // Executing command
        int result = execute(command, arg, nr_arg);

        if(result == 1)
            printf("\t%s : done", command);
        else if(result == 0)
            printf("\tWrong number of arguments: %d", nr_arg);
        else if(result == -1)
        {
            free(arg);
            free(command);
            break;
        }

        free(command);
        printf("\n$ ");
    }
}
