#ifndef CONFIG_H
#define CONFIG_H

#include <errno.h>

#define SERVER_QUEUE_NAME "/my_server"
#define MAX_MSG_LEN 100
#define MAX_CLIENTS 10
#define NAME_LEN 20

typedef enum msg_type {
    STOP        = 1,
    DISCONNECT  = 2,
    LIST        = 3,
    CONNECT     = 4,
    INIT        = 5
} msg_type;

#endif //CONFIG_H