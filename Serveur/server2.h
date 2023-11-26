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
typedef struct {
    int board[NB_HOUSES_TOTAL];
    int score[2];
    int currentPlayer;
    int position;
} AwaleGame;

#else

#error not defined for this platform

#endif

#define CRLF        "\r\n"
#define PORT         1977
#define MAX_CLIENTS     100
#define MAX_GAMES     50

#define BUF_SIZE    1024

#include "client2.h"

static void init(void);
static void end(void);
static void app(void);
static int init_connection(void);
static void end_connection(int sock);
static int read_client(SOCKET sock, char *buffer);
static void write_client(SOCKET sock, const char *buffer);
static void send_message_to_all_clients(Client *clients, Client client, int actual, const char *buffer, char from_server);
static void send_list_of_clients(Client *clients, Client client, int actual, int sender_sock, const char *buffer, int from_server);
static void challenge_another_client_init(Client *clients, Client *client, int actual, int sender_sock, const char *buffer, int from_server);
static void challenge_another_client_request(Client *clients, Client *client, int actual, int sender_sock, const char *buffer, int from_server);
static void challenge_another_client_accept(Client *clients, Client client, int actual, int sender_sock, const char *buffer, int from_server);
static void remove_client(Client *clients, int to_remove, int *actual);
static void handle_disconnect_client(Client client, Client *clients, int i, int actual);
static void clear_clients(Client *clients, int actual);

#endif /* SERVER_H */
