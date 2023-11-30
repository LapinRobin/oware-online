#ifndef CLIENT_H
#define CLIENT_H

#include "server2.h"

typedef struct Client Client;

typedef enum {
    IDLE,
    CHALLENGE,
    WAITING,
    CHOICE,
    PLAYER1,
    PLAYER2,
    INITSTANDBY,
    STANDBY,
    BIO,
    DM,
    ADDFRIEND,
    WAITINGFRIEND,
    FRIEND,
    REMOVEFRIEND,
    HISTORY,
    VIEWGAME
} ClientState;

struct Client {
    SOCKET sock;
    char name[NAME_SIZE+1];
    ClientState state;
    Client *opponent;
    int score;
    int currentGame;
    int observeGame;
    char bio[BUF_SIZE];
    char friend[MAX_CLIENTS][NAME_SIZE+1];
    int number_friend;
    Client *friend_request;
    int history[MAX_GAMES];
    int number_game;
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
    int privacy_mode;
} AwaleGame;

#endif /* guard */
