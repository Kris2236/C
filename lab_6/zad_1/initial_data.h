#ifndef CONFIG_H
#define CONFIG_H

#include <sys/types.h>
#include <sys/ipc.h>

#define MAX_CLIENTS 5

typedef enum customer_status {
    FREE        = -1,
    BUSY        = 0,
    AVAILABLE   = 1
} customer_status;

typedef enum msg_type {
    STOP        = 1,
    DISCONNECT  = 2,
    LIST        = 3,
    CONNECT     = 4,
    INIT        = 5
} msg_type;

typedef struct customer {
    key_t   queue_key;
    int     customer_id;
    int     recipient_id;
} customer;

typedef struct msg_buff {
    long        msg_type;
    char        msg_text[1024];
    /* time_t mssg_time; */
    customer    customer;
} msg_buff;


const int SERVER_KEY = 2236;
const int MSG_SIZE = sizeof(msg_buff) - sizeof(long);

#endif //CONFIG_H