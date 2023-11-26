#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "server2.h"
#include "client2.h"


void displayBoard(int board[], int score[]) {
    printf("\nGame Board:\n");
    printf("Player 1 (bottom): ");
    for (int i = 0; i < NB_HOUSES_PER; i++) {
        printf("%d ", board[i]);
    }
    printf("\nscore: %d ", score[0]);
    printf("\nPlayer 2 (top): ");
    for (int i = NB_HOUSES_TOTAL - 1; i >= NB_HOUSES_PER; i--) {
        printf("%d ", board[i]);
    }
    printf("\nscore: %d ", score[1]);
    printf("\n");
}

void distributePieces(int board[]) {
    for (int i = 0; i < NB_HOUSES_TOTAL; i++) {
        board[i] = NB_SEEDS;
    }
}

int isValidMove(int board[], int player, int position) {
    int totalPlayer1 = 0, totalPlayer2 = 0;
    for (int i = 0; i < NB_HOUSES_PER; i++) {
        totalPlayer1 += board[i];
    }
    for (int i = NB_HOUSES_PER; i < NB_HOUSES_TOTAL; i++) {
        totalPlayer2 += board[i];
    }

    if (totalPlayer1 == 0 && player == 2) {
        if (position >= NB_HOUSES_PER && position < NB_HOUSES_TOTAL &&
            board[position] + position >= NB_HOUSES_PER)
            return 1;
        else return 0;
    }

    if (totalPlayer2 == 0 && player == 1) {
        if (position >= 0 && position < NB_HOUSES_PER && board[position] + position >= NB_HOUSES_PER) return 1;
        else return 0;
    }

    if ((player == 1 && position >= 0 && position < NB_HOUSES_PER) ||
        (player == 2 && position >= NB_HOUSES_PER && position < NB_HOUSES_TOTAL)) {
        if (board[position] > 0) {
            return 1;
        }
    }
    return 0;
}

void playablePositions(int board[], int player, int positions[]) {
    int index = 0;
    for (int i = 0; i < NB_HOUSES_TOTAL; i++) {
        if (isValidMove(board, player, i)) {
            positions[index] = i;
            index++;
        }
    }
    positions[index] = -1; // Mark the end of the array
}

// Play a move
void playMove(int board[], int score[], int player, int position) {
    int index = position;
    int pieces = board[position];
    board[position] = 0;

    while (pieces > 0) {
        index = (index + 1) % NB_HOUSES_TOTAL;
        if (index == position) {
            continue;
        }
        board[index]++;
        pieces--;
    }

    while ((player == 1 && index >= NB_HOUSES_PER && index < NB_HOUSES_TOTAL &&
            (board[index] == 2 || board[index] == 3)) ||
           (player == 2 && index >= 0 && index < NB_HOUSES_PER && (board[index] == 2 || board[index] == 3))) {
        score[player - 1] += board[index];
        board[index] = 0;
        index = (index - 1) % NB_HOUSES_TOTAL;
    }
}

int isGameOver(int board[], int score[]) {
    int total = 0;

    if (score[0] > NB_SEEDS * NB_HOUSES_PER || score[1] > NB_SEEDS * NB_HOUSES_PER) return 1;

    for (int i = 0; i < NB_HOUSES_TOTAL; i++) {
        total += board[i];
    }

    if (total <= 6) {
        return 1;
    }

    return 0;
}

void collectSeeds(int board[], int score[], int currentPlayer) {
    int total = 0;
    for (int i = 0; i < NB_HOUSES_TOTAL; i++) {
        total += board[i];
        board[i] = 0;
    }
    score[currentPlayer - 1] += total;
}

