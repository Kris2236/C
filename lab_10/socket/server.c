#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>
#include <stdint-gcc.h>
#include <inttypes.h>
#include <sys/un.h>
#include <strings.h>
#include <signal.h>

#define MAX 80
#define SA struct sockaddr
#define MAX_CON 10

enum Status{
    INACTIVE = 0,
    INIT = 1,
    WAITING = 2,
    CONNECTED = 3
};

struct Client{
    char *name;
    enum Status status;
    int conn_fd;
    int connected_with_id;
} client[MAX_CON];

struct Server{
    uint16_t port;
    uint16_t ping_port;
    char *conn_type;
    char *path;
} server;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
socklen_t address_len;
struct sockaddr_in serv_addr, client_addr;
int sock_fd;
int active_users;



/****************
 * General functions
 ***************/
void sig_handler(int signum)
{
    if (active_users == 0)
    {
        printf("Active users %d\nFinishing...\n", active_users);
        kill(getpid(), signum);
        exit(0);
    }
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
    if (argc != 3)
    {
        perror("Wrong number of arguments. Expected connection_type (INET/UNIX), port/(path if UNIX)\n");
        exit(-1);
    }

    server.conn_type = calloc(strlen(argv[1]), sizeof(char));
    server.conn_type = argv[1];

    if (strcmp(server.conn_type, "UNIX") == 0)
    {
        server.path = calloc(strlen(argv[2]), sizeof(char));
        server.path = argv[2];
    }
    else
    {
        server.port = str_to_uint16(argv[2]);
        server.ping_port = server.port + str_to_uint16("1");
    }
}

int unique_name(char *str)
{
    for(int i=0; i<MAX_CON; ++i)
    {
        if (client[i].name == NULL)
            continue;

        if (strcmp(str, client[i].name) == 0)
            return 0;
    }
    return 1;
}

void clear_client(int id)
{
    client[id].name = NULL;
    client[id].status = INACTIVE;
    client[id].conn_fd = -1;
    client[id].connected_with_id = -1;
}

void client_init()
{
    for(int i=0; i<MAX_CON; i++)
        clear_client(i);
}

void send_to(struct Client *client_x, char *message)
{
    if (write(client_x->conn_fd, message, strlen(message)) == -1)     // send that buffer to client_
    {
//        printf("Write failed...\n");
    }

    pthread_mutex_lock(&mutex);
    if (strncmp("exit", message, 4) == 0)       // if msg contains "Exit" then server exit and chat_with_user ended.
    {
        printf("Disconnecting user %s...\n", client_x->name);
        int opponent_id = client_x->connected_with_id;
        if (opponent_id != -1)
        {
            printf("Disconnecting opponent %s...\n", client[opponent_id].name);
            write(client[opponent_id].conn_fd, "exit", sizeof(char)*4);
            usleep(1000);
            clear_client(opponent_id);
            active_users--;
            raise(SIGINT);
        }

        write(client_x->conn_fd, "exit", sizeof(char)*4);
        client_x->status = INACTIVE;
        client_x->name = NULL;
        client_x->connected_with_id = -1;
        client_x->conn_fd = -1;
        active_users--;
        raise(SIGINT);
    }
    pthread_mutex_unlock(&mutex);
}

char *receive_from(struct Client *client_x)
{
    char *buff = calloc(MAX, sizeof(char));
    bzero(buff, MAX);

    pthread_mutex_lock(&mutex);
    // read the message from client and copy it in buffer
    if (read(client_x->conn_fd, buff, sizeof(char)*MAX) == 0)
    {
        printf("Read failed...\n");
        return 0;
    }
    if (strlen(buff) > 0)
        printf("[%s]: %s", client_x->name, buff);
    pthread_mutex_unlock(&mutex);

    return buff;
}

void set_name(struct Client *client_x)
{
    char *buff = receive_from(client_x);

    if (client_x->name != NULL)
        printf("User have name %s. Resetting user...\n", client_x->name);

    if (unique_name(buff))
    {
        client_x->name = malloc(sizeof(char) * (strlen(buff) + 1));
        strncpy(client_x->name, buff, strlen(buff));
        bzero(buff, MAX);
        strcpy(buff, "Waiting for opponent.\n");
    }
    else
    {
        printf("User name is used.");
        bzero(buff, MAX);
        strcpy(buff, "User name is used.");
    }

//    send_to(client_x, buff);
    client_x->status = WAITING;
}

void chat_with_user(struct Client *client_x)
{
    char *buff = receive_from(client_x);
//    printf(" -> [%s]\n", client[client_x->connected_with_id].name);
    send_to(&client[client_x->connected_with_id], buff);
}

int set_nonblock(int sock) {
    int flags;
    flags = fcntl(sock, F_GETFL, 0);
    if (-1 == flags)
        return -1;
    return fcntl(sock, F_SETFL, flags | O_NONBLOCK);
}

void user_list()
{
//    printf("[Client list]\n");
//    printf("[id] [name] [status] [connected_with]\n");
//    for(int i=0; i<MAX_CON; ++i)
//        printf("%d\t%s\t%d\t%d\n", i, client[i].name, client[i].status, client[i].connected_with_id);
}

int find_free_id()
{
    user_list();
    for(int i=0; i<MAX_CON; ++i)
        if (client[i].status == INACTIVE)
            return i;

    return -1;
}

int find_opponent(int id)
{
    user_list();
    for(int i=0; i<MAX_CON; ++i)
    {
        if (client[i].status==WAITING && id!=i && client[i].connected_with_id==-1)
            return i;
    }

    return -1;
}

int generate_rand(int min, int max)
{
    time_t t;
    srand((unsigned) time(&t));

    return (rand() % (max-min + 1) + min);
}

