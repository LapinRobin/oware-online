#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "server2.h"
#include "client2.h"

void display_board(AwaleGame *game, int board[], int score[])
{
    char buffer[BUF_SIZE];
    char numStr[2];
    buffer[0] = '\0';
    strcat(buffer, "\nGame Board:\n");
    strcat(buffer, "Player 1 (top): ");

    for (int i = 0; i < NB_HOUSES_PER; i++)
    {
        numStr[0] = '\0';
        sprintf(numStr, "%d", board[i]);
        strcat(buffer, numStr);
        strcat(buffer, " ");
    }

    strcat(buffer, "\nscore: ");
    numStr[0] = '\0';
    sprintf(numStr, "%d", score[0]);
    strcat(buffer, numStr);
    strcat(buffer, " \n");

    strcat(buffer, "Player 2 (buttom): ");
    for (int i = NB_HOUSES_TOTAL - 1; i >= NB_HOUSES_PER; i--)
    {
        numStr[0] = '\0';
        sprintf(numStr, "%d", board[i]);
        strcat(buffer, numStr);
        strcat(buffer, " ");
    }

    strcat(buffer, "\nscore: ");
    numStr[0] = '\0';
    sprintf(numStr, "%d", score[1]);
    strcat(buffer, numStr);
    strcat(buffer, " ");

    write_client(game->player1->sock, buffer);
    write_client(game->player2->sock, buffer);
}

void distribute_pieces(int board[])
{
    for (int i = 0; i < NB_HOUSES_TOTAL; i++)
    {
        board[i] = NB_SEEDS;
    }
}

int is_valid_move(int board[], int player, int position)
{
    int totalPlayer1 = 0, totalPlayer2 = 0;
    for (int i = 0; i < NB_HOUSES_PER; i++)
    {
        totalPlayer1 += board[i];
    }
    for (int i = NB_HOUSES_PER; i < NB_HOUSES_TOTAL; i++)
    {
        totalPlayer2 += board[i];
    }

    if (totalPlayer1 == 0 && player == 2)
    {
        if (position >= NB_HOUSES_PER && position < NB_HOUSES_TOTAL &&
            board[position] + position >= NB_HOUSES_PER)
            return 1;
        else
            return 0;
    }

    if (totalPlayer2 == 0 && player == 1)
    {
        if (position >= 0 && position < NB_HOUSES_PER && board[position] + position >= NB_HOUSES_PER)
            return 1;
        else
            return 0;
    }

    if ((player == 1 && position >= 0 && position < NB_HOUSES_PER) ||
        (player == 2 && position >= NB_HOUSES_PER && position < NB_HOUSES_TOTAL))
    {
        if (board[position] > 0)
        {
            return 1;
        }
    }
    return 0;
}

void playable_positions(int board[], int player, int positions[])
{
    int index = 0;
    for (int i = 0; i < NB_HOUSES_TOTAL; i++)
    {
        if (is_valid_move(board, player, i))
        {
            positions[index] = i;
            index++;
        }
    }
    positions[index] = -1; // Mark the end of the array
}

void play_move(int board[], int score[], int player, int position)
{
    int index = position;
    int pieces = board[position];
    board[position] = 0;

    while (pieces > 0)
    {
        index = (index + 1) % NB_HOUSES_TOTAL;
        if (index == position)
        {
            continue;
        }
        board[index]++;
        pieces--;
    }

    while ((player == 1 && index >= NB_HOUSES_PER && index < NB_HOUSES_TOTAL &&
            (board[index] == 2 || board[index] == 3)) ||
           (player == 2 && index >= 0 && index < NB_HOUSES_PER && (board[index] == 2 || board[index] == 3)))
    {
        score[player - 1] += board[index];
        board[index] = 0;
        index = (index - 1) % NB_HOUSES_TOTAL;
    }
}