void initGame(AwaleGame *game) {
    for (int i = 0; i < NB_HOUSES_TOTAL; i++) {
        game->board[i] = NB_SEEDS;
    }
    game->score[0] = 0;
    game->score[1] = 0;
    game->currentPlayer = 1;
    int possibleCases[NB_HOUSES_TOTAL + 1];

    distributePieces(game->board);

    while (!isGameOver(game->board, game->score)) {
        displayBoard(game->board, game->score);
        playablePositions(game->board, game->currentPlayer, possibleCases);
        if (possibleCases[0] == -1) {
            printf("\nAll remaining pieces are in the same camp and player %d can no longer feed the opponent.\n",
                   game->currentPlayer);
            collectSeeds(game->board, game->score, game->currentPlayer);
        } else {
            printf("Playable positions for player %d:\n", game->currentPlayer);
            for (int i = 0; possibleCases[i] != -1; i++) {
                printf("%d ", possibleCases[i]);
            }
            printf("\n");
            printf("\nPlayer %d, enter the position of the move: ", game->currentPlayer);
            scanf("%d", &game->position);
            if (isValidMove(game->board, game->currentPlayer, game->position)) {
                playMove(game->board, game->score, game->currentPlayer, game->position);
                game->currentPlayer = (game->currentPlayer == 1) ? 2 : 1;
            } else {
                printf("Invalid move. Please choose a valid position.\n");
            }
        }

    }

    printf("\nGame over. Final score:\n");
    displayBoard(game->board, game->score);
}

static void init(void) {
#ifdef WIN32
    WSADATA wsa;
    int err = WSAStartup(MAKEWORD(2, 2), &wsa);
    if (err < 0)
    {
       puts("WSAStartup failed !");
       exit(EXIT_FAILURE);
    }
#endif
}

static void end(void) {
#ifdef WIN32
    WSACleanup();
#endif
}

static void handle_new_client(SOCKET sock, Client *clients, int *actual, int *max) {
    SOCKADDR_IN csin = {0};
    socklen_t sinsize = sizeof(csin);
    char buffer[BUF_SIZE];
    int csock = accept(sock, (SOCKADDR *)&csin, &sinsize);

    if (csock == SOCKET_ERROR) {
        perror("accept()");
        return;
    }

    if (read_client(csock, buffer) == -1) {
        // If reading from client failed, close the socket and return
        closesocket(csock);
        return;
    }

    // Update the maximum file descriptor if necessary
    *max = csock > *max ? csock : *max;

    // Initialize the new client
    Client c = {csock};
    strncpy(c.name, buffer, BUF_SIZE - 1);
    c.name[BUF_SIZE - 1] = '\0'; // Ensure null-termination
    c.state = IDLE;

    // Add the new client to the clients array
    if (*actual < MAX_CLIENTS) {
        clients[*actual] = c;
        (*actual)++;
        printf("%s joins the server\n", c.name);
    } else {
        printf("Maximum number of clients reached. Cannot accept more.\n");
        closesocket(csock);
    }
}

static void handle_client_input(Client *clients, Client *client, int actual, int max) {

}

static void handle_client_state(Client *clients, Client *client, int actual, fd_set *rdfs, char *buffer, int i) {
    int c;

    switch (client->state) {
        case IDLE:
            if (FD_ISSET(client->sock, rdfs)) {
                c = read_client(client->sock, buffer);
                if (c == 0) {
                    handle_disconnect_client(*client, clients, i, actual);
                } else if ((strcmp(buffer, "ls") == 0) || (strcmp(buffer, "list") == 0)) {
                    send_list_of_clients(clients, *client, actual, client->sock, buffer, 0);
                } else if (strcmp(buffer, "v") == 0) {
                    printf("Client %s is challenging\n", client->name);
                    client->state = CHALLENGE;
                    challenge_another_client_init(clients, client, actual, client->sock, buffer, 0);
                } else if (strcmp(buffer, "exit") == 0) {
                    closesocket(client->sock);
                    remove_client(clients, actual, client);
                } else {
                    send_message_to_all_clients(clients, *client, actual, buffer, 0);
                }
            }
            break;

        case CHALLENGE:
            if (FD_ISSET(client->sock, rdfs)) {
                c = read_client(client->sock, buffer);
                if (c == 0) {
                    handle_disconnect_client(*client, clients, i, actual);
                } else {
                    printf("Client %s is challenging\n", client->name);
                    challenge_another_client_request(clients, client, actual, client->sock, buffer, 0);
                }
            }
            break;

        case CHOICE:
            if (FD_ISSET(client->sock, rdfs)) {
                c = read_client(client->sock, buffer);
                if (c == 0) {
                    handle_disconnect_client(*client, clients, i, actual);
                } else {
                    // logic for handling CHOICE state
                }
            }
            break;

        case BUSY:
            // Add logic for BUSY state if necessary
            break;

        default:
            fprintf(stderr, "Unknown state for client %s\n", client->name);
            break;
    }
}


