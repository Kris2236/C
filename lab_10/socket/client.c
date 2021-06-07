#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdint.h>
#include <inttypes.h>
#include <errno.h>
#include<signal.h>
#include <pthread.h>

#define MAX 80
#define SA struct sockaddr


struct Client{
    int sock_fd;
    int player;
    uint16_t port;
    char *conn_type;
    char *server_adr;
    char mark;
    char *name;
} client;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
char square[10] = { 'o', '1', '2', '3', '4', '5', '6', '7', '8', '9' };


int check_win()
{
    if (square[1] == square[2] && square[2] == square[3])
        return 1;
    else if (square[4] == square[5] && square[5] == square[6])
        return 1;
    else if (square[7] == square[8] && square[8] == square[9])
        return 1;
    else if (square[1] == square[4] && square[4] == square[7])
        return 1;
    else if (square[2] == square[5] && square[5] == square[8])
        return 1;
    else if (square[3] == square[6] && square[6] == square[9])
        return 1;
    else if (square[1] == square[5] && square[5] == square[9])
        return 1;
    else if (square[3] == square[5] && square[5] == square[7])
        return 1;
    else if (square[1] != '1' && square[2] != '2' && square[3] != '3' &&
             square[4] != '4' && square[5] != '5' && square[6] != '6' &&
             square[7] != '7' && square[8] != '8' && square[9] != '9')
        return 0;
    else
        return  - 1;
}

void board()
{
    printf("\n\n\tTic Tac Toe\n\n");

    printf("     |     |     \n");
    printf("  %c  |  %c  |  %c \n", square[1], square[2], square[3]);
    printf("_____|_____|_____\n");
    printf("     |     |     \n");
    printf("  %c  |  %c  |  %c \n", square[4], square[5], square[6]);
    printf("_____|_____|_____\n");
    printf("     |     |     \n");
    printf("  %c  |  %c  |  %c \n", square[7], square[8], square[9]);
    printf("     |     |     \n\n");
}

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

void check_ping(char *buff)
{
    if (strcmp(buff, "exit") == 0)
    {
        kill(getpid(), SIGINT);
        exit(0);
    }
}

void chat_with_server()
{
    char buff[MAX];

    // initial message - send name to server
    write(client.sock_fd, client.name, strlen(client.name)* sizeof(char));
    printf("[From Server]: ");

    for(int i=0; i<2; ++i)
    {
        bzero(buff, sizeof(buff));
        read(client.sock_fd, buff, sizeof(buff));
        check_ping(buff);
        printf(" %s", buff);
        if (strncmp("User name is used.", buff, sizeof(buff)) == 0)
            exit(-1);
    }

    bzero(buff, sizeof(buff));
    read(client.sock_fd, buff, sizeof(buff));
    check_ping(buff);
    client.mark = buff[0];
    printf("\n=============\nGAME STARTED\nYOU ARE: \"%c\"\n=============\n", client.mark);

    // Game engine
    int player, i, choice, invalid;
    char mark;

    if (client.mark == 'X')
    {
        printf("Your turn\n");
        player = 1;
        client.player = 1;
    }
    else
    {
        printf("Wait for opponent\n");
        player = 2;
        client.player = 2;
    }

    do
    {
        invalid = 0;
        player = (player % 2) ? 1 : 2;
        printf("\n\n*** Player %d ***\n", player);

        if (player == 1)
        {
            board();
            mark = client.mark;

            // set to opponent your move
            printf("[%s]: enter a number: ", client.name);
            bzero(buff, sizeof(buff));
            buff[0] = getchar();
            choice = str_to_int(buff);
        }
        else
        {
            if (client.mark == 'X')
                mark = 'O';
            else
                mark = 'X';

            bzero(buff, sizeof(buff));
            read(client.sock_fd, buff, sizeof(buff));
            printf("[From Server]: %s\n", buff);
            check_ping(buff);
            choice = str_to_int(buff);
        }

        if (choice == 1 && square[1] == '1')
            square[1] = mark;
        else if (choice == 2 && square[2] == '2')
            square[2] = mark;
        else if (choice == 3 && square[3] == '3')
            square[3] = mark;
        else if (choice == 4 && square[4] == '4')
            square[4] = mark;
        else if (choice == 5 && square[5] == '5')
            square[5] = mark;
        else if (choice == 6 && square[6] == '6')
            square[6] = mark;
        else if (choice == 7 && square[7] == '7')
            square[7] = mark;
        else if (choice == 8 && square[8] == '8')
            square[8] = mark;
        else if (choice == 9 && square[9] == '9')
            square[9] = mark;

        else
        {
            printf("Invalid move (press enter to continue)");
            player--;
            invalid = 1;
        }

        if (player==1 && invalid==0)
        {
            board();
            write(client.sock_fd, buff, sizeof(buff));
        }

        i = check_win();
        player++;
    } while (i == -1);


    board();
    player--;

    if (client.mark == 'X')
    {
        if (player == client.player)
            printf("You won");
        else
            printf("You lose");
    }
    else
    {
        if (player == client.player)
            printf("You lose");
        else
            printf("You won");
    }

    getchar();
}

