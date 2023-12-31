#ifndef SERVER_H
#define SERVER_H

#ifdef WIN32

#include <winsock2.h>

#elif defined (linux) || defined(__APPLE__)

#define NB_HOUSES_TOTAL 12 /* number of houses on the board */
#define NB_HOUSES_PER 6 /* number of houses per player */
#define NB_SEEDS 4 /* number of seeds per house */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h> /* close */
#include <netdb.h> /* gethostbyname */

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(s) close(s)
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;


#else

#error not defined for this platform

#endif

#define CRLF        "\r\n"
#define PORT         1977
#define MAX_CLIENTS     100
#define MAX_GAMES     50

#define BUF_SIZE    1024
#define NAME_SIZE    26

#define K_FACTOR 32 
#define INITIAL_SCORE 1000

#include "client2.h"

void display_board(AwaleGame *game, int board[], int score[], Client* client);

void distribute_pieces(int board[]);

int is_valid_move(int board[], int player, int position);

void playable_positions(int board[], int player, int positions[]);

void play_move(int board[], int score[], int player, int position);

double calculateExpectedScore(int ratingA, int ratingB);

void updateEloRatings(Client *playerA, Client *playerB, int outcome);

int is_game_over(AwaleGame *game, char status[], int board[], int score[]);

void collect_seeds(int board[], int score[], int currentPlayer);

int is_number(char *str);

void init_game(AwaleGame *game);

void game_play(AwaleGame *game);

void game_over(AwaleGame *game);

void add_observer(AwaleGame *game, Client *observer_client);

void remove_observer(AwaleGame *game, Client *observer_client);

int is_friend(Client *client, Client *friend);

void add_friend(Client *client, Client *friend);

void remove_friend(Client *client, Client *friend);

static void init(void);

static void end(void);

static void app(void);

static int init_connection(void);

static void end_connection(int sock);

static int read_client(SOCKET sock, char *buffer);

static void write_client(SOCKET sock, const char *buffer);

static void handle_new_client(SOCKET sock, Client *clients, int *actual, int *max);

static void handle_server_input(Client *clients, int actual, int sock, char *buffer, AwaleGame games[], int game_index[]);

static void
handle_client_state(Client *clients, Client *client, int *actual, fd_set *rdfs, char *buffer, int i, AwaleGame games[],
                    AwaleGame *game, int game_index[]);


static void
send_message_to_all_clients(Client *clients, Client client, int actual, const char *buffer, char from_server);

static void send_ranking_to_client(Client *clients, Client client, int actual, int sender_sock, const char *buffer,
                       int from_server);

static void
send_list_of_clients(Client *clients, Client client, int actual, int sender_sock, const char *buffer, int from_server, int include_self);

static void
send_list_of_games(AwaleGame games[], int game_index[], Client client, int actual, int sender_sock, const char *buffer, int from_server);

static void
send_list_of_saved_games(AwaleGame games[], int game_index[], Client client, int actual, int sender_sock, const char *buffer, int from_server);

static void
send_list_of_friends(Client *clients, Client client, int actual, int sender_sock, const char *buffer, int from_server);

static void
send_list_of_available_friends(Client *clients, Client client, int actual, int sender_sock, const char *buffer, int from_server);

static void
add_friend_init(Client *clients, Client *client, int actual, int sender_sock, const char *buffer,
                int from_server);
static void
add_friend_request(Client *clients, Client *client, int actual, int sender_sock, const char *buffer,
                                 int from_server);

static void
remove_friend_request(Client *clients, Client *client, int actual, int sender_sock, const char *buffer,
                      int from_server);

static void
challenge_another_client_init(Client *clients, Client *client, int actual, int sender_sock, const char *buffer,
                              int from_server);

static void
challenge_another_client_request(Client *clients, Client *client, int actual, int sender_sock, const char *buffer,
                                 int from_server);


static void remove_client(Client *clients, int to_remove, int *actual);

static void handle_disconnect_client(Client *clients, Client client, int i, int *actual);

static void clear_clients(Client *clients, int actual);

#endif /* SERVER_H */
