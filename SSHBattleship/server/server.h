#ifndef SERVER_H
#define SERVER_H

#include <sys/socket.h>
#include <netdb.h>
#include "battleship.h"
#include "player_pool.h"

#define PORT "17400"
#define BACKLOG 10
#define MAXDATASIZE 1000

#define START_STATUS 0
#define SHIP_STATUS 1
#define MOVE_STATUS 2
#define HIT_STATUS  3
#define GAMEOVER_STATUS 4
#define INVALID_STATUS 5
#define REGISTER_STATUS 6
#define SIGNIN_STATUS 7
#define SUCCESS_STATUS 8
#define CHALLENGE_STATUS 9
#define ACCEPT_STATUS 10
#define DECLINE_STATUS 11

typedef struct thread_args{
    struct Player *player1;
    struct Player *player2;
    struct player_pool *storage;
} ThreadArgs;

void setAdditionalInfo(struct addrinfo *additionalInfo);
void getServerInfoList(struct addrinfo **result);
int createAndBindReusableSocket(struct addrinfo *serverInfoList);

void listenForConnections(int socket_FileDescriptor);

int acceptPlayerConnection(int socket_FileDescriptor);
void printAcceptedConnection(struct sockaddr_storage player_addr);
void *getInetVersionSpecificAddress(struct sockaddr *player_addr);

void initializeRecordsFile();

char *registerPlayer(int player_socket);
struct Player *signinPlayer();
void handlePlayerMode(struct Player *player, struct player_pool *random,
        struct player_pool *friendly, struct player_pool *storage);
void sendFriendlyWaitingList(int player_socket, struct player_pool *friendly);

void dispatchPlayersToSubserver(struct player_pool *storage,
        struct Player *player1, struct Player *player2);
void *sub_server_main(void *argv);

void place_ships(void *argv);
void print_shipboard(char **board);

void sendTurnInfo(int player_socket, int player2_socket);
void playerTurn(int turnPlayer, int opponent, Game *playerBoard, int *gameOver);
void constructHitMessage(Game *playerBoard, char **buff, int turnPlayer, int opponent);

void sendPlayersStats(struct Player *me, struct Player *opponent);
void printPlayersStats(struct Player *player1, struct Player *player2);
void sendWinningUsername(struct Player *winner, int player1, int player2);
void updateGameInfo(struct Player *winner, struct Player *loser);

#endif