int is_game_over(AwaleGame *game, char status[], int board[], int score[])
{
    int total = 0;

    if (score[0] > NB_SEEDS * NB_HOUSES_PER)
    {
        game->player1->score++;
        strcat(status, "Player 1 ");
        strcat(game->status, " name : ");
        strcat(game->status, game->player1->name);
        strcat(game->status, " won.\n");
        return 1;
    }
    if (score[1] > NB_SEEDS * NB_HOUSES_PER)
    {
        game->player2->score++;
        strcat(status, "Player 2");
        strcat(game->status, " name : ");
        strcat(game->status, game->player2->name);
        strcat(game->status, " won.\n");
        return 1;
    }

    for (int i = 0; i < NB_HOUSES_TOTAL; i++)
    {
        total += board[i];
    }

    if (total <= 6)
    {
        if (score[0] > score[1])
        {
            game->player1->score++;
            strcat(status, "Player 1");
            strcat(game->status, " name : ");
            strcat(game->status, game->player1->name);
            strcat(game->status, " won.\n");
        }
        else if (score[0] < score[1])
        {
            game->player2->score++;
            strcat(status, "Player 2");
            strcat(game->status, " name : ");
            strcat(game->status, game->player2->name);
            strcat(game->status, " won.\n");
        }
        else
        {
            strcat(status, "Two players tie.\n");
        }
        return 1;
    }

    return 0;
}

void collect_seeds(int board[], int score[], int currentPlayer)
{
    int total = 0;
    for (int i = 0; i < NB_HOUSES_TOTAL; i++)
    {
        total += board[i];
        board[i] = 0;
    }
    score[currentPlayer - 1] += total;
}

int is_number(char *str)
{
    int i = 0;

    while (str[i] != '\0')
    {
        if (str[i] < '0' || str[i] > '9')
        {
            return 0;
        }
        i++;
    }
    return 1;
}

void init_game(AwaleGame *game)
{
    for (int i = 0; i < NB_HOUSES_TOTAL; i++)
    {
        game->board[i] = NB_SEEDS;
    }
    game->score[0] = 0;
    game->score[1] = 0;
    game->currentPlayer = 1;
    distribute_pieces(game->board);
}

void game_play(AwaleGame *game)
{
    Client *player = (game->currentPlayer == 1) ? game->player1 : game->player2;
    display_board(game, game->board, game->score);
    int possibleCases[NB_HOUSES_TOTAL + 1];
    playable_positions(game->board, game->currentPlayer, possibleCases);
    char buffer[BUF_SIZE];
    char numStr[BUF_SIZE];
    buffer[0] = '\0';
    numStr[0] = '\0';
    sprintf(numStr, "%d", game->currentPlayer);
    if (is_game_over(game, game->status, game->board, game->score))
    {
        game_over(game);
    }
    else if (possibleCases[0] == -1)
    {
        strcat(buffer, "\nAll remaining pieces are in the same camp and player ");
        strcat(buffer, numStr);
        strcat(buffer, " can no longer feed the opponent.\n");
        write_client(game->player1->sock, buffer);
        write_client(game->player2->sock, buffer);
        collect_seeds(game->board, game->score, game->currentPlayer);
        game_over(game);
    }
    else
    {
        strcat(buffer, "Playable positions for player ");
        strcat(buffer, numStr);
        strcat(buffer, " : \n");
        for (int i = 0; possibleCases[i] != -1; i++)
        {
            numStr[0] = '\0';
            sprintf(numStr, "%d", possibleCases[i]);
            strcat(buffer, numStr);
            strcat(buffer, " ");
        }
        strcat(buffer, "\n\nPlayer ");
        numStr[0] = '\0';
        sprintf(numStr, "%d", game->currentPlayer);
        strcat(buffer, numStr);
        strcat(buffer, ", enter the position of the move, or enter s to surrender this game: ");
        write_client(player->sock, buffer);
    }
}

