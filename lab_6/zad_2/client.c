#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <signal.h>
#include <mqueue.h>
#include <time.h>

#include "initial_data.h"

char queue_name[NAME_LEN];

mqd_t mqdesc;
mqd_t server_mqdesc;
int client_id;


void stop() {
    char* msg = (char*)calloc(MAX_MSG_LEN, sizeof(char));
    msg[0] = client_id;

    if(mq_send(server_mqdesc, msg, MAX_MSG_LEN, STOP) < 0) {
        perror("Customer can't send message.\n");
        exit(1);
    }
    if(mq_close(server_mqdesc) < 0) {
        perror("Customer can'r close queue.\n");
        exit(1);
    }
    exit(0);
}


void quit(int signum) {
    stop();
}


void open_chat(int other_id, mqd_t other_mqdesc) {
    char* cmd = NULL;
    size_t len = 0;
    ssize_t read = 0;
    char* msg = (char*)calloc(MAX_MSG_LEN, sizeof(char));
    while(1) {
        printf("Enter message or DISCONNECT: ");
        read = getline(&cmd, &len, stdin);
        cmd[read - 1] = '\0';

        struct timespec* t_spec = (struct timespec*)malloc(sizeof(struct timespec));
        unsigned int type;
        int disconnect = 0;
        while(mq_timedreceive(mqdesc, msg, MAX_MSG_LEN, &type, t_spec) >= 0) {
            if(type == STOP) {
                printf("Server STOPPed this user...\n");
                stop();
            } else if(type == DISCONNECT) {
                printf("Ending...\n");
                disconnect = 1;
                break;
            } else {
                printf("From %d: %s\n", other_id, msg);
            }
        }

        if(disconnect) break;

        if(strcmp(cmd, "DISCONNECT") == 0) {
            msg[0] = client_id;
            msg[1] = other_id;
            if(mq_send(server_mqdesc, msg, MAX_MSG_LEN, DISCONNECT) < 0) {
                perror("Customer can't send message.\n");
                exit(1);
            }
            break;
        } else if(strcmp(cmd, "") != 0) {
            strcpy(msg, cmd);
            if(mq_send(other_mqdesc, msg, MAX_MSG_LEN, CONNECT) < 0) {
                perror("Customer can't send message.\n");
                exit(1);
            }
        }
    }
}


int init() {
    char* msg = (char*)calloc(MAX_MSG_LEN, sizeof(char));
    strcpy(msg, queue_name);

    if(mq_send(server_mqdesc, msg, MAX_MSG_LEN, INIT) < 0) {
        perror("Customer can't send message.\n");
        exit(1);
    }

    unsigned int client_id;
    if(mq_receive(mqdesc, msg, MAX_MSG_LEN, &client_id) < 0) {
        perror("Customer can't receive message.\n");
        exit(1);
    }
    printf("ID received %d\n", client_id);

    return client_id;
}

void list() {
    char* msg = (char*)calloc(MAX_MSG_LEN, sizeof(char));
    msg[0] = client_id;

    if(mq_send(server_mqdesc, msg, MAX_MSG_LEN, LIST) < 0) {
        perror("Customer can't send message.\n");
        exit(1);
    }

    if(mq_receive(mqdesc, msg, MAX_MSG_LEN, NULL) < 0) {
        perror("Customer can't receive message.\n");
        exit(1);
    }

    printf("%s\n", msg);
}

void connect(int id) {
    char* msg = (char*)calloc(MAX_MSG_LEN, sizeof(char));
    msg[0] = client_id;
    msg[1] = id;

    if(mq_send(server_mqdesc, msg, MAX_MSG_LEN, CONNECT) < 0) {
        perror("Customer can't send message.\n");
        exit(1);
    }

    if(mq_receive(mqdesc, msg, MAX_MSG_LEN, NULL) < 0) {
        perror("Customer can't receive message.\n");
        exit(1);
    }

    char* other_queue_name = (char*)calloc(NAME_LEN, sizeof(char));
    strncpy(other_queue_name, msg + 1, strlen(msg) - 1);
    mqd_t other_mqdesc = mq_open(other_queue_name, O_RDWR);


    open_chat(id, other_mqdesc);
}

int synchronize() {
    char* msg = calloc(MAX_MSG_LEN, sizeof(char*));

    struct timespec* t_spec = (struct timespec*)malloc(sizeof(struct timespec));
    unsigned int type;

    if(mq_timedreceive(mqdesc, msg, MAX_MSG_LEN, &type, t_spec) >= 0) {
        if(type == STOP) {
            printf("Server STOPPED this user...\n");
            stop();
            return 0;
        } else if(type == CONNECT) {
            char* recipient_qname = calloc(NAME_LEN, sizeof(char *));
            printf("Connection initiated with %s...\n", recipient_qname);

            strncpy(recipient_qname, msg + 1, strlen(msg) - 1);
            printf("Recipient name %s\n", recipient_qname);
            mqd_t other_mqdesc = mq_open(recipient_qname, O_RDWR);

            open_chat((int) msg[0] - 1, other_mqdesc);
        }
    }
    return 1;
}

mqd_t create_queue(const char* name) {
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_CLIENTS;
    attr.mq_msgsize = MAX_MSG_LEN;
    attr.mq_curmsgs = 0;

    return mq_open(name, O_RDWR | O_CREAT, 0666, &attr);
}

char generate_char() {
    if (rand() % 2 == 0)
        return rand() % ('Z' - 'A' + 1) + 'A';
    else
        return rand() % ('z' - 'a' + 1) + 'a';
}
void generate_name() {
    queue_name[0] = '/';
    for(int i = 1; i < NAME_LEN; i++) queue_name[i] = generate_char();
}

void print_help() {
    printf("Available commands:\n"
           "\tLIST    | l\n"
           "\tCONNECT | c\n"
           "\tSTOP    | s\n"
           "\tHELP    | h\n");
}


void init_customer() {
    srand(time(NULL));
    generate_name();
    printf("Posix queue name %s\n", queue_name);
    print_help();

    mqdesc = create_queue(queue_name);
    server_mqdesc = mq_open(SERVER_QUEUE_NAME, O_RDWR, 0666, NULL);
    client_id = init();
    signal(SIGINT, quit);
}

int parse_comm(char *input) {
    if(strcmp(&input[0], " ")==0 || strcmp(input, "")==0)
        return 1;

    char* command = strtok(input, " ");

    if(strcmp(command, "LIST")==0 || strcmp(command, "l")==0) {
        list();
    } else if(strcmp(command, "CONNECT")==0 || strcmp(command, "c")==0) {
        command = strtok(NULL, " ");
        printf("Connecting\n");
        connect(atoi(command));
    } else if(strcmp(command, "STOP")==0 || strcmp(command, "s")==0) {
        stop();
        return 0;
    } else if(strcmp(command, "HELP")==0 || strcmp(command, "h")==0) {
        print_help();
    } else {
        printf("Bad command structure: %s\n", input);
    }

    return 1;
}

int main() {
    init_customer();

    while(1) {
        printf("Enter command: ");

        char* input;
        size_t len = 0;
        ssize_t read;

        read = getline(&input, &len, stdin);
        input[read - 1] = '\0';

        if (synchronize() == 0)
            break;

        if (parse_comm(input) == 0)
            break;
    }
}