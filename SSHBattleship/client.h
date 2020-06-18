#ifndef CLIENT_H
#define CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define GAME_SIZE 10
#define PORT "17400"
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

void setAdditionalInfo(struct addrinfo *additionalInfo);
void getServerInfoList(char *hostname, struct addrinfo **result);
int connectToSocket(struct addrinfo *serverInfoList);
void *getInetVersionSpecificAddress(struct sockaddr *player_addr);
void printAcceptedConnection(struct sockaddr *socket_addr, int ai_family);
void print_shipboard(char **board);
void print_shotboard(char **board);
char * getdata(int server_socket);
void senddata(int ss, char *data);
void sendshipdata(int ss, char *data);
void sendshipcoords(int ss);
void sendcoords(int ss);
void addDataToRow(char **board, char *data, int row);
void setBoardState(char **board, int player1_socket);
void dogameover(int winner, int server_socket);
void place_ship(char *name, int server_socket, char **board);
char **createShotBoard();
void update_miss(char **board, int x, int y, char *message);
void update_hit(char **board, int x, int y, char *message);
void recieve_and_update(int server_socket, char **board, char **shotboard, int playerid, char *length);
void registerUsername(int server_socket);
void signIn(int server_socket);
char chooseMode(int server_socket);
void findOpponent(int server_socket);
#endif /* CLIENT_H */