void game_over(AwaleGame *game)
{
    write_client(game->player1->sock, "\nGame over. Final score:\n");
    write_client(game->player2->sock, "\nGame over. Final score:\n");
    display_board(game, game->board, game->score);
    char buffer[BUF_SIZE];
    buffer[0] = '\0';
    strcat(buffer, "\nGame status : ");
    strcat(buffer, game->status);
    strcat(buffer, "\n");
    write_client(game->player1->sock, buffer);
    write_client(game->player2->sock, buffer);
    game->player1->state = IDLE;
    game->player2->state = IDLE;
    game->player1->opponent = NULL;
    game->player2->opponent = NULL;
}

static void init(void)
{
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

static void end(void)
{
#ifdef WIN32
    WSACleanup();
#endif
}

static void handle_new_client(SOCKET sock, Client *clients, int *actual, int *max)
{
    SOCKADDR_IN csin = {0};
    socklen_t sinsize = sizeof(csin);
    char buffer[BUF_SIZE];
    int csock = accept(sock, (SOCKADDR *)&csin, &sinsize);

    if (csock == SOCKET_ERROR)
    {
        perror("accept()");
        return;
    }

    if (read_client(csock, buffer) == -1)
    {
        // If reading from client failed, close the socket and return
        closesocket(csock);
        return;
    }

    // Update the maximum file descriptor if necessary
    *max = csock > *max ? csock : *max;

    buffer[26] = '\0'; // Truncate to 26 characters
    for (int i = 0; buffer[i] != '\0'; i++)
    {
        if ((unsigned char)buffer[i] > 127)
        {
            // write to client: invalid name (contains non-ASCII characters)
            char message[50];
            message[0] = '\0';
            strcat(message, "Invalid name, contains non-ASCII characters.\n");
            write_client(csock, message);

            closesocket(csock);
            return;
        }
    }
    // Initialize the new client
    Client c = {csock};
    strncpy(c.name, buffer, 26);
    c.name[26] = '\0'; // Ensure null-termination
    c.state = IDLE;
    c.score = 0;
    c.currentGame = -1;
    // Add the new client to the clients array
    if (*actual < MAX_CLIENTS)
    {
        clients[*actual] = c;
        (*actual)++;
        printf("%s joins the server\n", c.name);
        write_client(c.sock, "Welcome to oware game server! Enter :help to get help.");
    }
    else
    {
        printf("Maximum number of clients reached. Cannot accept more.\n");
        closesocket(csock);
    }
}

static void handle_client_input(Client *clients, Client *client, int actual, int max)
{
}

static void handle_client_state(Client *clients, Client *client, int *actual, fd_set *rdfs, char *buffer, int i, AwaleGame games[], AwaleGame *current_game, int game_index[])
{
    int c;
    AwaleGame *game;
    buffer[0] = '\0';
    switch (client->state)
    {
    case IDLE:
        if (FD_ISSET(client->sock, rdfs))
        {
            c = read_client(client->sock, buffer);
            if (c == 0)
            {
                handle_disconnect_client(clients, *client, i, actual);
            }
            else if ((strcmp(buffer, ":help") == 0))
            {
                write_client(client->sock, "Available commands:\n");
                write_client(client->sock, "`:ls` or `:list` - list all connected clients\n");
                write_client(client->sock, "`:exit`, `CTRL-C` or `CTRL-D` - shut down the server\n");
            }
            else if ((strcmp(buffer, ":ls") == 0) || (strcmp(buffer, ":list") == 0))
            {
                send_list_of_clients(clients, *client, *actual, client->sock, buffer, 0);
            }
            else if ((strcmp(buffer, ":v") == 0) || (strcmp(buffer, ":vie") == 0))
            {
                printf("Client %s is challenging\n", client->name);
                client->state = CHALLENGE;
                challenge_another_client_init(clients, client, *actual, client->sock, buffer, 0);
            }
            else if (strcmp(buffer, ":exit") == 0)
            {
                handle_disconnect_client(clients, *client, i, actual);
            }
            else
            {
                send_message_to_all_clients(clients, *client, *actual, buffer, 0);
            }
        }
        break;

    case CHALLENGE:
        if (FD_ISSET(client->sock, rdfs))
        {
            c = read_client(client->sock, buffer);
            if (c == 0)
            {
                handle_disconnect_client(clients, *client, i, actual);
            }
            else
            {
                challenge_another_client_request(clients, client, *actual, client->sock, buffer, 0);
            }
        }
        break;

    case CHOICE:
        if (FD_ISSET(client->sock, rdfs))
        {
            c = read_client(client->sock, buffer);
            if (c == 0)
            {
                write_client(client->opponent->sock, "Your opponent has disconnected!");
                client->opponent->state = IDLE;
                client->opponent->opponent = NULL;
                client->opponent = NULL;
                handle_disconnect_client(clients, *client, i, actual);
            }
            else
            {
                char message[BUF_SIZE];
                message[0] = '\0';
                if (strcmp(buffer, "yes") == 0)
                {
                    printf("Client %s accepted the challenge\n", client->name);

                    strcat(message, "Are you ready? Game start!\n");

                    write_client(client->sock, message);
                    write_client(client->opponent->sock, message);

                    // start game
                    current_game->player1 = client;
                    current_game->player2 = client->opponent;
                    init_game(current_game);
                    write_client(current_game->player1->sock, "You are player1, you move first.\n");
                    write_client(current_game->player2->sock, "You are player2, you move after player1 moved.\n");
                    client->currentGame = game_index[0];
                    client->opponent->currentGame = game_index[0];
                    client->state = PLAYER1;
                    client->opponent->state = PLAYER2;
                    game_play(current_game);

                    game_index[0]++;
                }
                else if (strcmp(buffer, "no") == 0)
                {
                    printf("Client %s declined the challenge.\n", client->name);
                    client->state = IDLE;
                    client->opponent->state = IDLE;
                    // send message to challenger that challengee declined
                    strcat(message, "You declined the challenge.\n");

                    write_client(client->sock, message);

                    message[0] = '\0';
                    strcat(message, "Your opponent declined the challenge.\n");

                    write_client(client->opponent->sock, message);
                }
                else
                {
                    strcat(message, "Invalid response. Please resend the answer.\n");
                    printf("Client %s sent invalid response\n", client->name);
                    write_client(client->sock, message);
                }
            }
        }
        break;

    case PLAYER1:
        game = (games + client->currentGame);
        char numStr[BUF_SIZE];
        buffer[0] = '\0';
        numStr[0] = '\0';
        sprintf(numStr, "%d", game->currentPlayer);
        Client *anotherPlayer = client->opponent;
        if (FD_ISSET(client->sock, rdfs))
        {
            int check = 0;
            int end = 0;
            c = read_client(client->sock, buffer);
            if (c == 0)
            {
                anotherPlayer->score++;
                write_client(anotherPlayer->sock, "Your opponent disconnected, you won this game!\n");
                strcat(game->status, "Player ");
                numStr[0] = '\0';

                sprintf(numStr, "%d", (game->currentPlayer == 1) ? 2 : 1);
                strcat(game->status, numStr);

                strcat(game->status, " name : ");
                strcat(game->status, anotherPlayer->name);
                strcat(game->status, " won.\n");
                end = 1;

                anotherPlayer->opponent = NULL;
                anotherPlayer->state = IDLE;
                strcat(buffer, "\nGame status : ");
                strcat(buffer, game->status);
                strcat(buffer, "\n");
                write_client(anotherPlayer->sock, buffer);
                handle_disconnect_client(clients, *client, i, actual);
            }
            else if ((strcmp(buffer, "s") == 0))
            {
                end = 1;
                anotherPlayer->score++;
                write_client(anotherPlayer->sock, "Your opponent surrendered and you won the game!\n");
                strcat(game->status, "Player ");
                numStr[0] = '\0';
                sprintf(numStr, "%d", (game->currentPlayer == 1) ? 2 : 1);
                strcat(game->status, numStr);
                strcat(game->status, " name : ");
                strcat(game->status, anotherPlayer->name);

                strcat(game->status, " won.\n");
                game_over(game);
            }
            else if (is_number(buffer))
            {
                game->position = atoi(buffer);
                check = 1;
            }
            else
            {
                write_client(client->sock, "Invalid input.\n");
            }

            if (check && is_valid_move(game->board, game->currentPlayer, game->position))
            {
                play_move(game->board, game->score, game->currentPlayer, game->position);
                game->currentPlayer = (game->currentPlayer == 1) ? 2 : 1;
                client->state = PLAYER2;
                anotherPlayer->state = PLAYER1;
                game_play(game);
            }
            else if (!end)
            {
                write_client(client->sock, "Invalid move. Please choose a valid position.\n");
            }
        }
        break;

    case PLAYER2:
        game = (games + client->currentGame);
        anotherPlayer = client->opponent;

        if (FD_ISSET(client->sock, rdfs))
        {
            c = read_client(client->sock, buffer);
            if (c == 0)
            {
                anotherPlayer->score++;
                write_client(anotherPlayer->sock, "Your opponent disconnected, you won this game!\n");
                strcat(game->status, "Player ");
                numStr[0] = '\0';

                sprintf(numStr, "%d", game->currentPlayer);
                strcat(game->status, numStr);

                strcat(game->status, " name : ");
                strcat(game->status, anotherPlayer->name);
                strcat(game->status, " won.\n");
                anotherPlayer->opponent = NULL;
                anotherPlayer->state = IDLE;
                strcat(buffer, "\nGame status : ");
                strcat(buffer, game->status);
                strcat(buffer, "\n");
                write_client(anotherPlayer->sock, buffer);
                handle_disconnect_client(clients, *client, i, actual);
            }
        }
        break;

    default:
        fprintf(stderr, "Unknown state for client %s\n", client->name);
        break;
    }
}

static void handle_server_input(Client *clients, int actual, int sock, char *buffer)
{
    if (strcmp(buffer, ":help\n") == 0)
    {
        printf("Available commands:\n");
        printf("`:ls` or `:list` - list all connected clients\n");
        printf("`:v` - invite a client to play an oware game\n");
        printf("`:exit`, `CTRL-C` or `CTRL-D` - shut down the server\n");
    }
    else if (strcmp(buffer, ":exit\n") == 0)
    {
        clear_clients(clients, actual);
        end_connection(sock);
        exit(0);
    }
    else if (strcmp(buffer, ":ls\n") == 0 || strcmp(buffer, ":list\n") == 0)
    {
        if (actual == 0)
        {
            printf("No client connected\n");
            return;
        }

        int width = 30;           // Width of the frame
        char line[width * 3 + 1]; // +1 for the null terminator

        // Create a horizontal line with '─'
        memset(line, 0xE2, width * 3); // 0xE2 is the first byte of '─' in UTF-8
        for (int i = 0; i < width * 3; i += 3)
        {
            line[i + 1] = 0x94;
            line[i + 2] = 0x80;
        }
        line[width * 3] = '\0'; // Null-terminate the string

        printf("\u250C%s\u2510\n", line); // Print the top border with corners
        printf("\u2502  List of connected clients:  \u2502\n");

        printf("\u251C"); // Left border of the separator line
        for (int i = 0; i < width - 2; i += 3)
        {
            printf("%s", "\u2500\u2500\u2500"); // Middle part of the separator line
        }
        printf("\u2524\n"); // Right border of the separator line

        for (int i = 0; i < actual; i++)
        {
            printf("\u2502  %-27s \u2502\n", clients[i].name);
            // client name max length: 26
        }
        printf("\u2514%s\u2518\n", line); // Print the bottom border with corners
    }
    else
    {
        printf("Server received unknown command.\n");
    }
}

static void app(void)
{
    SOCKET sock = init_connection();
    char buffer[BUF_SIZE];
    /* the index for the array */
    int actual = 0;
    int max = sock;
    /* an array for all clients */
    Client clients[MAX_CLIENTS];
    AwaleGame games[MAX_GAMES];
    int game_index[1];
    game_index[0] = 0;
    fd_set rdfs;

    while (1)
    {

        FD_ZERO(&rdfs);

        /* add STDIN_FILENO */
        FD_SET(STDIN_FILENO, &rdfs);

        /* add the connection socket */
        FD_SET(sock, &rdfs);

        /* add socket of each client */
        for (int i = 0; i < actual; i++)
        {
            FD_SET(clients[i].sock, &rdfs);
        }

        if (select(max + 1, &rdfs, NULL, NULL, NULL) == -1)
        {
            perror("select()");
            exit(errno);
        }

        /* something from standard input : i.e keyboard */
        if (FD_ISSET(STDIN_FILENO, &rdfs))
        {
            char buffer[1024]; // Buffer for input
            ssize_t bytesRead = read(STDIN_FILENO, buffer, sizeof(buffer) - 1);
            if (bytesRead > 1)
            {
                buffer[bytesRead] = '\0';
                handle_server_input(clients, actual, sock, buffer);
            }
            else if (bytesRead == 0)
            {
                // EOF
                break;
            }
            else
            {
                perror("read");
                exit(EXIT_FAILURE);
            }
        }

        if (FD_ISSET(sock, &rdfs))
        {
            /* new client */
            handle_new_client(sock, clients, &actual, &max);
        }

        for (int i = 0; i < actual; i++)
        {
            if (FD_ISSET(clients[i].sock, &rdfs))
            {
                handle_client_state(clients, &clients[i], &actual, &rdfs, buffer, i, games, (games + game_index[0]), game_index);
            }
        }
    }

    clear_clients(clients, actual);
    end_connection(sock);
}

static void clear_clients(Client *clients, int actual)
{
    for (int i = 0; i < actual; i++)
    {
        closesocket(clients[i].sock);
    }
}

static void handle_disconnect_client(Client *clients, Client client, int i, int *actual)
{
    // char buffer[BUF_SIZE];
    closesocket(clients[i].sock);
    remove_client(clients, i, actual);
    printf("%s disconnected\n", client.name);
    // strncpy(buffer, client.name, BUF_SIZE - 1);
    // strncat(buffer, " disconnected !", BUF_SIZE - strlen(buffer) - 1);
    // send_message_to_all_clients(clients, client, actual, buffer, 1);
}

static void remove_client(Client *clients, int to_remove, int *actual)
{
    /* we remove the client in the array */
    memmove(clients + to_remove, clients + to_remove + 1, (*actual - to_remove - 1) * sizeof(Client));
    /* number client - 1 */
    (*actual)--;
}

static void
send_message_to_all_clients(Client *clients, Client sender, int actual, const char *buffer, char from_server)
{
    int i = 0;
    char message[BUF_SIZE];
    message[0] = 0;
    for (i = 0; i < actual; i++)
    {
        /* we don't send message to the sender */
        if (sender.sock != clients[i].sock)
        {
            if (from_server == 0)
            {
                strncpy(message, sender.name, BUF_SIZE - 1);
                strncat(message, " : ", sizeof message - strlen(message) - 1);
            }
            strncat(message, buffer, sizeof message - strlen(message) - 1);
            write_client(clients[i].sock, message);
        }
    }
}

static void
send_list_of_clients(Client *clients, Client client, int actual, int sender_sock, const char *buffer, int from_server)
{
    char list_buffer[1024]; // Assuming 1024 is sufficient
    char numStr[1024];
    strcpy(list_buffer, "Connected clients:\n");

    for (int i = 0; i < actual; i++)
    {
        numStr[0] = '\0';
        strcat(list_buffer, clients[i].name);
        strcat(list_buffer, " score : ");
        sprintf(numStr, "%d", clients[i].score);
        strcat(list_buffer, numStr);
        strcat(list_buffer, "\n");
        // printf("Client %d: %s\n", i, clients[i].name);
    }

    if (sender_sock != -1)
    {
        // Send only to the requesting client
        write_client(client.sock, list_buffer);
    }
}

static void
challenge_another_client_init(Client *clients, Client *client, int actual, int sender_sock, const char *buffer,
                              int from_server)
{
    char list_buffer[1024]; // Assuming 1024 is sufficient

    strcpy(list_buffer, "Connected clients:\n");

    for (int i = 0; i < actual; i++)
    {
        if (clients[i].sock == sender_sock || clients[i].state != IDLE)
        {
            continue;
        }
        strcat(list_buffer, clients[i].name);
        strcat(list_buffer, "\n");
    }

    if (sender_sock != -1)
    {
        // Send only to the requesting client
        write_client(client->sock, list_buffer);
        char challenge_buffer[1024];
        strcpy(challenge_buffer, "Who do you want to fight?\n");
        write_client(client->sock, challenge_buffer);
    }
    else
    {
        return;
    }
}

static void
challenge_another_client_request(Client *clients, Client *client, int actual, int sender_sock, const char *buffer,
                                 int from_server)
{

    int challengee_sock = -1;

    if (strcmp(client->name, buffer) == 0)
    {
        printf("Client %s tried to challenge themselves\n", client->name);
        write_client(client->sock, "You can't challenge yourself\n");

        return;
    }

    for (int i = 0; i < actual; i++)
    {
        if (strcmp(clients[i].name, buffer) == 0 && clients[i].state == IDLE)
        {

            challengee_sock = clients[i].sock;
            clients[i].state = CHOICE;
            clients[i].opponent = client;
            client->opponent = (clients + i);
            break;
        }
    }

    if (challengee_sock == -1)
    {
        // Challengee not found
        char not_found_buffer[1024]; // Assuming 1024 is sufficient
        strcpy(not_found_buffer, "Opponent not found\n");

        if (sender_sock != -1)
        {
            // Send only to the requesting client
            write_client(client->sock, not_found_buffer);
        }
    }
    else
    {
        // Opponent found
        char challenge_buffer[1024]; // Assuming 1024 is sufficient
        strcpy(challenge_buffer, "Opponent found\n");
        strcpy(challenge_buffer, "Your opponent is thinking about your invitation.\n");
        write_client(client->sock, challenge_buffer);

        char challengee_response_buffer[1024]; // Assuming 1024 is sufficient
        challengee_response_buffer[0] = '\0';
        strcat(challengee_response_buffer, "Your received an invitation from user: ");
        strcat(challengee_response_buffer, client->name);
        strcat(challengee_response_buffer, " \n");
        strcat(challengee_response_buffer, "Do you want to accept the fight?\n");

        write_client(challengee_sock, challengee_response_buffer);
    }
}

static int init_connection(void)
{
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    SOCKADDR_IN sin = {0};

    if (sock == INVALID_SOCKET)
    {
        perror("socket()");
        exit(errno);
    }

    int yes = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons(PORT);
    sin.sin_family = AF_INET;

    if (bind(sock, (SOCKADDR *)&sin, sizeof sin) == SOCKET_ERROR)
    {
        perror("bind()");
        exit(errno);
    }

    if (listen(sock, MAX_CLIENTS) == SOCKET_ERROR)
    {
        perror("listen()");
        exit(errno);
    }

    return sock;
}

static void end_connection(int sock)
{
    closesocket(sock);
}

static int read_client(SOCKET sock, char *buffer)
{
    int n = 0;

    if ((n = recv(sock, buffer, BUF_SIZE - 1, 0)) < 0)
    {
        perror("recv()");
        /* if recv error we disonnect the client */
        n = 0;
    }

    buffer[n] = 0;

    return n;
}

static void write_client(SOCKET sock, const char *buffer)
{
    if (send(sock, buffer, strlen(buffer), 0) < 0)
    {
        perror("send()");
        exit(errno);
    }
}

int main(int argc, char **argv)
{
    init();

    app();

    end();

    return EXIT_SUCCESS;
}
