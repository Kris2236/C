#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include "initial_data.h"

key_t customers_queues[MAX_CLIENTS];
int is_customer_active[MAX_CLIENTS];
int server_queue;

void quit(int signum) {
    msg_buff* msgp = malloc(sizeof(msg_buff));

    /* Send stop message for all active customers */
    for(int i = 0; i < MAX_CLIENTS; i++) {
        key_t server_qkey = customers_queues[i];

        if(server_qkey != -1) {
            msgp->msg_type = STOP;
            int customer_qid = msgget(server_qkey, 0);
            msgsnd(customer_qid, msgp, MSG_SIZE, 0);
            msgrcv(server_queue, msgp, MSG_SIZE, STOP, 0);
        }
    }

    /* Close server's queue */
    msgctl(server_queue, IPC_RMID, NULL);
    exit(0);
}

void init(msg_buff* msg, msg_buff* msgp) {
    int min_id = -1;
    printf("INIT Received\n");

    /* Find min available id for customer */
    for(int i=0; i<MAX_CLIENTS; i++) {
        if (customers_queues[i] == -1) {
            min_id = i;
            break;
        }
    }

    if (min_id == -1) {
        perror("To many customers.\n");
        exit(1);
    }

    msgp->msg_type = INIT;
    msgp->customer.customer_id = min_id;

    /* Create message queue for customer */
    int customer_qid = msgget(msg->customer.queue_key, 0);
    customers_queues[min_id] = msg->customer.queue_key;
    is_customer_active[min_id] = AVAILABLE;     // Set customer as active in map
    msgsnd(customer_qid, msgp, MSG_SIZE, 0);

}

void show_list(msg_buff* msgp, int customer_id) {
    printf("LIST received\n");
    strcpy(msgp->msg_text, "ID | is active\n");

    for(int i = 0; i < MAX_CLIENTS; i++)
        if(customers_queues[i] != -1)
            sprintf(msgp->msg_text + strlen(msgp->msg_text),
                    " %d | %s\n", i , is_customer_active[i] ? "TRUE" : "FALSE");

    /* Create new queue and reply */
    int customer_qid = msgget(customers_queues[customer_id], 0);

    msgp->msg_type = LIST;
    msgsnd(customer_qid, msgp, MSG_SIZE, 0);
}

void connect(msg_buff* msg, msg_buff* msgp, int requester_id) {
    printf("Received: CONNECT\n");
    int recipient_id = msg->customer.recipient_id;

    /* Set data and create new message for recipient <- requester */
    msgp->msg_type = CONNECT;
    msgp->customer.queue_key = customers_queues[recipient_id];
    int customer_qid = msgget(customers_queues[requester_id], 0);
    msgsnd(customer_qid, msgp, MSG_SIZE, 0);


    /* Set data and create new message for requester <- recipient */
    msgp->msg_type = CONNECT;
    msgp->customer.queue_key = customers_queues[requester_id];
    msgp->customer.customer_id = requester_id;
    customer_qid = msgget(customers_queues[recipient_id], 0);
    msgsnd(customer_qid, msgp, MSG_SIZE, 0);

    /* Set customers as not active in map */
    is_customer_active[requester_id] = BUSY;
    is_customer_active[recipient_id] = BUSY;
}

void disconnect(msg_buff* msg, msg_buff* msgp, int customer_id) {
    printf("Received: DISCONNECT\n");
    int recipient_id = msg->customer.recipient_id;

    msgp->msg_type = DISCONNECT;
    int customer_qid = msgget(customers_queues[recipient_id], 0);
    msgsnd(customer_qid, msgp, MSG_SIZE, 0);

    /* Set customers as available in map */
    is_customer_active[customer_id] = AVAILABLE;
    is_customer_active[recipient_id] = AVAILABLE;
}

void stop(int customer_id) {
    printf("Received: STOP\n");

    /* Set customer id as free in map */
    customers_queues[customer_id] = -1;
    is_customer_active[customer_id] = FREE;
}

int main() {
    /* Initial setting customers map */
    for(int i = 0; i < MAX_CLIENTS; i++) {
//        customers_queues[i] = -1;
        customers_queues[i] = FREE;
    }

    /* Get homedir */
    struct passwd *pwd = getpwuid(getuid());
    char *homedir = pwd->pw_dir;

    /* Create key basing on $HOME dir */
    key_t server_qkey = ftok(homedir, SERVER_KEY);
    server_queue = msgget(server_qkey, 0644 | IPC_CREAT);
    printf("Server message queue KEY: %d, \tID: %d\n", server_qkey, server_queue);

    /* Set interrupt signal handler */
    signal(SIGINT, quit);

    struct msg_buff *msg = malloc(sizeof(struct msg_buff));

    while(1) {
        /* Get message with smallest absolute msg_type - smaller msg_type will be first receive */
        if (msgrcv(server_queue, msg, MSG_SIZE, -10, 0) == -1) {
            perror("Server can't receive message.\n");
            exit(1);
        }

        struct msg_buff* msgp = malloc(sizeof(struct msg_buff));
        int customer_id = msg->customer.customer_id;

        switch(msg->msg_type) {
            case LIST:
                show_list(msgp, customer_id);
                break;
            case CONNECT:
                connect(msg, msgp, customer_id);
                break;
            case STOP:
                stop(customer_id);
                break;
            case DISCONNECT:
                disconnect(msg, msgp, customer_id);
                break;
            case INIT:
                init(msg, msgp);
                break;
            default:
                printf("Server received wrong message.\n");
        }
    }
}