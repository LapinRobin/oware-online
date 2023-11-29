#ifndef CLIENT_H
#define CLIENT_H

#include "server2.h"

typedef struct Client Client;

typedef enum {
    IDLE,
    CHALLENGE,
    CHOICE,
    PLAYER1,
    PLAYER2,
    INITSTANDBY,
    STANDBY,
    BIO,
    DM
} ClientState;

struct Client {
    SOCKET sock;
    char name[BUF_SIZE];
    ClientState state;
    Client *opponent;
    int score;
    int currentGame;
    int observeGame;
    char bio[BUF_SIZE];
};

typedef struct {
    int board[NB_HOUSES_TOTAL];
    int score[2];
    int currentPlayer;
    int position;
    Client *player1;
    Client *player2;
    char status[BUF_SIZE];
    Client *observer[MAX_CLIENTS];
    int number_observer;
} AwaleGame;

#endif /* guard */
