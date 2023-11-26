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
    int id;
    int score;
};

typedef struct {
    int board[NB_HOUSES_TOTAL];
    int score[2];
    int currentPlayer;
    int position;
    Client *player1;
    Client *player2;
    char status[BUF_SIZE];
} AwaleGame;

#endif /* guard */