void init_opponent(int self_id, int opponent_id, int is_X)
{
    int t_sleep = 1000;
    client[self_id].connected_with_id = opponent_id;

    send_to(&client[self_id], "Found opponent: ");
    usleep(t_sleep);
    send_to(&client[self_id], client[opponent_id].name);
    usleep(t_sleep);

    if (is_X)
        send_to(&client[self_id], "X");
    else
        send_to(&client[self_id], "O");

    usleep(t_sleep);
    client[self_id].status = CONNECTED;
}

void pair(int i)
{
    int opponent_id = find_opponent(i);

    if (opponent_id != -1)
    {
        printf("Found pair [%d] %s <-> [%d] %s\n", i, client[i].name, opponent_id, client[opponent_id].name);
        int is_X = generate_rand(0,1);
        init_opponent(i, opponent_id, is_X);
        init_opponent(opponent_id, i, !is_X);
    }
}

void clean(pthread_t tid)
{
    if (pthread_cancel(tid) != 0)       // Terminate threads
    {
        perror("Can not join santa thread.\n");
        exit(EXIT_FAILURE);
    }

    shutdown(sock_fd, SHUT_RDWR);
    close(sock_fd);     // Close the socket

    for(int i=0; i<MAX_CON; ++i)
        free(client[i].name);
}



/****************
 * INET connection functions
 ***************/
int create_socket_inet()
{
    int socked_fd = socket(AF_INET, SOCK_STREAM, 0);      // socket create and verification
    if (socked_fd == -1)
    {
        printf("socket creation failed...\n");
        exit(1);
    }

    return socked_fd;
}

int get_connection_adr(int thread_sock_fd)
{
    // Accept the data packet from client and verification
    int conn_fd = accept(thread_sock_fd, (SA*)&client_addr, &address_len);
    if (conn_fd < 0)
        return -1;

    return conn_fd;
}

void *accept_user(void * arg)
{
    int *tmp = (int *) arg;
    int thread_sock_fd = *tmp;
    int tmp_ard;
    int id;

    while(thread_sock_fd != -1)
    {
        if ((tmp_ard = get_connection_adr(thread_sock_fd)) == -1)
        {
            sleep(4);
            continue;
        }

        while((id = find_free_id()) == -1)
        {
            printf("There is to many users, active users %d\n", active_users);
            sleep(5);
        }

        pthread_mutex_lock(&mutex);

        if (set_nonblock(tmp_ard) == -1)
        {
            perror("Failed setting nonblock socket");
            exit(1);
        }

        client[id].conn_fd = tmp_ard;
        client[id].status = INIT;
        active_users++;
        printf("Accepted cilent %d, active users %d\n", id, active_users);

        pthread_mutex_unlock(&mutex);
    }

    free(arg);
    return 0;
}

void create_thread(pthread_t *tid, int arg)
{
    int *id = malloc(sizeof(int*));
    *id = arg;

    /* Create thread pinging users .*/
    if (pthread_create(tid, NULL, accept_user, (void *) id) != 0)
    {
        perror("Could not create thread.\n");
        exit(EXIT_FAILURE);
    }
}



/****************
 * UNIX connection functions
 ***************/
int uname(int sock, char *name) {
    struct sockaddr_un serveraddr;

    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sun_family = AF_UNIX;
    strcpy(serveraddr.sun_path, name);

    // give socket unique name
    return bind(sock, (struct sockaddr *)&serveraddr, SUN_LEN(&serveraddr));
}


int main(int argc, char *argv[])
{
    parse_arg(argc, argv);
    signal(SIGINT,sig_handler);


    if (strcmp(server.conn_type, "INET") == 0)
    {
        sock_fd = create_socket_inet();

        pthread_t tid_accept_client, tid_ping;
        client_init();

        bzero(&serv_addr, sizeof(struct sockaddr_in));      // set with zeros
        // assign IP, PORT
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        serv_addr.sin_port = htons(server.port);

        // Binding newly created socket to given IP and verification
        if ((bind(sock_fd, (SA*)&serv_addr, sizeof(serv_addr))) != 0)
        {
            printf("socket bind failed...\n");
            exit(0);
        }
        printf("Socket successfully binded..\n");

        // Now server is ready to listen and verification
        if ((listen(sock_fd, MAX_CON)) != 0)
        {
            printf("Listen failed...\n");
            exit(0);
        }
        printf("Server listening..\n");
        address_len = sizeof(client_addr);

        create_thread(&tid_accept_client, sock_fd);

        int timeout = 30;   // after this time without users, server exit
        int noactive_users;
        while(active_users > 0 || timeout > 0)
        {
            noactive_users = 0;
            for (int i=0; i<MAX_CON; ++i)
            {
                if (client[i].status == INACTIVE)  // skip inactive
                {
                    noactive_users++;
                    continue;
                }

                if (client[i].status == INIT)  // initial settings
                    set_name(&client[i]);

                if (client[i].status == WAITING && client[i].connected_with_id == -1)  // try find opponent
                    pair(i);

                if (client[i].status == CONNECTED)  // switch message if there is a pair
                    chat_with_user(&client[i]);
            }

            sleep(1);
            if (active_users == 0 || noactive_users==MAX_CON)
                timeout--;
        }

        clean(tid_accept_client);
//        clean(tid_ping);
    }
    else if (strcmp(server.conn_type, "UNIX") == 0)
    {
        int s = socket(AF_UNIX, SOCK_STREAM, 0);

        // set socket name and address
        if (uname(s, server.path) || listen(s, 1)) return -1;

        // accept client
//        int fd = accept(s, NULL, 0);
    }
    else
    {
        perror("Wrong connection type");
    }
}
