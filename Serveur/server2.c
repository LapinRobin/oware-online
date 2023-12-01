#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <errno.h>
#include <string.h>
#include <math.h>
#include "server2.h"
#include "client2.h"

void display_board(AwaleGame *game, int board[], int score[], Client *client)
{
    char buffer[BUF_SIZE];
    char numStr[3];
    buffer[0] = '\0';
    strcat(buffer, "\nGame Board:\n");
    strcat(buffer, "Player 1 (top): \n");
    strcat(buffer, "score: ");
    numStr[0] = '\0';
    sprintf(numStr, "%d", score[0]);
    strcat(buffer, numStr);
    strcat(buffer, " \n");
    strcat(buffer, "|");
    for (int i = 0; i < NB_HOUSES_PER; i++)
    {
        numStr[0] = '\0';
        sprintf(numStr, "%d", board[i]);
        strcat(buffer, numStr);
        strcat(buffer, "|");
    }
    strcat(buffer, " \n");
    strcat(buffer, "|");
    for (int i = NB_HOUSES_TOTAL - 1; i >= NB_HOUSES_PER; i--)
    {
        numStr[0] = '\0';
        sprintf(numStr, "%d", board[i]);
        strcat(buffer, numStr);
        strcat(buffer, "|");
    }
    strcat(buffer, "\nPlayer 2 (buttom): ");
    strcat(buffer, "\nscore: ");
    numStr[0] = '\0';
    sprintf(numStr, "%d", score[1]);
    strcat(buffer, numStr);
    strcat(buffer, " \n");

    write_client(client->sock, buffer);
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

double calculateExpectedScore(int ratingA, int ratingB)
{
    return 1.0 / (1 + pow(10, (ratingB - ratingA) / 400.0));
}

void updateEloRatings(Client *playerA, Client *playerB, int outcome)
{
    double expectedScoreA = calculateExpectedScore(playerA->score, playerB->score);
    double expectedScoreB = calculateExpectedScore(playerB->score, playerA->score);

    double actualScoreA, actualScoreB;

    if (outcome == 1)
    { // Player A wins
        actualScoreA = 1.0;
        actualScoreB = 0.0;
    }
    else if (outcome == -1)
    { // Player B wins
        actualScoreA = 0.0;
        actualScoreB = 1.0;
    }
    else
    { // Draw
        actualScoreA = 0.5;
        actualScoreB = 0.5;
    }

    // Update Elo ratings for both players
    playerA->score += K_FACTOR * (actualScoreA - expectedScoreA);
    playerB->score += K_FACTOR * (actualScoreB - expectedScoreB);
}

int is_game_over(AwaleGame *game, char status[], int board[], int score[])
{
    int total = 0;

    if (score[0] > NB_SEEDS * NB_HOUSES_PER)
    {
        updateEloRatings(game->player1, game->player2, 1);
        status[0] = '\0';
        strcat(status, "Player 1 ");
        strcat(game->status, " name : ");
        strcat(game->status, game->player1->name);
        strcat(game->status, " won.");
        return 1;
    }
    if (score[1] > NB_SEEDS * NB_HOUSES_PER)
    {
        updateEloRatings(game->player1, game->player2, -1);
        status[0] = '\0';
        strcat(status, "Player 2");
        strcat(game->status, " name : ");
        strcat(game->status, game->player2->name);
        strcat(game->status, " won.");
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
            updateEloRatings(game->player1, game->player2, 1);
            status[0] = '\0';
            strcat(status, "Player 1");
            strcat(game->status, " name : ");
            strcat(game->status, game->player1->name);
            strcat(game->status, " won.");
        }
        else if (score[0] < score[1])
        {
            updateEloRatings(game->player1, game->player2, -1);
            status[0] = '\0';
            strcat(status, "Player 2");
            strcat(game->status, " name : ");
            strcat(game->status, game->player2->name);
            strcat(game->status, " won.");
        }
        else
        {
            updateEloRatings(game->player1, game->player2, 0);
            status[0] = '\0';
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
    game->number_observer = 0;
    game->privacy_mode = 0;
    strcat(game->status, "In game");
    distribute_pieces(game->board);
}

void game_play(AwaleGame *game)
{
    Client *player = (game->currentPlayer == 1) ? game->player1 : game->player2;
    Client *anotherPlayer = (game->currentPlayer == 1) ? game->player2 : game->player1;

    display_board(game, game->board, game->score, game->player1);
    display_board(game, game->board, game->score, game->player2);
    for (int i = 0; i < game->number_observer; i++)
    {
        display_board(game, game->board, game->score, game->observer[i]);
    }
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
        is_game_over(game, game->status, game->board, game->score);
        game_over(game);
    }
    else
    {
        strcat(buffer, "Number positions progress clockwise from the top left corner to the bottom right corner.\n"); 
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
        strcat(buffer, ", enter the position of the move\n");
        write_client(player->sock, buffer);

        buffer[0] = '\0';
        strcat(buffer, "Enter ':s' to surrender this game\n");
        strcat(buffer, "Enter ':p' to open privacy mode\n");
        strcat(buffer, "Enter ':e' to close privacy mode\n");
        strcat(buffer, "Enter ':dm [message]' to send a message to your opponent\n");
        write_client(player->sock, buffer);
        write_client(anotherPlayer->sock, buffer);
    }
}

void game_over(AwaleGame *game)
{
    write_client(game->player1->sock, "\nGame over. Final score:\n");
    write_client(game->player2->sock, "\nGame over. Final score:\n");
    display_board(game, game->board, game->score, game->player1);
    display_board(game, game->board, game->score, game->player2);
    for (int i = 0; i < game->number_observer; i++)
    {
        write_client(game->observer[i]->sock, "\nGame over. Final score:\n");
        display_board(game, game->board, game->score, game->observer[i]);
    }
    char buffer[BUF_SIZE];
    buffer[0] = '\0';
    strcat(buffer, "\nGame status : ");
    strcat(buffer, game->status);
    strcat(buffer, "\n");
    write_client(game->player1->sock, buffer);
    write_client(game->player2->sock, buffer);
    for (int i = 0; i < game->number_observer; i++)
    {
        write_client(game->observer[i]->sock, buffer);
    }

    game->player1->opponent = NULL;
    game->player2->opponent = NULL;
    write_client(game->player1->sock, "Do you want to save this game in your history? (yes/no)\n");
    write_client(game->player2->sock, "Do you want to save this game in your history? (yes/no)\n");
    game->player1->state = HISTORY;
    game->player2->state = HISTORY;
}

void add_observer(AwaleGame *game, Client *observer_client)
{
    if (game->number_observer < MAX_CLIENTS)
    {
        game->observer[game->number_observer] = observer_client;
        game->number_observer++;
    }
}

void remove_observer(AwaleGame *game, Client *observer_client)
{
    int i, found = 0;

    for (i = 0; i < game->number_observer; i++)
    {
        if (game->observer[i] == observer_client)
        {
            found = 1;
            break;
        }
    }

    if (found)
    {
        for (int j = i; j < game->number_observer - 1; j++)
        {
            game->observer[j] = game->observer[j + 1];
        }
        game->number_observer--;
    }
}

int is_friend(Client *client, Client *friend)
{

    for (int i = 0; i < client->number_friend; i++)
    {
        if (strcmp(client->friend[i], friend->name) == 0)
        {
            return 1;
        }
    }
    return 0;
}

void add_friend(Client *client, Client *friend)
{
    if (client->number_friend < MAX_CLIENTS)
    {
        strcpy(client->friend[client->number_friend], friend->name);
        client->number_friend++;
    }
}

void remove_friend(Client *client, Client *friend)
{
    int i, found = 0;

    for (i = 0; i < client->number_friend; i++)
    {
        if (strcmp(client->friend[i], friend->name) == 0)
        {
            found = 1;
            break;
        }
    }

    if (found)
    {
        for (int j = i; j < client->number_friend - 1; j++)
        {
            strcpy(client->friend[j], client->friend[j + 1]);
        }
        client->number_friend--;
    }
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

    buffer[NAME_SIZE] = '\0'; // Truncate to 26 characters
    for (int i = 0; buffer[i] != '\0'; i++)
    {
        if ((unsigned char)buffer[i] > 127)
        {
            write_client(csock, "Invalid name, contains non-ASCII characters.\n");
            closesocket(csock);
            return;
        }
    }

    for (int i = 0; i < *actual; i++)
    {
        if (strcmp(clients[i].name, buffer) == 0)
        {
            write_client(csock, "Username already exists. Please choose another name.\n");
            closesocket(csock);
            return;
        }
    }

    // Initialize the new client
    Client c = {csock};
    strncpy(c.name, buffer, NAME_SIZE);
    c.name[NAME_SIZE] = '\0'; // Ensure null-termination
    c.state = IDLE;
    c.score = INITIAL_SCORE;
    c.currentGame = -1;
    c.observeGame = -1;
    c.number_friend = 0;
    c.number_game = 0;
    // Add the new client to the clients array
    if (*actual < MAX_CLIENTS)
    {
        clients[*actual] = c;
        (*actual)++;
        printf("%s joins the server\n", c.name);
        write_client(c.sock, "Welcome to oware game server\n");
        write_client(c.sock, "Enter `:help` to get help.");
    }
    else
    {
        printf("Maximum number of clients reached. Cannot accept more.\n");
        closesocket(csock);
    }
}

static void handle_client_state(Client *clients, Client *client, int *actual, fd_set *rdfs, char *buffer, int i, AwaleGame games[], AwaleGame *current_game, int game_index[])
{
    int c;
    int randomValue;
    AwaleGame *game;
    char message[BUF_SIZE];
    message[0] = '\0';
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
                write_client(client->sock, "\n");
                write_client(client->sock, "| Available commands:\n");
                write_client(client->sock, "\n");
                write_client(client->sock, "| `:ls` or `:list` - list all connected clients\n");
                write_client(client->sock, "| `:lsg` or `:listGames` - list all games\n");
                write_client(client->sock, "| `:lssg` or `:listSavedGames` - list all saved games\n");
                write_client(client->sock, "| `:rank` - show ranking of all connected clients\n");
                write_client(client->sock, "| `:v` or `:vie`- invite a user to an oware game\n");
                write_client(client->sock, "| `:o` or `:observe` - observe a game\n");
                write_client(client->sock, "| `:bio` - write your bio\n");
                write_client(client->sock, "| `:db [username]` - display bio of a user\n");
                write_client(client->sock, "| `:dm` - send a message to a user\n");
                write_client(client->sock, "| `:f` or `:friend` - add a friend\n");
                write_client(client->sock, "| `:rmf` or `:removeFriend` - remove a friend\n");
                write_client(client->sock, "| `:lf` or `:listFriends` - list all your friends\n");
                write_client(client->sock, "| `:exit`, `CTRL-C` - disconnect from the server\n");
                write_client(client->sock, "\n");
            }
            else if ((strcmp(buffer, ":ls") == 0) || (strcmp(buffer, ":list") == 0))
            {
                send_list_of_clients(clients, *client, *actual, client->sock, buffer, 0, 1);
            }
            else if ((strcmp(buffer, ":lsg") == 0) || (strcmp(buffer, ":listGames") == 0))
            {
                send_list_of_games(games, game_index, *client, *actual, client->sock, buffer, 0);
            }
            else if ((strcmp(buffer, ":lssg") == 0) || (strcmp(buffer, ":listSavedGames") == 0))
            {
                if (game_index[0] == 0 || client->number_game == 0)
                {
                    write_client(client->sock, "No available games.\n");
                }
                else
                {
                    send_list_of_saved_games(games, game_index, *client, *actual, client->sock, buffer, 0);
                    client->state = VIEWGAME;
                    write_client(client->sock, "Enter the game number to view the game, or ':exit' to cancel viewing.\n");
                }
            }
            else if ((strcmp(buffer, ":v") == 0) || (strcmp(buffer, ":vie") == 0))
            {
                if (*actual > 1)
                {
                    client->state = CHALLENGE;
                    challenge_another_client_init(clients, client, *actual, client->sock, buffer, 0);
                }
                else
                {
                    write_client(client->sock, "No available clients.\n");
                }
            }
            else if ((strcmp(buffer, ":rank") == 0))
            {
                send_ranking_to_client(clients, *client, *actual, client->sock, buffer, 0);
            }
            else if ((strcmp(buffer, ":o") == 0) || (strcmp(buffer, ":observe") == 0))
            {
                if (game_index[0] > 0)
                {
                    client->state = INITSTANDBY;
                    send_list_of_games(games, game_index, *client, *actual, client->sock, buffer, 0);
                    write_client(client->sock, "Enter the game number, or ':exit' to cancel spectating.\n");
                }
                else
                {
                    write_client(client->sock, "No available games.\n");
                }
            }
            else if ((strcmp(buffer, ":bio") == 0))
            {
                client->state = BIO;
                write_client(client->sock, "Write your bio here, or enter ':exit' to cancel the modification: \n");
            }
            else if (strncmp(buffer, ":db ", 4) == 0)
            {
                if (strlen(buffer) > 4)
                {
                    char name[NAME_SIZE + 1];
                    strncpy(name, buffer + 4, NAME_SIZE);
                    name[NAME_SIZE] = '\0';
                    int found = 0;
                    for (int i = 0; i < *actual; i++)
                    {
                        if (strcmp(clients[i].name, name) == 0)
                        {
                            found = 1;
                            write_client(client->sock, "User bio: ");
                            write_client(client->sock, clients[i].bio);
                            write_client(client->sock, "\n");
                            break;
                        }
                    }
                    if (!found)
                    {
                        write_client(client->sock, "User not found.\n");
                    }
                }
                else
                {
                    write_client(client->sock, "Invalid command.\n");
                }
            }
            else if ((strcmp(buffer, ":dm") == 0))
            {
                client->state = DM;
                send_list_of_clients(clients, *client, *actual, client->sock, buffer, 0, 0);
                write_client(client->sock, "Write the name of the person you want to send message to,\n"
                                           "Immediately after a space, write the message.\n"
                                           "Example: 'Alex Hello'\n"
                                           "Or enter ':exit' to cancel the message: \n");
            }
            else if ((strcmp(buffer, ":f") == 0) || (strcmp(buffer, ":friend") == 0))
            {
                if (*actual > 1)
                {
                    client->state = ADDFRIEND;
                    add_friend_init(clients, client, *actual, client->sock, buffer, 0);
                }
                else
                {
                    write_client(client->sock, "No available clients.\n");
                }
            }
            else if ((strcmp(buffer, ":rmf") == 0) || (strcmp(buffer, ":removeFriend") == 0))
            {
                if (*actual > 1)
                {
                    client->state = REMOVEFRIEND;
                    send_list_of_available_friends(clients, *client, *actual, client->sock, buffer, 0);
                    write_client(client->sock, "Enter the friend name, or ':exit' to cancel removing friend.\n");
                }
                else
                {
                    write_client(client->sock, "No available clients.\n");
                }
            }
            else if ((strcmp(buffer, ":lf") == 0) || (strcmp(buffer, ":listFriends") == 0))
            {
                if (client->number_friend > 0)
                {
                    send_list_of_friends(clients, *client, *actual, client->sock, buffer, 0);
                }
                else
                {
                    write_client(client->sock, "You have no friends.\n");
                }
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
            else if (strcmp(buffer, ":exit") == 0)
            {
                client->state = IDLE;
                write_client(client->sock, "You canceled the invitation.\n");
            }
            else
            {
                challenge_another_client_request(clients, client, *actual, client->sock, buffer, 0);
            }
        }
        break;
    case WAITING:
        if (FD_ISSET(client->sock, rdfs))
        {
            c = read_client(client->sock, buffer);
            if (c == 0)
            {
                client->opponent->state = IDLE;
                client->opponent->opponent = NULL;
                write_client(client->opponent->sock, "Your opponent has disconnected!");
                handle_disconnect_client(clients, *client, i, actual);
            }
            else
            {
                write_client(client->sock, "You are waiting for your opponent to accept your challenge.\n");
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
                message[0] = '\0';
                if (strcmp(buffer, "yes") == 0)
                {
                    strcat(message, "Are you ready? Game start!\n");

                    write_client(client->sock, message);
                    write_client(client->opponent->sock, message);
                    srand(time(NULL));
                    randomValue = rand() % 2;
                    if (randomValue == 0)
                    {
                        current_game->player1 = client;
                        current_game->player2 = client->opponent;
                    }
                    else
                    {
                        current_game->player2 = client;
                        current_game->player1 = client->opponent;
                    }
                    // start game

                    init_game(current_game);
                    write_client(current_game->player1->sock, "You are player1, you move first.\n");
                    write_client(current_game->player2->sock, "You are player2, you move after player1 moved.\n");
                    current_game->player1->currentGame = game_index[0];
                    current_game->player2->currentGame = game_index[0];
                    current_game->player1->state = PLAYER1;
                    current_game->player2->state = PLAYER2;
                    game_play(current_game);

                    game_index[0]++;
                }
                else if (strcmp(buffer, "no") == 0)
                {
                    client->state = IDLE;
                    client->opponent->state = IDLE;

                    // send message to challenger that challengee declined
                    strcat(message, "You declined the challenge.\n");

                    write_client(client->sock, message);

                    message[0] = '\0';
                    strcat(message, "Your opponent declined the challenge.\n");

                    write_client(client->opponent->sock, message);
                    client->opponent->opponent = NULL;
                    client->opponent = NULL;
                }
                else
                {
                    strcat(message, "Invalid response. Please resend the answer.\n");
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
            c = read_client(client->sock, buffer);
            if (c == 0)
            {
                updateEloRatings(client, anotherPlayer, -1);
                write_client(anotherPlayer->sock, "Your opponent disconnected, you won this game!\n");
                game->status[0] = '\0';
                strcat(game->status, "Player ");
                numStr[0] = '\0';
                sprintf(numStr, "%d", (game->currentPlayer == 1) ? 2 : 1);
                strcat(game->status, numStr);

                strcat(game->status, " name : ");
                strcat(game->status, anotherPlayer->name);
                strcat(game->status, " won.");

                anotherPlayer->opponent = NULL;
                anotherPlayer->state = HISTORY;
                strcat(buffer, "\nGame status : ");
                strcat(buffer, game->status);
                strcat(buffer, "\n");
                write_client(anotherPlayer->sock, buffer);
                write_client(anotherPlayer->sock, "Do you want to save this game in your history? (yes/no)\n");
                handle_disconnect_client(clients, *client, i, actual);
            }
            else if ((strcmp(buffer, ":s") == 0))
            {
                updateEloRatings(client, anotherPlayer, -1);
                write_client(anotherPlayer->sock, "Your opponent surrendered and you won the game!\n");
                game->status[0] = '\0';
                strcat(game->status, "Player ");
                numStr[0] = '\0';
                sprintf(numStr, "%d", (game->currentPlayer == 1) ? 2 : 1);
                strcat(game->status, numStr);
                strcat(game->status, " name : ");
                strcat(game->status, anotherPlayer->name);

                strcat(game->status, " won.");
                game_over(game);
            }
            else if ((strcmp(buffer, ":p") == 0))
            {
                game->privacy_mode = 1;
                write_client(client->sock, "You opened privacy mode.\n");
                write_client(anotherPlayer->sock, "Your opponent opened privacy mode.\n");
            }
            else if ((strcmp(buffer, ":e") == 0))
            {
                game->privacy_mode = 0;
                write_client(client->sock, "You closed privacy mode.\n");
                write_client(anotherPlayer->sock, "Your opponent closed privacy mode.\n");
            }
            else if (strncmp(buffer, ":dm ", 4) == 0)
            {
                message[0] = '\0';
                strcat(message, "Your opponent ");
                strcat(message, client->name);
                strcat(message, " : ");
                strcat(message, buffer + 4);
                write_client(client->opponent->sock, message);
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
            else if (check)
            {
                write_client(client->sock, "Please choose a valid position.\n");
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
                updateEloRatings(client, anotherPlayer, -1);
                write_client(anotherPlayer->sock, "Your opponent disconnected, you won this game!\n");
                game->status[0] = '\0';
                strcat(game->status, "Player ");
                numStr[0] = '\0';
                sprintf(numStr, "%d", game->currentPlayer);
                strcat(game->status, numStr);
                strcat(game->status, " name : ");
                strcat(game->status, anotherPlayer->name);
                strcat(game->status, " won.");
                anotherPlayer->opponent = NULL;
                anotherPlayer->state = HISTORY;
                strcat(buffer, "\nGame status : ");
                strcat(buffer, game->status);
                strcat(buffer, "\n");
                write_client(anotherPlayer->sock, buffer);
                write_client(anotherPlayer->sock, "Do you want to save this game in your history? (yes/no)\n");
                handle_disconnect_client(clients, *client, i, actual);
            }
            else if ((strcmp(buffer, ":s") == 0))
            {
                updateEloRatings(client, anotherPlayer, -1);
                write_client(anotherPlayer->sock, "Your opponent surrendered and you won the game!\n");
                game->status[0] = '\0';
                strcat(game->status, "Player ");
                numStr[0] = '\0';
                sprintf(numStr, "%d", (game->currentPlayer == 1) ? 2 : 1);
                strcat(game->status, numStr);
                strcat(game->status, " name : ");
                strcat(game->status, anotherPlayer->name);

                strcat(game->status, " won.");
                game_over(game);
            }
            else if ((strcmp(buffer, ":p") == 0))
            {
                game->privacy_mode = 1;
                write_client(client->sock, "You opened privacy mode.\n");
                write_client(anotherPlayer->sock, "Your opponent opened privacy mode.\n");
            }
            else if ((strcmp(buffer, ":e") == 0))
            {
                game->privacy_mode = 0;
                write_client(client->sock, "You closed privacy mode.\n");
                write_client(anotherPlayer->sock, "Your opponent closed privacy mode.\n");
            }
            else if (strncmp(buffer, ":dm ", 4) == 0)
            {
                message[0] = '\0';
                strcat(message, "Your opponent ");
                strcat(message, client->name);
                strcat(message, " : ");
                strcat(message, buffer + 4);
                write_client(client->opponent->sock, message);
            }
            else
            {
                write_client(client->sock, "Invalid input.\n");
            }
        }
        break;

    case INITSTANDBY:
        if (FD_ISSET(client->sock, rdfs))
        {
            c = read_client(client->sock, buffer);
            if (c == 0)
            {
                handle_disconnect_client(clients, *client, i, actual);
            }
            else
            {
                if (is_number(buffer))
                {
                    int number = atoi(buffer);
                    if (number < 0 || number >= game_index[0])
                    {
                        write_client(client->sock, "Invalid number, please enter a valid game number.\n");
                    }
                    else if (games[number].privacy_mode && !is_friend(client, games[number].player1) && !is_friend(client, games[number].player2))
                    {
                        write_client(client->sock, "This game is in privacy mode. You are not allowed to spectate this game.\n");
                    }
                    else
                    {
                        client->state = STANDBY;
                        client->observeGame = number;
                        add_observer((games + number), client);
                        game = (games + number);

                        write_client(client->sock, "You are now in spectator mode. Enter ':exit' to quit the mode.\n");
                        display_board(game, game->board, game->score, client);
                        if ((strcmp(game->status, "In game")))
                        {
                            write_client(client->sock, "\nThe game is over. Game status : ");
                            write_client(client->sock, game->status);
                            write_client(client->sock, "\n");
                        }
                    }
                }
                else if ((strcmp(buffer, ":exit") == 0))
                {
                    write_client(client->sock, "Spectate request cancelled.\n");
                    client->state = IDLE;
                }
                else
                {
                    write_client(client->sock, "Invalid input, please enter a number.\n");
                }
            }
        }
        break;

    case STANDBY:
        if (FD_ISSET(client->sock, rdfs))
        {
            c = read_client(client->sock, buffer);
            game = (games + client->observeGame);

            if (c == 0)
            {
                remove_observer(game, client);
                handle_disconnect_client(clients, *client, i, actual);
            }
            else if ((strcmp(buffer, ":exit") == 0))
            {
                remove_observer(game, client);
                client->state = IDLE;
                client->observeGame = -1;
                write_client(client->sock, "You are no longer watching this game.\n");
            }
        }
        break;

    case BIO:
        if (FD_ISSET(client->sock, rdfs))
        {
            c = read_client(client->sock, buffer);
            if (c == 0)
            {
                handle_disconnect_client(clients, *client, i, actual);
            }
            else if ((strcmp(buffer, ":exit") == 0))
            {
                client->state = IDLE;
                write_client(client->sock, "You canceled modifying your bio.\n");
            }
            else
            {
                client->bio[0] = '\0';
                strcat(client->bio, buffer);
                write_client(client->sock, "You modified your bio.\n");
                client->state = IDLE;
            }
        }
        break;

    case DM:

        if (FD_ISSET(client->sock, rdfs))
        {
            c = read_client(client->sock, buffer);
            if (c == 0)
            {
                handle_disconnect_client(clients, *client, i, actual);
            }
            else if ((strcmp(buffer, ":exit") == 0))
            {

                write_client(client->sock, "You canceled sending direct messages.\n"
                                           "Back to broadcast mode.\n");
                client->state = IDLE;
            }
            else
            {
                // split the buffer into 2 parts separated by space
                char *username = strtok(buffer, " ");
                // rest of the buffer is the message
                char *message = strtok(NULL, "\n");
                if (message == NULL)
                {
                    write_client(client->sock, "Invalid message.\n"
                                               "Back to broadcast mode.\n");
                    client->state = IDLE;
                }
                else
                {
                    int found = 0;
                    for (int j = 0; j < *actual; j++)
                    {
                        if (strcmp(client->name, username) == 0)
                        {
                            write_client(client->sock, "You cannot send message to yourself.\n"
                                                       "Back to broadcast mode.\n");
                            client->state = IDLE;
                            found = 1;
                            break;
                        }
                        if (strcmp(clients[j].name, username) == 0)
                        {
                            found = 1;
                            write_client(clients[j].sock, "You have a private message from ");
                            write_client(clients[j].sock, client->name);
                            write_client(clients[j].sock, " : ");
                            write_client(clients[j].sock, message);
                            write_client(clients[j].sock, "\n");
                            write_client(client->sock, "Your message has been sent.\n"
                                                       "Back to broadcast mode.\n");
                            client->state = IDLE;
                            break;
                        }
                    }
                    if (!found)
                    {
                        write_client(client->sock, "No client with this name. Please enter a valid name.\n");
                    }
                }
            }
        }
        break;
    case ADDFRIEND:
        if (FD_ISSET(client->sock, rdfs))
        {
            c = read_client(client->sock, buffer);
            if (c == 0)
            {
                handle_disconnect_client(clients, *client, i, actual);
            }
            else if ((strcmp(buffer, ":exit") == 0))
            {
                client->state = IDLE;
                write_client(client->sock, "You canceled adding friend.\n");
            }
            else
            {
                add_friend_request(clients, client, *actual, client->sock, buffer, 0);
            }
        }
        break;
    case WAITINGFRIEND:
        if (FD_ISSET(client->sock, rdfs))
        {
            c = read_client(client->sock, buffer);

            if (c == 0)
            {
                client->friend_request->state = IDLE;
                client->friend_request->friend_request = NULL;
                write_client(client->friend_request->sock, "Your friend has disconnected!");
                handle_disconnect_client(clients, *client, i, actual);
            }
            else
            {
                write_client(client->sock, "You are waiting for your friend to accept your friend request.\n");
            }
        }
        break;

    case FRIEND:
        if (FD_ISSET(client->sock, rdfs))
        {
            c = read_client(client->sock, buffer);
            if (c == 0)
            {
                write_client(client->friend_request->sock, "Your friend has disconnected!");
                client->friend_request->state = IDLE;
                client->friend_request->friend_request = NULL;
                handle_disconnect_client(clients, *client, i, actual);
            }
            else
            {
                message[0] = '\0';
                if (strcmp(buffer, "yes") == 0)
                {
                    strcat(message, "You have a new friend : ");
                    strcat(message, client->friend_request->name);
                    strcat(message, "\n");
                    write_client(client->sock, message);

                    message[0] = '\0';
                    strcat(message, "You have a new friend : ");
                    strcat(message, client->name);
                    strcat(message, "\n");
                    write_client(client->friend_request->sock, message);
                    add_friend(client, client->friend_request);
                    add_friend(client->friend_request, client);
                    client->state = IDLE;
                    client->friend_request->state = IDLE;
                    client->friend_request->friend_request = NULL;
                    client->friend_request = NULL;
                }
                else if (strcmp(buffer, "no") == 0)
                {
                    strcat(message, "You declined the friend request.\n");
                    write_client(client->sock, message);
                    message[0] = '\0';
                    strcat(message, "Client ");
                    strcat(message, client->name);
                    strcat(message, " declined the friend request.\n");
                    write_client(client->friend_request->sock, message);
                    client->state = IDLE;
                    client->friend_request->state = IDLE;
                    client->friend_request->friend_request = NULL;
                    client->friend_request = NULL;
                }
                else
                {
                    strcat(message, "Invalid response. Please resend the answer.\n");
                    write_client(client->sock, message);
                }
            }
        }
        break;
    case REMOVEFRIEND:
        if (FD_ISSET(client->sock, rdfs))
        {
            c = read_client(client->sock, buffer);
            if (c == 0)
            {
                handle_disconnect_client(clients, *client, i, actual);
            }
            else if ((strcmp(buffer, ":exit") == 0))
            {
                client->state = IDLE;
                write_client(client->sock, "You canceled removing friend.\n");
            }
            else
            {
                remove_friend_request(clients, client, *actual, client->sock, buffer, 0);
            }
        }
        break;
    case HISTORY:
        if (FD_ISSET(client->sock, rdfs))
        {
            c = read_client(client->sock, buffer);
            if (c == 0)
            {
                handle_disconnect_client(clients, *client, i, actual);
            }
            else if ((strcmp(buffer, "yes") == 0))
            {
                write_client(client->sock, "You saved this game in your history.\n");
                client->state = IDLE;
                client->history[client->number_game] = client->currentGame;
                client->currentGame = -1;
                client->number_game++;
            }
            else if ((strcmp(buffer, "no") == 0))
            {
                write_client(client->sock, "You did not save this game in your history.\n");
                client->state = IDLE;
                client->currentGame = -1;
            }
            else
            {
                write_client(client->sock, "Invalid input.\n");
            }
        }
        break;
    case VIEWGAME:
        if (FD_ISSET(client->sock, rdfs))
        {
            c = read_client(client->sock, buffer);
            if (c == 0)
            {
                handle_disconnect_client(clients, *client, i, actual);
            }
            else if ((strcmp(buffer, ":exit") == 0))
            {
                client->state = IDLE;
                write_client(client->sock, "You canceled viewing game.\n");
            }
            else if (is_number(buffer))
            {
                int number = atoi(buffer);
                int found = 0;
                for (int i = 0; i < client->number_game; i++)
                {
                    if (client->history[i] == number)
                    {
                        found = 1;
                        game = (games + number);
                        display_board(game, game->board, game->score, client);
                        write_client(client->sock, "Game status : ");
                        write_client(client->sock, game->status);
                        write_client(client->sock, "\n");
                        break;
                    }
                }
                if (!found)
                {
                    write_client(client->sock, "Invalid number, please enter a valid game number.\n");
                }
            }
            else
            {
                write_client(client->sock, "Invalid input, please enter a number.\n");
            }
        }
        break;

    default:
        fprintf(stderr, "Unknown state for client %s\n", client->name);
        break;
    }
}

static void handle_server_input(Client *clients, int actual, int sock, char *buffer, AwaleGame games[], int game_index[])
{
    if (strcmp(buffer, ":help\n") == 0)
    {
        int width = 50; // Width of the frame

        // Top border
        for (int i = 0; i < width; i++)
        {
            printf("=");
        }
        printf("\n");

        // Title
        printf("| Available commands:\n");

        // Separator
        for (int i = 0; i < width; i++)
        {
            printf("-");
        }
        printf("\n");

        // Commands
        printf("| `:ls` or `:list` - list all connected clients\n");
        printf("| `:lsg` or `:listGames` - list all games\n");
        printf("| `:rank` - show ranking of all connected clients\n");
        printf("| `:broadcast [message]- send message to all connected clients\n");
        printf("| `:kick [username]` - kick a user out of server\n");
        printf("| `:exit`, `CTRL-C` or `CTRL-D` - shut down server\n");

        // Bottom border
        for (int i = 0; i < width; i++)
        {
            printf("=");
        }
        printf("\n");
    }
    else if (strcmp(buffer, ":exit\n") == 0)
    {
        clear_clients(clients, actual);
        end_connection(sock);
        exit(0);
    }
    else if (strncmp(buffer, ":broadcast ", 11) == 0)
    {
        char message[BUF_SIZE];
        message[0] = '\0';
        strcat(message, "[Server] ");
        strcat(message, buffer + 11);
        for (int i = 0; i < actual; i++)
        {
            write_client(clients[i].sock, message);
        }
    }
    else if (strncmp(buffer, ":warn ", 6) == 0)
    {

        char name[NAME_SIZE + 1];
        strncpy(name, buffer + 6, NAME_SIZE);
        name[NAME_SIZE] = '\0';

        char *newline = strchr(name, '\n');
        if (newline) {
            *newline = '\0';
        }

        int found = 0;
        for (int i = 0; i < actual; i++)
        {
            if (strcmp(clients[i].name, name) == 0)
            {
                found = 1;
                write_client(clients[i].sock, "[Server] You have been warned. Watch yourself.\n");
                break;
            }
        }
        if (!found)
        {
            printf("Client %s not found\n", name);
        }
        else
        {
            printf("Client %s warned\n", name);
        }
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
    else if (strcmp(buffer, ":lsg\n") == 0 || strcmp(buffer, ":listGames\n") == 0)
    {
        char list_buffer[2048];
        list_buffer[0] = '\0';
        strcpy(list_buffer, "Games:\n");
        if (game_index[0] == 0)
        {
            printf("No available games.\n");
            return;
        }
        else
        {
            strncat(list_buffer, "┌────────┬────────────────────────────────────────┐\n", sizeof(list_buffer) - strlen(list_buffer) - 1);
            strncat(list_buffer, "│ Number │                 Status                 │\n", sizeof(list_buffer) - strlen(list_buffer) - 1);

            for (int i = 0; i < game_index[0]; i++)
            {

                char line[256];
                strncat(list_buffer, "├────────┼────────────────────────────────────────┤\n", sizeof(list_buffer) - strlen(list_buffer) - 1);
                snprintf(line, sizeof(line), "│ %-6d │ %-38s │\n", i, games[i].status);
                strncat(list_buffer, line, sizeof(list_buffer) - strlen(list_buffer) - 1);
            }

            strncat(list_buffer, "└────────┴────────────────────────────────────────┘\n", sizeof(list_buffer) - strlen(list_buffer) - 1);
        }
        printf("%s", list_buffer);
    }
    else if (strcmp(buffer, ":rank\n") == 0)
    {
        if (actual == 0)
        {
            printf("No client connected\n");
            return;
        }
        for (int i = 0; i < actual; i++)
        {
            for (int j = i + 1; j < actual; j++)
            {
                if (clients[i].score < clients[j].score)
                {
                    Client temp = clients[i];
                    clients[i] = clients[j];
                    clients[j] = temp;
                }
            }
        }
        char ranking_buffer[2048];
        snprintf(ranking_buffer, sizeof(ranking_buffer), "Ranking:\n");

        // Header with separators
        strncat(ranking_buffer, "┌──────┬────────────────────┬───────┐\n", sizeof(ranking_buffer) - strlen(ranking_buffer) - 1);
        strncat(ranking_buffer, "│ Rank │        Name        │ Score │\n", sizeof(ranking_buffer) - strlen(ranking_buffer) - 1);

        int current_rank = 0;
        int last_score = -1;
        for (int i = 0; i < actual; i++)
        {
            if (clients[i].score != last_score)
            {
                current_rank = i + 1;
                last_score = clients[i].score;
            }

            char line[256];
            strncat(ranking_buffer, "├──────┼────────────────────┼───────┤\n",
                    sizeof(ranking_buffer) - strlen(ranking_buffer) - 1);
            snprintf(line, sizeof(line), "│ %-4d │ %-18s │ %-5d │\n", current_rank, clients[i].name, clients[i].score);
            strncat(ranking_buffer, line, sizeof(ranking_buffer) - strlen(ranking_buffer) - 1);
        }
        strncat(ranking_buffer, "└──────┴────────────────────┴───────┘\n", sizeof(ranking_buffer) - strlen(ranking_buffer) - 1);
        printf("%s", ranking_buffer);
    }

    else
    {
        printf("Server received unknown command.\n");
    }
}

static void app(void)
{
    SOCKET sock = init_connection();
    printf("Oware game server start.\n");
    printf("Enter `:help` to get help.\n");
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
                handle_server_input(clients, actual, sock, buffer, games, game_index);
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

static void send_ranking_to_client(Client *clients, Client client, int actual, int sender_sock, const char *buffer,
                                   int from_server)
{
    for (int i = 0; i < actual; i++)
    {
        for (int j = i + 1; j < actual; j++)
        {
            if (clients[i].score < clients[j].score)
            {
                Client temp = clients[i];
                clients[i] = clients[j];
                clients[j] = temp;
            }
        }
    }

    char ranking_buffer[2048]; // Assuming 2048 is sufficient
    snprintf(ranking_buffer, sizeof(ranking_buffer), "Ranking:\n");

    // Header with separators
    strncat(ranking_buffer, "┌──────┬────────────────────┬───────┐\n", sizeof(ranking_buffer) - strlen(ranking_buffer) - 1);
    strncat(ranking_buffer, "│ Rank │        Name        │ Score │\n", sizeof(ranking_buffer) - strlen(ranking_buffer) - 1);

    int current_rank = 0;
    int last_score = -1;
    for (int i = 0; i < actual; i++)
    {
        if (clients[i].score != last_score)
        { // New rank for different score
            current_rank = i + 1;
            last_score = clients[i].score;
        }

        char line[256];
        strncat(ranking_buffer, "├──────┼────────────────────┼───────┤\n", sizeof(ranking_buffer) - strlen(ranking_buffer) - 1);
        snprintf(line, sizeof(line), "│ %-4d │ %-18s │ %-5d │\n", current_rank, clients[i].name, clients[i].score);
        strncat(ranking_buffer, line, sizeof(ranking_buffer) - strlen(ranking_buffer) - 1);
    }

    strncat(ranking_buffer, "└──────┴────────────────────┴───────┘\n", sizeof(ranking_buffer) - strlen(ranking_buffer) - 1);

    // Send the ranking to the client
    if (sender_sock != -1)
    {
        write_client(sender_sock, ranking_buffer);
    }
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
    for (int j = 0; j < client.number_friend; j++)
    {
        for (int k = 0; k < *actual; k++)
        {
            if (strcmp(clients[k].name, client.friend[j]) == 0)
            {
                remove_friend((clients + i), (clients + k));
                remove_friend((clients + k), (clients + i));
            }
        }
    }
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
send_list_of_clients(Client *clients, Client client, int actual, int sender_sock, const char *buffer, int from_server, int include_self)
{
    char list_buffer[2048]; // Increase buffer size if necessary
    char line_buffer[128];  // Buffer for individual lines

    // Check if any clients are connected
    if (actual == 0)
    {
        strcpy(list_buffer, "No clients connected.\n");
    }
    else
    {
        // Start of the framed list
        strcpy(list_buffer, "┌───────────────┬───────────────────────────────┐\n");
        strcat(list_buffer, "│  Client name  │              Bio              │\n");
        strcat(list_buffer, "├───────────────┴───────────────────────────────┤\n");

        // Add each client to the list
        for (int i = 0; i < actual; i++)
        {
            if (strcmp(clients[i].name, client.name) == 0 && include_self == 0)
            {
                continue;
            }
            snprintf(line_buffer, sizeof(line_buffer), "│ %-13s │ %-29s │\n", clients[i].name, clients[i].bio);
            strcat(list_buffer, line_buffer);
        }

        // End of the framed list
        strcat(list_buffer, "└───────────────────────────────────────────────┘\n");
    }

    // Send the list to the requesting client
    if (sender_sock != -1)
    {
        write_client(sender_sock, list_buffer);
    }
}

static void
send_list_of_games(AwaleGame games[], int game_index[], Client client, int actual, int sender_sock, const char *buffer, int from_server)
{
    char list_buffer[2048]; // Assuming 1024 is sufficient
    strcpy(list_buffer, "Games:\n");
    if (game_index[0] == 0)
    {
        strcpy(list_buffer, "No available games.\n");
    }
    else
    {
        strncat(list_buffer, "┌────────┬────────────────────────────────────────┐\n", sizeof(list_buffer) - strlen(list_buffer) - 1);
        strncat(list_buffer, "│ Number │                 Status                 │\n", sizeof(list_buffer) - strlen(list_buffer) - 1);

        for (int i = 0; i < game_index[0]; i++)
        {

            char line[256];
            strncat(list_buffer, "├────────┼────────────────────────────────────────┤\n", sizeof(list_buffer) - strlen(list_buffer) - 1);
            snprintf(line, sizeof(line), "│ %-6d │ %-38s │\n", i, games[i].status);
            strncat(list_buffer, line, sizeof(list_buffer) - strlen(list_buffer) - 1);
        }

        strncat(list_buffer, "└────────┴────────────────────────────────────────┘\n", sizeof(list_buffer) - strlen(list_buffer) - 1);
    }

    if (sender_sock != -1)
    {
        // Send only to the requesting client
        write_client(client.sock, list_buffer);
    }
}

static void
send_list_of_saved_games(AwaleGame games[], int game_index[], Client client, int actual, int sender_sock, const char *buffer, int from_server)
{
    char list_buffer[2048]; // Assuming 1024 is sufficient
    strcpy(list_buffer, "Games:\n");

    strncat(list_buffer, "┌────────┬────────────────────────────────────────┐\n", sizeof(list_buffer) - strlen(list_buffer) - 1);
    strncat(list_buffer, "│ Number │                 Status                 │\n", sizeof(list_buffer) - strlen(list_buffer) - 1);

    for (int i = 0; i < client.number_game; i++)
    {
        char line[256];
        strncat(list_buffer, "├────────┼────────────────────────────────────────┤\n", sizeof(list_buffer) - strlen(list_buffer) - 1);
        snprintf(line, sizeof(line), "│ %-6d │ %-38s │\n", client.history[i], games[client.history[i]].status);
        strncat(list_buffer, line, sizeof(list_buffer) - strlen(list_buffer) - 1);
    }

    strncat(list_buffer, "└────────┴────────────────────────────────────────┘\n", sizeof(list_buffer) - strlen(list_buffer) - 1);

    if (sender_sock != -1)
    {
        // Send only to the requesting client
        write_client(client.sock, list_buffer);
    }
}

static void
send_list_of_friends(Client *clients, Client client, int actual, int sender_sock, const char *buffer, int from_server)
{
    char list_buffer[2048];
    char line_buffer[128]; // Buffer for individual lines

    list_buffer[0] = '\0';
    strcat(list_buffer, "┌──────────────────────────────┐\n");
    strcat(list_buffer, "│     List of your friends:    │\n");
    strcat(list_buffer, "├──────────────────────────────┤\n");
    for (int i = 0; i < client.number_friend; i++)
    {
        snprintf(line_buffer, sizeof(line_buffer), "│  %-27s │\n", client.friend[i]);
        strcat(list_buffer, line_buffer);
    }
    strcat(list_buffer, "└──────────────────────────────┘\n");

    if (sender_sock != -1)
    {
        // Send only to the requesting client
        write_client(client.sock, list_buffer);
    }
}

static void
send_list_of_available_friends(Client *clients, Client client, int actual, int sender_sock, const char *buffer, int from_server)
{
    char list_buffer[2048];
    char line_buffer[128]; // Buffer for individual lines

    list_buffer[0] = '\0';
    strcat(list_buffer, "┌──────────────────────────────┐\n");
    strcat(list_buffer, "│  List of available friends:  │\n");
    strcat(list_buffer, "├──────────────────────────────┤\n");
    for (int i = 0; i < client.number_friend; i++)
    {
        for (int j = 0; j < actual; j++)
        {
            if (strcmp(client.friend[i], clients[j].name) == 0)
            {
                snprintf(line_buffer, sizeof(line_buffer), "│  %-27s │\n", client.friend[i]);
                strcat(list_buffer, line_buffer);
            }
        }
    }
    strcat(list_buffer, "└──────────────────────────────┘\n");

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
    char list_buffer[2048];
    char line_buffer[128]; // Buffer for individual lines

    list_buffer[0] = '\0';
    strcat(list_buffer, "┌──────────────────────────────┐\n");
    strcat(list_buffer, "│  List of available clients:  │\n");
    strcat(list_buffer, "├──────────────────────────────┤\n");
    for (int i = 0; i < actual; i++)
    {
        if (clients[i].sock == sender_sock || clients[i].state != IDLE)
        {
            continue;
        }
        snprintf(line_buffer, sizeof(line_buffer), "│  %-27s │\n", clients[i].name);
        strcat(list_buffer, line_buffer);
    }
    strcat(list_buffer, "└──────────────────────────────┘\n");

    if (sender_sock != -1)
    {
        // Send only to the requesting client
        write_client(client->sock, list_buffer);
        char challenge_buffer[1024];
        strcpy(challenge_buffer, "Who do you want to fight?\n");
        strcat(challenge_buffer, "Enter ':exit' to cancel this invitation.\n");

        write_client(client->sock, challenge_buffer);
    }
    else
    {
        return;
    }
}

static void
add_friend_init(Client *clients, Client *client, int actual, int sender_sock, const char *buffer,
                int from_server)
{
    char list_buffer[2048];
    char line_buffer[128]; // Buffer for individual lines

    list_buffer[0] = '\0';
    strcat(list_buffer, "┌──────────────────────────────┐\n");
    strcat(list_buffer, "│  List of available clients:  │\n");
    strcat(list_buffer, "├──────────────────────────────┤\n");
    for (int i = 0; i < actual; i++)
    {
        if (clients[i].sock == sender_sock || clients[i].state != IDLE || is_friend(client, (clients + i)))
        {
            continue;
        }
        snprintf(line_buffer, sizeof(line_buffer), "│  %-27s │\n", clients[i].name);
        strcat(list_buffer, line_buffer);
    }
    strcat(list_buffer, "└──────────────────────────────┘\n");

    if (sender_sock != -1)
    {
        write_client(client->sock, list_buffer);
        write_client(client->sock, "Enter the name of the friend you want to add, or enter ':exit' to cancel the request: \n");
    }
    else
    {
        return;
    }
}

static void
add_friend_request(Client *clients, Client *client, int actual, int sender_sock, const char *buffer,
                   int from_server)
{
    int friend_sock = -1;

    if (strcmp(client->name, buffer) == 0)
    {
        write_client(client->sock, "You can't add yourself\n");
        return;
    }

    for (int i = 0; i < actual; i++)
    {
        if (strcmp(clients[i].name, buffer) == 0 && clients[i].state == IDLE)
        {
            if (is_friend(client, (clients + i)))
            {
                write_client(client->sock, "You can't add a friend twice\n");
                return;
            }
            friend_sock = clients[i].sock;
            clients[i].state = FRIEND;
            clients[i].friend_request = client;
            client->friend_request = (clients + i);
            client->state = WAITINGFRIEND;

            break;
        }
    }

    if (friend_sock == -1)
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
        char friend_buffer[1024];
        strcpy(friend_buffer, "Client found\n");
        strcat(friend_buffer, "Your friend request is being considered.\n");
        write_client(client->sock, friend_buffer);

        friend_buffer[0] = '\0';
        strcat(friend_buffer, "Your received a friend request from user: ");
        strcat(friend_buffer, client->name);
        strcat(friend_buffer, " \n");
        strcat(friend_buffer, "Do you want to accept the request? (yes/no)\n");
        write_client(friend_sock, friend_buffer);
    }
}

static void
remove_friend_request(Client *clients, Client *client, int actual, int sender_sock, const char *buffer,
                      int from_server)
{
    int friend_sock = -1;

    for (int i = 0; i < actual; i++)
    {
        if (strcmp(clients[i].name, buffer) == 0)
        {
            if (!is_friend(client, (clients + i)))
            {
                write_client(client->sock, "You can't remove a client that is not in your friend list\n");
                return;
            }
            friend_sock = clients[i].sock;
            remove_friend(client, (clients + i));
            remove_friend((clients + i), client);
            break;
        }
    }

    if (friend_sock == -1)
    {
        if (sender_sock != -1)
        {
            write_client(client->sock, "Client not found\n");
        }
    }
    else
    {
        char friend_buffer[1024];
        strcat(friend_buffer, "You removed your friend ");
        strcat(friend_buffer, buffer);
        strcat(friend_buffer, "\n");
        write_client(client->sock, friend_buffer);

        friend_buffer[0] = '\0';
        strcat(friend_buffer, "Your friend ");
        strcat(friend_buffer, client->name);
        strcat(friend_buffer, " removed you from their friend list.\n");
        write_client(friend_sock, friend_buffer);
        client->state = IDLE;
    }
}

static void
challenge_another_client_request(Client *clients, Client *client, int actual, int sender_sock, const char *buffer,
                                 int from_server)
{

    int challengee_sock = -1;

    if (strcmp(client->name, buffer) == 0)
    {
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
            client->state = WAITING;
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
        strcpy(challenge_buffer, "Your opponent is considering your invitation.\n");
        write_client(client->sock, challenge_buffer);

        char challengee_response_buffer[1024]; // Assuming 1024 is sufficient
        challengee_response_buffer[0] = '\0';
        strcat(challengee_response_buffer, "Your received an invitation from user: ");
        strcat(challengee_response_buffer, client->name);
        strcat(challengee_response_buffer, " \n");
        strcat(challengee_response_buffer, "Do you want to accept the fight? (yes/no)\n");

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
    int n = recv(sock, buffer, BUF_SIZE - 1, 0);
    if (n < 0)
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
