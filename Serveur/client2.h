#ifndef CLIENT_H
#define CLIENT_H

#include "server2.h"

typedef struct Client Client;

typedef enum {
    IDLE,
    CHALLENGE,
    CHOICE,
    BUSY
} ClientState;

struct Client {
    SOCKET sock;
    char name[BUF_SIZE];
    ClientState state;
    Client *opponent;

};

#endif /* guard */