static void app(void) {
    SOCKET sock = init_connection();
    char buffer[BUF_SIZE];
    /* the index for the array */
    int actual = 0;
    int max = sock;
    /* an array for all clients */
    Client clients[MAX_CLIENTS];
    AwaleGame games[MAX_GAMES];
    fd_set rdfs;

    while (1) {
        int i = 0;
        FD_ZERO(&rdfs);

        /* add STDIN_FILENO */
        FD_SET(STDIN_FILENO, &rdfs);

        /* add the connection socket */
        FD_SET(sock, &rdfs);

        /* add socket of each client */
        for (i = 0; i < actual; i++) {
            FD_SET(clients[i].sock, &rdfs);
        }

        if (select(max + 1, &rdfs, NULL, NULL, NULL) == -1) {
            perror("select()");
            exit(errno);
        }

        /* something from standard input : i.e keyboard */
        if (FD_ISSET(STDIN_FILENO, &rdfs)) {
            /* stop process when type on keyboard */
            break;
        }

        if (FD_ISSET(sock, &rdfs)) {
            /* new client */
            handle_new_client(sock, clients, &actual, &max);
        }

        for (int i = 0; i < actual; i++) {
            if (FD_ISSET(clients[i].sock, &rdfs)) {
                handle_client_state(clients, &clients[i], actual, &rdfs, buffer, i);
            }
        }
    }

    clear_clients(clients, actual);
    end_connection(sock);
}


static void clear_clients(Client *clients, int actual) {
    int i = 0;
    for (i = 0; i < actual; i++) {
        closesocket(clients[i].sock);
    }
}

static void handle_disconnect_client(Client client, Client *clients, int i, int actual) {
    char buffer[BUF_SIZE];
    closesocket(clients[i].sock);
    remove_client(clients, i, &actual);
    strncpy(buffer, client.name, BUF_SIZE - 1);
    strncat(buffer, " disconnected !", BUF_SIZE - strlen(buffer) - 1);
    send_message_to_all_clients(clients, client, actual, buffer, 1);
}

static void remove_client(Client *clients, int to_remove, int *actual) {
    /* we remove the client in the array */
    memmove(clients + to_remove, clients + to_remove + 1, (*actual - to_remove - 1) * sizeof(Client));
    /* number client - 1 */
    (*actual)--;
}

static void
send_message_to_all_clients(Client *clients, Client sender, int actual, const char *buffer, char from_server) {
    int i = 0;
    char message[BUF_SIZE];
    message[0] = 0;
    for (i = 0; i < actual; i++) {
        /* we don't send message to the sender */
        if (sender.sock != clients[i].sock) {
            if (from_server == 0) {
                strncpy(message, sender.name, BUF_SIZE - 1);
                strncat(message, " : ", sizeof message - strlen(message) - 1);
            }
            strncat(message, buffer, sizeof message - strlen(message) - 1);
            write_client(clients[i].sock, message);
        }
    }
}

