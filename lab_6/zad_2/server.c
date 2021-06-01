#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <mqueue.h>

#include "initial_data.h"

char* clients_queues[MAX_CLIENTS];
int is_customer_active[MAX_CLIENTS];
mqd_t server_mq;


void close_connection(char *msgp, int i) {
    if(clients_queues[i] != NULL) {
        mqd_t client_mqdesc = mq_open(clients_queues[i], O_RDWR);
        if(client_mqdesc < 0) {
            perror("Server can't open customer's queue.\n");
            exit(1);
        }

        if(mq_send(client_mqdesc, msgp, MAX_MSG_LEN, STOP) < 0) {
            perror("Server can't send message.\n");
            exit(1);
        }

        if(mq_receive(server_mq, msgp, MAX_MSG_LEN, NULL) < 0) {
            perror("Server can't receive message.\n");
            exit(1);
        }

        if(mq_close(client_mqdesc) < 0) {
            perror("Server can't close queue.\n");
            exit(1);
        }
    }
}

void quit_handler(int signum) {
    char* msgp = (char*)calloc(MAX_MSG_LEN, sizeof(char));
    for(int i = 0; i < MAX_CLIENTS; i++) {
        close_connection(msgp, i);
    }

    if (mq_close(server_mq) < 0) {
        perror("Server can't close queue.\n");
        exit(1);
    }

    if (mq_unlink(SERVER_QUEUE_NAME) < 0) {
        perror("Server can't unlink customer's queue.\n");
        exit(1);
    }

    exit(0);
}

void init(char* msg, char* msgp) {
    int min_id = -1;
    printf("Received INIT\n");

    /* Find min available id for customer */
    for(int i=0; i<MAX_CLIENTS; i++) {
        if (is_customer_active[i] == 0) {
            min_id = i;
            break;
        }
    }

    if (min_id == -1) {
        perror("To many customers.\n");
        exit(1);
    }

    mqd_t client_mqdesc = mq_open(msg, O_RDWR);

    is_customer_active[min_id] = 1;
    clients_queues[min_id] = calloc(NAME_LEN, sizeof(char *));
    strcpy(clients_queues[min_id], msg);

    printf("IM here %d, %d\n", client_mqdesc, min_id);

    if(mq_send(client_mqdesc, msgp, MAX_MSG_LEN, min_id) < 0) {
        perror("Server can't send message.\n");
        exit(1);
    }

    if(mq_close(client_mqdesc) < 0) {
        perror("Server can't close queue.\n");
        exit(1);
    }
}

void check(mqd_t client_mqdesc, char *msgp) {
    if(client_mqdesc < 0) {
        perror("Can't receive customer queue.\n");
        exit(1);
    }

    if(mq_send(client_mqdesc, msgp, MAX_MSG_LEN, LIST) < 0) {
        perror("Server can't send message.\n");
        exit(1);
    }

    if(mq_close(client_mqdesc) < 0) {
        perror("Server can't close queue.\n");
        exit(1);
    }
}

void show_list(char* msg, char* msgp) {
    sprintf(msgp + strlen(msgp), "ID | is active\n");

    for(int i = 0; i < MAX_CLIENTS; i++)
        if(clients_queues[i] != NULL) {
            if (is_customer_active[i])
                sprintf(msgp + strlen(msgp), " %d | TRUE\n", i);
            else
                sprintf(msgp + strlen(msgp), " %d | FALSE\n", i);
        }


    int client_id = (int)msg[0];
    mqd_t client_mqdesc = mq_open(clients_queues[client_id], O_RDWR);

    check(client_mqdesc, msgp);
}

void connect(char *msg, char* msgp) {
    int client_id = (int)msg[0];
    int other_client_id = (int)msg[1];

    mqd_t client_mqdesc = mq_open(clients_queues[client_id], O_RDWR);
    msgp[0] = (char) (other_client_id + 1);
    strcat(msgp, clients_queues[other_client_id]);

    if(mq_send(client_mqdesc, msgp, MAX_MSG_LEN, CONNECT) < 0) {
        perror("Server can't send message.\n");
        exit(1);
    }

    memset(msgp, 0, strlen(msgp));
    client_mqdesc = mq_open(clients_queues[other_client_id], O_RDWR);
    msgp[0] = (char) (client_id + 1);
    strcat(msgp, clients_queues[client_id]);

    if(mq_send(client_mqdesc, msgp, MAX_MSG_LEN, CONNECT) < 0) {
        perror("Server can't send message.\n");
        exit(1);
    }

    if(mq_close(client_mqdesc) < 0) {
        perror("Server can't close queue.\n");
        exit(1);
    }

    is_customer_active[client_id] = 0;
    is_customer_active[other_client_id] = 0;
}

void check_customer(mqd_t client_mqdesc, char *msgp) {
    if(client_mqdesc < 0) {
        perror("Server can't open customer's queue.\n");
        exit(1);
    }

    if(mq_send(client_mqdesc, msgp, MAX_MSG_LEN, DISCONNECT) < 0) {
        perror("Server can't send message.\n");
        exit(1);
    }

    if(mq_close(client_mqdesc) < 0) {
        perror("Server can't close customer's queue.\n");
        exit(1);
    }
}

void disconnect(char* msg, char* msgp) {
    int client_id = (int)msg[0];
    int other_client_id = (int)msg[1];

    mqd_t client_mqdesc = mq_open(clients_queues[other_client_id], O_RDWR);
    check_customer(client_mqdesc, msgp);

    is_customer_active[client_id] = 1;
    is_customer_active[other_client_id] = 1;
}

void stop(char* msg) {
    printf("Received: STOP\n");
    int client_id = (int)msg[0];

    is_customer_active[client_id] = 0;
    clients_queues[client_id] = NULL;
}

mqd_t create_queue(const char* name) {
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_CLIENTS;
    attr.mq_msgsize = MAX_MSG_LEN;
    attr.mq_curmsgs = 0;

    return mq_open(name, O_RDWR | O_CREAT, 0666, &attr);
}

void init_server() {
    /* Initial settings */
    printf("Server is starting.\n");
    for(int i = 0; i < MAX_CLIENTS; i++)
        clients_queues[i] = NULL;

    server_mq = create_queue(SERVER_QUEUE_NAME);
    signal(SIGINT, quit_handler);
}

int main() {
    init_server();

    char* msg = calloc(MAX_MSG_LEN, sizeof(char *));
    unsigned int prio;

    while(1) {
        if(mq_receive(server_mq, msg, MAX_MSG_LEN, &prio) < 0) {
            perror("Server can't receive message.\n");
            exit(1);
        }

        char* msgp = (char*)calloc(MAX_MSG_LEN, sizeof(char));

        switch(prio) {
            case LIST:
                show_list(msg, msgp);
                break;
            case CONNECT:
                connect(msg, msgp);
                break;
            case STOP:
                stop(msg);
                break;
            case DISCONNECT:
                disconnect(msg, msgp);
                break;
            case INIT: ;
                init(msg, msgp);
                break;
            default:
                printf("Server received wrong message.\n");
        }
    }
}
