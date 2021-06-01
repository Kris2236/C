#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <pwd.h>
#include "initial_data.h"

key_t qkey;
int queue_id, server_queue_id, customer_id;

void stop() {
    printf("Stopping...\n");
    struct msg_buff* msg = malloc(sizeof(struct msg_buff));

    /* Send message stop to server and confirm finishing state */
    msg->msg_type = STOP;
    msg->customer.customer_id = customer_id;
    msgsnd(server_queue_id, msg, MSG_SIZE, 0);

    /* Delete message queue */
    msgctl(queue_id, IPC_RMID, NULL);
    msgctl(queue_id, IPC_RMID, NULL);

    exit(0);
}

void stop_handler(int signum) {
    stop();
}

void open_chat(int recipient_qid, int recipient_id) {
    struct msg_buff* msg = malloc(sizeof(struct msg_buff));
    char* input;
    int flag = IPC_NOWAIT;

    size_t len = 0;
    ssize_t read;
    printf("If you want finish chat type: END or DISCONNECT.\n");

    while(1) {
        printf("Type message: ");
        read = getline(&input, &len, stdin);
        input[read - 1] = '\0';

        if(msgrcv(queue_id, msg, MSG_SIZE, DISCONNECT, flag) >= 0) {
            printf("End connection...\n");
            break;
        } else if (msgrcv(queue_id, msg, MSG_SIZE, STOP, flag) >= 0) {
            printf("Server STOPPED this user.\n");
            stop();
        }

        while(msgrcv(queue_id, msg, MSG_SIZE, 0, flag) >= 0)
            printf("From %d: %s\n", recipient_id, msg->msg_text);

        if(strcmp(input, "DISCONNECT")==0 || strcmp(input, "END")==0) {
            msg->msg_type = DISCONNECT;
            msg->customer.recipient_id = recipient_id;
            msg->customer.customer_id = customer_id;
            msgsnd(server_queue_id, msg, MSG_SIZE, 0);
            break;
        } else if(strcmp(input, "") != 0) {
            msg->msg_type = CONNECT;
            strcpy(msg->msg_text, input);
            msgsnd(recipient_qid, msg, MSG_SIZE, 0);
        }
    }
}

void connect(int id_to_connect) {
    struct msg_buff* msg = malloc(sizeof(struct msg_buff));

    msg->msg_type = CONNECT;
    msg->customer.customer_id = customer_id;
    msg->customer.recipient_id = id_to_connect;

    msgsnd(server_queue_id, msg, MSG_SIZE, 0);
    struct msg_buff* msg_rcv = malloc(sizeof(struct msg_buff));
    msgrcv(queue_id, msg_rcv, MSG_SIZE, 0, 0);

    key_t other_qkey = msg_rcv->customer.queue_key;
    int other_queue_id = msgget(other_qkey, 0);
    open_chat(other_queue_id, id_to_connect);
}

void list() {
    struct msg_buff* msg = malloc(sizeof(struct msg_buff));
    msg->msg_type = LIST;
    msg->customer.customer_id = customer_id;
    msgsnd(server_queue_id, msg, MSG_SIZE, 0);

    struct msg_buff* msg_rcv = malloc(sizeof(struct msg_buff));
    msgrcv(queue_id, msg_rcv, MSG_SIZE, 0, 0);
    printf("%s\n", msg_rcv->msg_text);
}

int synchronize() {
    struct msg_buff* msg = malloc(sizeof(struct msg_buff));

    if (msgrcv(queue_id, msg, MSG_SIZE, 0, IPC_NOWAIT) >= 0) {
        if(msg->msg_type == STOP) {
            printf("Server STOPPED this user...\n");
            stop();
            return 0;
        } else if(msg->msg_type == CONNECT) {
            int other_queue_id = msgget(msg->customer.queue_key, 0);
            printf("Connection initiated with %d...\n", msg->customer.customer_id);
            open_chat(other_queue_id, msg->customer.customer_id);
            return 1;
        }
    }

    return 1;
}

int init() {
    struct msg_buff* msg = malloc(sizeof(struct msg_buff));
    msg->msg_type = INIT;
    msg->customer.queue_key = qkey;
    msgsnd(server_queue_id, msg, MSG_SIZE, 0);

    struct msg_buff* msg_rcv = malloc(sizeof(struct msg_buff));
    msgrcv(queue_id, msg_rcv, MSG_SIZE, 0, 0);

    int new_customer_id = msg_rcv->customer.customer_id;
    return new_customer_id;
}

void print_help() {
    printf("Available commands:\n"
           "\tLIST    | l\n"
           "\tCONNECT | c\n"
           "\tSTOP    | s\n"
           "\tHELP    | h\n");
}

int random_number() {
    srand(time(NULL));
    return rand() % 255 + 1;
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
    struct passwd *pwd = getpwuid(getuid());
    char *home_dir = pwd->pw_dir;

    qkey = ftok(home_dir, random_number());
    queue_id = msgget(qkey, IPC_CREAT | 0644);
    printf("Client message queue KEY: %d, \tID: %d\n", qkey, queue_id);

    key_t server_key = ftok(home_dir, SERVER_KEY);
    server_queue_id = msgget(server_key, 0);
    printf("Server message queue ID: %d\n", server_queue_id);

    customer_id = init();
    printf("ID received: %d\n", customer_id);
    print_help();

    signal(SIGINT, stop_handler);

    char* input;
    size_t len = 0;
    ssize_t read;

    while(1) {
        printf("Enter command: ");

        /* Read stdin command */
        read = getline(&input, &len, stdin);
        input[read - 1] = '\0';

        if (synchronize() == 0)
            break;

        if (parse_comm(input) == 0)
            break;
    }
}