static void
send_list_of_clients(Client *clients, Client client, int actual, int sender_sock, const char *buffer, int from_server) {
    char list_buffer[1024]; // Assuming 1024 is sufficient
    strcpy(list_buffer, "Connected clients:\n");

    for (int i = 0; i < actual; i++) {
        strcat(list_buffer, clients[i].name);
        strcat(list_buffer, "\n");
        // printf("Client %d: %s\n", i, clients[i].name);
    }

    if (sender_sock != -1) {
        // Send only to the requesting client
        write_client(client.sock, list_buffer);
    }
}

static void
challenge_another_client_init(Client *clients, Client *client, int actual, int sender_sock, const char *buffer,
                              int from_server) {
    char list_buffer[1024]; // Assuming 1024 is sufficient

    strcpy(list_buffer, "Connected clients:\n");

    for (int i = 0; i < actual; i++) {
        if (clients[i].sock == sender_sock || clients[i].state != IDLE) {
            continue;
        }
        strcat(list_buffer, clients[i].name);
        strcat(list_buffer, "\n");

    }

    if (sender_sock != -1) {
        // Send only to the requesting client
        write_client(client->sock, list_buffer);
        char challenge_buffer[1024];
        strcpy(challenge_buffer, "Who do you want to fight?\n");
        write_client(client->sock, challenge_buffer);
    } else {
        return;
    }


}

static void
challenge_another_client_request(Client *clients, Client *client, int actual, int sender_sock, const char *buffer,
                                 int from_server) {

    int challengee_sock = -1;

    if (strcmp(client->name, buffer) == 0) {
        printf("Client %s tried to challenge themselves\n", client->name);
        write_client(client->sock, "You can't challenge yourself\n");

        return;
    }

    for (int i = 0; i < actual; i++) {
        if (strcmp(clients[i].name, buffer) == 0 && clients[i].state == IDLE) {

            challengee_sock = clients[i].sock;
            clients[i].state = CHOICE;
            clients[i].opponent = client;
            client->opponent = (clients + i);
            break;
        }
    }

    if (challengee_sock == -1) {
        // Challengee not found
        char not_found_buffer[1024]; // Assuming 1024 is sufficient
        strcpy(not_found_buffer, "Opponent not found\n");

        if (sender_sock != -1) {
            // Send only to the requesting client
            write_client(client->sock, not_found_buffer);
        }
    } else {
        // Opponent found
        char challenge_buffer[1024]; // Assuming 1024 is sufficient
        strcpy(challenge_buffer, "Opponent found\n");
        write_client(client->sock, challenge_buffer);

        char challengee_response_buffer[1024]; // Assuming 1024 is sufficient
        strcpy(challengee_response_buffer, "Do you want to accept the fight?\n");
        write_client(challengee_sock, challengee_response_buffer);

    }

}

static int init_connection(void) {
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    SOCKADDR_IN sin = {0};

    if (sock == INVALID_SOCKET) {
        perror("socket()");
        exit(errno);
    }

    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons(PORT);
    sin.sin_family = AF_INET;

    if (bind(sock, (SOCKADDR *) &sin, sizeof sin) == SOCKET_ERROR) {
        perror("bind()");
        exit(errno);
    }

    if (listen(sock, MAX_CLIENTS) == SOCKET_ERROR) {
        perror("listen()");
        exit(errno);
    }

    return sock;
}

static void end_connection(int sock) {
    closesocket(sock);
}

static int read_client(SOCKET sock, char *buffer) {
    int n = 0;

    if ((n = recv(sock, buffer, BUF_SIZE - 1, 0)) < 0) {
        perror("recv()");
        /* if recv error we disonnect the client */
        n = 0;
    }

    buffer[n] = 0;

    return n;
}

static void write_client(SOCKET sock, const char *buffer) {
    if (send(sock, buffer, strlen(buffer), 0) < 0) {
        perror("send()");
        exit(errno);
    }
}

int main(int argc, char **argv) {
    init();

    app();

    end();

    return EXIT_SUCCESS;
}