uint16_t str_to_uint16(const char *str)
{
    char *end;
    errno = 0;
    intmax_t val = strtoimax(str, &end, 10);
    if (errno == ERANGE || val < 0 || val > UINT16_MAX || end == str || *end != '\0')
    {
        perror("Failed cast port to uint16_t");
        exit(2);
    }

    return (uint16_t) val;
}

void parse_arg(int argc, char* argv[])
{
    if (argc != 5)
    {
        perror("Wrong number of arguments. Expected user_name, connection_type (INET/UNIX), server_adr, port\n");
        exit(-1);
    }

    client.name = calloc(strlen(argv[1]), sizeof(char));
    client.name = argv[1];

    client.conn_type = calloc(strlen(argv[2]), sizeof(char));
    client.conn_type = argv[2];

    client.server_adr = calloc(strlen(argv[3]), sizeof(char));
    client.server_adr = argv[3];

    client.port = str_to_uint16(argv[4]);
}

void clean()
{
    shutdown(client.sock_fd, SHUT_RDWR);
    close(client.sock_fd);
}

void sig_handler(int signum)
{
    pthread_mutex_lock(&mutex);
    printf("\nEnding procedure\n");
    write(client.sock_fd, "exit", sizeof(char)*4);

    char buff[MAX];
    read(client.sock_fd, buff, sizeof(buff));

    if (strcmp(buff, "ok") != 0)
        perror("Disconnecting went wrong.");
    printf("Disconnected");

    clean();

    kill(getpid(), signum);
    signal(SIGINT,SIG_DFL);
    pthread_mutex_unlock(&mutex);
}


int main(int argc, char *argv[])
{
    struct sockaddr_in serv_addr;
    parse_arg(argc, argv);

    signal(SIGINT,sig_handler);

    if (strcmp(client.conn_type, "INET") == 0)
    {
        // socket create and verification
        client.sock_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (client.sock_fd == -1)
        {
            printf("socket creation failed...\n");
            exit(0);
        }
        printf("Socket successfully created..\n");

        bzero(&serv_addr, sizeof(serv_addr));
        printf("%s %d", client.server_adr, client.port);

        // assign IP, PORT
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = inet_addr(client.server_adr);
        serv_addr.sin_port = htons(client.port);

        // connect the client socket to server socket
        if (connect(client.sock_fd, (SA*)&serv_addr, sizeof(serv_addr)) != 0)
        {
            printf("connection with the server failed...\n");
            exit(0);
        }
        printf("connected to the server..\n");

        chat_with_server();

        clean();
    }
    else if (strcmp(client.conn_type, "UNIX") == 0)
    {
        printf("UNIX...");
        chat_with_server();
        clean();
    }
    else
    {
        perror("Wrong connection type.");
    }
}
