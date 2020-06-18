/*  Program:      Server
    Authors:      Tyler Mulvihill, Mark Liskowicz
    Filename:     main.c
    Compile:      cc server.c player_pool.c battleship.c main.c -o server_main -lpthread
    Run:          ./server_main
    Last updated: November 10, 2017
  
    A game server program that allow 2 players to connect and play battleships.
 
 */

#include "server.h"
#include <stdio.h>

#define NULL ((void *)0)

int main() {
    int server_socket, player_socket;
    char player_status[1], *username;
    struct addrinfo *serverInfoList;
    struct Player *player, *player1, *player2;
    struct player_pool *random, *friendly, *storage;
    
    getServerInfoList(&serverInfoList);
    server_socket = createAndBindReusableSocket(serverInfoList);
    freeaddrinfo(serverInfoList);
    
    listenForConnections(server_socket);
    
    initializeRecordsFile();
    
    player_pool_init(&random);
    player_pool_init(&friendly);
    player_pool_init(&storage);
    while(1){
        if(random->size>=2){
            player1 = nextPlayer(random);
            player2 = nextPlayer(random);
            
            dispatchPlayersToSubserver(storage, player1, player2);
        }
        player_socket = acceptPlayerConnection(server_socket);
        if(player_socket != -1){
            recv(player_socket, player_status, 1, 0);
            if(player_status[0] == REGISTER_STATUS){
                do{
                    username = registerPlayer(player_socket);
                    if(username == NULL){
                        player_status[0] = INVALID_STATUS;
                        send(player_socket, player_status, 1, 0);
                    }
                } while(username == NULL);
                player_status[0] = SUCCESS_STATUS;
                send(player_socket, player_status, 1, 0);
                player = addNewPlayerToPool(storage, player_socket, username);
            }
            else if(player_status[0] == SIGNIN_STATUS) {
                do{
                    player = signinPlayer(player_socket, storage);
                    
                    if(player == NULL){
                        player_status[0] = INVALID_STATUS;
                        send(player_socket, player_status, 1, 0);
                    }
                } while(player == NULL);
                player_status[0] = SUCCESS_STATUS;
                send(player_socket, player_status, 1, 0);
                player->player_socket = player_socket;
                printf("%s signed in successfully.\n", player->username);
            }
            handlePlayerMode(player, random, friendly, storage);
        }
    }
    
    return 0; 
}